// Fill out your copyright notice in the Description page of Project Settings.


#include "Dashboard/SSSVoiceDashboard.h"

#include "SSVoiceLocalizationSettings.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/JsonSerializer.h"
#include "Slate/SSSVoiceSlateComponents.h"
#include "Utils/SSVoiceLocalizationUI.h"
#include "Utils/SSVoiceLocalizationUtils.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"

#define LOCTEXT_NAMESPACE "SSVoiceLocalization"

const FName SSSVoiceDashboard::OverviewTabId("Overview");
const FName SSSVoiceDashboard::VoiceActorTabName("Voice Actors");

void SSSVoiceDashboard::Construct(const FArguments& InArgs, const TSharedPtr<SWindow>& OwningWindow, const TSharedRef<SDockTab>& OwningTab)
{
	// Load report
	FSSVoiceLocalizationUtils::LoadSavedCultureReport(CultureReport);

	// Load actors
	LoadActorListFromJson();
	
	// Crée un TabManager temporaire (non attaché à un level editor)
	TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("SSSVoiceDashboardLayout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(OverviewTabId, ETabState::OpenedTab)
				->AddTab(VoiceActorTabName, ETabState::OpenedTab)
				->SetForegroundTab(OverviewTabId)
			)
		);

	
	// Crée un TabManager local
	TabManager = FGlobalTabmanager::Get()->NewTabManager(OwningTab);

	// Enregistre les spawners
	TabManager->RegisterTabSpawner(OverviewTabId, FOnSpawnTab::CreateSP(this, &SSSVoiceDashboard::SpawnDashboardTab))
		.SetDisplayName(FText::FromString("Overview"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	TabManager->RegisterTabSpawner(VoiceActorTabName, FOnSpawnTab::CreateSP(this, &SSSVoiceDashboard::SpawnVoiceActorTab))
	.SetDisplayName(FText::FromString("Voice Actors"))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	// Génère le layout complet
	ChildSlot
	[
		TabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef()
	];
}

TSharedRef<SDockTab> SSSVoiceDashboard::SpawnDashboardTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(3.0f)
			.Style(FEditorStyle::Get(), "DetailsView.Splitter")
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				BuildLeftPanel()
			]
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				BuildRightPanel()
			]
		];
}

TSharedRef<SDockTab> SSSVoiceDashboard::SpawnVoiceActorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::PanelTab)
	[
		SNew(SVerticalBox)
		// --- Voice Assets Info
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(8)
		[
			BuildAssetSection()
		]
	];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildLeftPanel()
{
	return SNew(SScrollBox)

			// --- Profile Selector
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProfileSelector", "Profile Selector"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(SSSAutofillProfileSelector)
					.OnProfileChanged(FSimpleDelegate::CreateLambda([this]()
					{
						// Callback when profile change
						UE_LOG(LogTemp, Log, TEXT("[SSVoice] Profile changed from selector widget"));

						// TODO: Rafraîchir le contenu si besoin
					}))
				]
			]

			// --- Culture Filter
			+ SScrollBox::Slot().Padding(4)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CultureFilterTitle", "Culture Filter"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(STextBlock)
					.Text(FText::FromString("→ [CheckBoxes per culture]"))
				]
			]

			// --- Cook Settings
			+ SScrollBox::Slot().Padding(4)
			[
				SNew(SExpandableArea)
				.AreaTitle(LOCTEXT("CookSettings", "Cooking Settings"))
				.BodyContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("→ [Cook culture selection]"))
				]
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildRightPanel()
{
	return SNew(SVerticalBox)

			// --- Coverage Overview
			+ SVerticalBox::Slot().AutoHeight().Padding(6)
			[
				BuildCoverageSection()
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(SSeparator)
			];
}


TSharedRef<SWidget> SSSVoiceDashboard::BuildAssetSection()
{
	return SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.f, 20.f, 10.f, 10.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LocalizedVoiceAssetsTitle", "Localized Voice Assets"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(10.f)
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)

				// === Left: Actor list ===
				+ SSplitter::Slot()
				.Value(0.35f)
				[
					BuildActorList()
				]

				// === Right: Voice assets for actor (TODO) ===
				+ SSplitter::Slot()
				.Value(0.65f)
				[
					BuildAssetList()
				]
			];
}

TSharedRef<ITableRow> SSSVoiceDashboard::GenerateActorRow(TSharedPtr<FString> InItem,
                                                          const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.Padding(0)
			[
				SNew(SHorizontalBox)

				// Barre verticale à gauche
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(4.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.8f, 0.3f, 1.0f)) // Barre rose-violet
					]
				]

				// Fond principal de la "card"
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBorder)
					.Padding(FMargin(8, 4))
					.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
					.BorderBackgroundColor(FLinearColor(0.45f, 0.2f, 0.4f)) // Fond violet-rose plus contrasté
					[
						SNew(STextBlock)
						.Text(FText::FromString(*InItem))
						.Font(FCoreStyle::Get().GetFontStyle("NormalFont"))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

void SSSVoiceDashboard::RebuildUI()
{
	// this->Construct(SSSVoiceDashboard::FArguments());
	Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildActorList()
{
	return SNew(SVerticalBox)
			// Rescan Button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Rescan Actors"))
				.OnClicked_Lambda([this]()
				{
					FSSVoiceLocalizationUtils::GenerateActorListJson();
					LoadActorListFromJson(); // ← à écrire juste après
					return FReply::Handled();
				})
			]
			// Barre de recherche
			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SAssignNew(ActorSearchBox, SSearchBox)
				.HintText(LOCTEXT("ActorSearchHint", "Search Voice Actor..."))
				.OnTextChanged_Lambda([this](const FText& NewText)
				{
					ActorSearchFilter = NewText.ToString();
					RefreshActorFilter();
				})
			]

			// Titre
			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ActorsHeader", "Voice Actors"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			// Liste des acteurs
			+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
			[
				SAssignNew(ActorListView, SListView<TSharedPtr<FString>>)
				.ItemHeight(24)
				.ListItemsSource(&FilteredActorItems)
				.OnGenerateRow(this, &SSSVoiceDashboard::GenerateActorRow)
				.SelectionMode(ESelectionMode::Single)
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
				{
					SelectedActor = Item;
					// Call refresh voice sound asset list from selected actor
					RefreshAssetsForSelectedActor();
				})
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildAssetList()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4, 4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("VoiceAssetsHeader", "Voice Assets for Selected Actor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
		[
			SAssignNew(VoiceAssetListView, SListView<TSharedPtr<FLocalizedVoiceAssetDisplayData>>)
			.ListItemsSource(&VoiceAssetCards)
			.SelectionMode(ESelectionMode::None)
			.OnGenerateRow_Lambda([](TSharedPtr<FLocalizedVoiceAssetDisplayData> Item,
			                         const TSharedRef<STableViewBase>& Owner)
			{
				return SNew(STableRow<TSharedPtr<FLocalizedVoiceAssetDisplayData>>, Owner)
					[
						SNew(SOverlay)
						+ SOverlay::Slot().Padding(0, 4)
						[
							SNew(SBorder)
							.Padding(0)
							.BorderImage(FEditorStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.3f, 0.25f, 0.0f))
							// Fond opaque
							[
								SNew(SOverlay)

								// Main Content
								+ SOverlay::Slot()
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot().AutoHeight().Padding(8)
									[
										SNew(SHorizontalBox)

										// Nom de l'asset
										+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(FText::FromString(Item->AssetName))
											.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
										]

										// Infos culture
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0, 4, 0)
										[
											SNew(STextBlock)
											.Text(FText::Format(
												LOCTEXT("CultureInfo", "{0}/{1} ({2})"),
												FText::AsNumber(Item->AvailableCultures.Num()),
												FText::AsNumber(Item->TotalCultures),
												FText::FromString(FString::Join(Item->AvailableCultures, TEXT(", ")))
											))
										]
									]
								]

								// Colored bar at the bottom
								+ SOverlay::Slot()
								.VAlign(VAlign_Bottom)
								[
									SNew(SBorder)
									.Padding(FMargin(0.f))
									.BorderImage(FEditorStyle::GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(1.f, 0.78f, 0.f, 1.f)) // Ton type color
									[
										SNew(SBox)
										.HeightOverride(4.f)
									]
								]
							]
						]
					];
			})
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildCultureListWidget()
{
	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);

	for (const FSSVoiceCultureReportEntry& Entry : CultureReport.Entries)
	{
		const float Progress = Entry.GetCoveragePercent();

		List->AddSlot().AutoHeight().Padding(4)
		[
			SNew(SHorizontalBox)

			// Culture name
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.Culture))
				.MinDesiredWidth(60)
			]

			// Progress bar
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(4)
			[
				SNew(SProgressBar)
				.Percent(Progress)
			]

			// Percentage label
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					LOCTEXT("CoverageSummaryWithPercent", "{0}%  ({1} / {2})"),
					FText::AsNumber(FMath::RoundToInt(Progress * 100.f)),
					FText::AsNumber(Entry.AssetsWithCulture),
					FText::AsNumber(Entry.TotalAssets)
				))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.MinDesiredWidth(70)
			]

			// Autofill button
			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[
				SNew(SButton)
				.Text(LOCTEXT("AutofillCultureBtn", "Auto-fill"))
				.OnClicked_Lambda([this, Culture = Entry.Culture]()
				{
					// TODO : lancer l’autofill ciblé
					UE_LOG(LogTemp, Log, TEXT("[SSVoice] Requested autofill for culture: %s"), *Culture);

					OpenAutoFillConfirmationDialog(Culture);

					return FReply::Handled();
				})
			]
		];
	}

	return List;
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildCoverageSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CultureCoverageTitle", "Voice Culture Coverage"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RegenerateReportBtn", "Generate Report"))
				.ToolTipText(LOCTEXT("RegenerateReportTooltip",
				                     "Scan all voice assets and generate the latest culture coverage report."))
				.OnClicked_Lambda([this]()
				{
					// 1. Créer le slow task pour l’utilisateur
					TSharedPtr<FScopedSlowTask> SlowTask = MakeShared<FScopedSlowTask>(
						1.f,
						LOCTEXT("ScanningCultures", "Scanning voice cultures...")
					);
					SlowTask->MakeDialog(true);

					// 2. Lancer l'opération async
					FSSVoiceLocalizationUtils::GenerateCultureCoverageReportAsync(
						[this, SlowTask](const FSSVoiceCultureReport& Report)
						{
							CultureReport = Report;

							// Fermer la tâche + reconstruire UI
							if (SlowTask.IsValid())
							{
								SlowTask->EnterProgressFrame(1.f); // facultatif ici
							}

							FSSVoiceLocalizationUI::NotifySuccess(
								NSLOCTEXT("SSVoice", "ScanningCulturesSuccess", "Scanning cultures with success"));
							
							RebuildUI();
						}
					);

					return FReply::Handled();
				})
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				// Loop on each culture
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					BuildCultureListWidget()
				]
			]
		];
}

void SSSVoiceDashboard::OpenAutoFillConfirmationDialog(const FString& Culture)
{
	FText Title = FText::Format(
		LOCTEXT("ConfirmAutoFillTitle", "Auto-fill Culture: {0}"),
		FText::FromString(Culture.ToUpper())
	);

	FText Message = FText::Format(
		LOCTEXT("ConfirmAutoFillText",
		        "Are you sure you want to auto-fill all voice assets for culture '{0}'?\nThis operation cannot be undone."),
		FText::FromString(Culture)
	);

	FSSVoiceLocalizationUI::ConfirmDialog(Title, Message, FSimpleDelegate::CreateLambda([this, Culture]()
	{
		// Prochaine étape : Exécuter l'autofill ciblé ici
		UE_LOG(LogTemp, Log, TEXT("[SSVoice] Auto-fill confirmed for culture: %s"), *Culture);

		FSSVoiceLocalizationUtils::AutoFillCultureAsync(
			Culture, /* bOverrideExisting = */ false,
			[this](int32 ModifiedCount)
			{
				UE_LOG(LogTemp, Log, TEXT("[SSVoice] Modified voice: %d"), ModifiedCount);

				// Optionnel : Refresh Dashboard
				RebuildUI();
			}
		);
	}));
}

void SSSVoiceDashboard::LoadActorListFromJson()
{
	AllActorItems.Empty();

	const FString JsonPath = FPaths::ProjectSavedDir() / TEXT("SSVoiceLocalization/VoiceActors.json");
	FString JsonRaw;

	if (!FFileHelper::LoadFileToString(JsonRaw, *JsonPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Failed to read actor list JSON at %s"), *JsonPath);
		return;
	}

	TSharedPtr<FJsonValue> RootValue;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);

	if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Failed to parse actor list JSON"));
		return;
	}

	if (RootValue->Type != EJson::Array)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Expected root JSON to be an array"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>& JsonArray = RootValue->AsArray();
	for (const TSharedPtr<FJsonValue>& Value : JsonArray)
	{
		if (Value.IsValid() && Value->Type == EJson::String)
		{
			AllActorItems.Add(MakeShared<FString>(Value->AsString()));
		}
	}

	// Call the search filter result
	RefreshActorFilter();

	if (ActorListView.IsValid())
	{
		ActorListView->RequestListRefresh();
	}
}

void SSSVoiceDashboard::RefreshActorFilter()
{
	FilteredActorItems.Empty();

	for (const TSharedPtr<FString>& Item : AllActorItems)
	{
		if (ActorSearchFilter.IsEmpty() || Item->Contains(ActorSearchFilter, ESearchCase::IgnoreCase))
		{
			FilteredActorItems.Add(Item);
		}
	}

	if (ActorListView.IsValid())
	{
		ActorListView->RequestListRefresh();
	}
}

void SSSVoiceDashboard::RefreshAssetsForSelectedActor()
{
	VoiceAssetCards.Empty();

	if (!SelectedActor.IsValid())
		return;

	const FString& TargetActor = *SelectedActor;
	const USSVoiceLocalizationSettings* Settings = USSVoiceLocalizationSettings::GetSetting();
	const TSet<FString> AllCultures = Settings ? Settings->SupportedVoiceCultures : TSet<FString>();

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(USSLocalizedVoiceSound::StaticClass()->GetFName());
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add("/Game");

	TArray<FAssetData> Assets;
	AssetRegistry.Get().GetAssets(Filter, Assets);

	for (const FAssetData& AssetData : Assets)
	{
		const FString Name = AssetData.AssetName.ToString();

		if (!Name.Contains(TargetActor))
			continue;

		// Culture info via tag
		TArray<FString> Cultures;
		const FAssetTagValueRef TagValueRef = AssetData.TagsAndValues.FindTag("VoiceCultures");
		if (TagValueRef.IsSet())
		{
			TagValueRef.GetValue().ParseIntoArray(Cultures, TEXT(","));
		}

		TSharedPtr<FLocalizedVoiceAssetDisplayData> Entry = MakeShared<FLocalizedVoiceAssetDisplayData>();
		Entry->AssetPath = AssetData.ObjectPath.ToString(); // ← on garde le chemin, on chargera plus tard si besoin
		Entry->AssetName = Name;
		Entry->AvailableCultures = Cultures;
		Entry->TotalCultures = AllCultures.Num();

		VoiceAssetCards.Add(Entry);
	}

	if (VoiceAssetListView.IsValid())
	{
		VoiceAssetListView->RequestListRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
