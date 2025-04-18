// Fill out your copyright notice in the Description page of Project Settings.


#include "Dashboard/SSSVoiceDashboard.h"

#include "SSVoiceLocalizationFilters.h"
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
	
	// Instantiated TabManager layout in memory (non attaché à un level editor)
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

	
	// Create the TabManager
	TabManager = FGlobalTabmanager::Get()->NewTabManager(OwningTab);

	// Tab Spawners
	TabManager->RegisterTabSpawner(OverviewTabId, FOnSpawnTab::CreateSP(this, &SSSVoiceDashboard::SpawnDashboardTab))
		.SetDisplayName(FText::FromString("Overview"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	TabManager->RegisterTabSpawner(VoiceActorTabName, FOnSpawnTab::CreateSP(this, &SSSVoiceDashboard::SpawnVoiceActorTab))
	.SetDisplayName(FText::FromString("Voice Actors"))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	// Generate whole layout
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

void SSSVoiceDashboard::RebuildUI()
{
	// this->Construct(SSSVoiceDashboard::FArguments());
	Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
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

#undef LOCTEXT_NAMESPACE
