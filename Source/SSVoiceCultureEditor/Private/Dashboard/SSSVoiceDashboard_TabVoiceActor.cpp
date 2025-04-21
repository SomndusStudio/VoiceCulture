/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "EditorStyleSet.h"
#include "IContentBrowserSingleton.h"
#include "SSVoiceCultureEditorCommands.h"
#include "SSVoiceCultureEditorSubsystem.h"
#include "SSVoiceCultureEditorFilters.h"
#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceStyleCompat.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dashboard/SSSVoiceDashboard.h"
#include "Editor/ContentBrowser/Private/AssetContextMenu.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Utils/SSVoiceCultureUI.h"

#include "Utils/SSVoiceCultureUtils.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

namespace SSVoiceCultureOpenAssetDialogUtils
{
	static const FName ContextMenuName("SSVoiceCultureOpenAssetDialog.ContextMenu");
}

TSharedRef<SDockTab> SSSVoiceDashboard::SpawnVoiceActorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(3.0f)
			.Style(SSVoiceStyleCompat::Get(), "DetailsView.Splitter")
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().Padding(10.f)
				[
					BuildActorList()
				]
			]
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().Padding(10.f)
				[
					BuildAssetList()
				]
			]
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildActorList()
{
	return SNew(SVerticalBox)
			// Titre
			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("SSVoiceCultureEditor", "ActorsHeader", "Voice Actors"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			// Rescan Button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				SNew(SButton)
				.Text(FText::FromString("Rescan Actors"))
				.OnClicked(this, &SSSVoiceDashboard::OnClick_RescanActors)
			]
			// Barre de recherche
			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SAssignNew(ActorSearchBox, SSearchBox)
				.HintText(NSLOCTEXT("SSVoiceCultureEditor", "ActorSearchHint", "Search Voice Actor..."))
				.OnTextChanged_Lambda([this](const FText& NewText)
				{
					ActorSearchFilter = NewText.ToString();
					RefreshActorFilter();
				})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(3.0f)
			[
				SNew(SSeparator).Thickness(5.0f)
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

TSharedRef<ITableRow> SSSVoiceDashboard::GenerateActorRow(TSharedPtr<FString> InItem,
                                                          const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(SBorder)
			.Padding(2.f)
			.BorderImage(SSVoiceStyleCompat::GetBrush("ContentBrowser.ThumbnailShadow"))
			.BorderBackgroundColor(FLinearColor::White.CopyWithNewOpacity(0.4f))
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
					.Padding(FMargin(8, 8))
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.8f, 0.3f, 1.0f, 0.4f)) // Fond violet-rose plus contrasté
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

////////////////////////////////////////////////////////////////////
// Asset List

TSharedRef<ITableRow> SSSVoiceDashboard::GenerateVoiceSoundAssetRow(TSharedPtr<FVoiceCultureAssetDisplayData> InItem,
                                                                    const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FVoiceCultureAssetDisplayData>>, OwnerTable)
		[
			// Drop shadow border
			SNew(SBorder)
			.Padding(1.5f)
			.BorderImage(SSVoiceStyleCompat::GetBrush("ContentBrowser.ThumbnailShadow"))
			.BorderBackgroundColor(FLinearColor::White.CopyWithNewOpacity(0.4))
			[

				SNew(SOverlay)
				+ SOverlay::Slot().Padding(0, 4)
				[
					SNew(SBorder)
					.Padding(0)
					.BorderImage(SSVoiceStyleCompat::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.31f, 0.27f, 0.08f, 0.5f))
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
									.Text(FText::FromString(InItem->AssetName))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
								]

								// Infos culture
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(
									4, 0, 4, 0)
								[
									SNew(STextBlock)
									.Text(FText::Format(
										NSLOCTEXT("SSVoiceCultureEditor", "CultureInfo", "{0}/{1} ({2})"),
										FText::AsNumber(InItem->AvailableCultures.Num()),
										FText::AsNumber(InItem->TotalCultures),
										FText::FromString(
											FString::Join(InItem->AvailableCultures, TEXT(", ")))
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
							.BorderImage(SSVoiceStyleCompat::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(1.f, 0.78f, 0.f, 1.f)) // Ton type color
							[
								SNew(SBox)
								.HeightOverride(4.f)
							]
						]
					]
				]
			]
		];
}

void SSSVoiceDashboard::UpdateContentBrowser()
{
	DelegateFilter.Execute(Filter);
}

TSharedPtr<SWidget> SSSVoiceDashboard::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	if (SelectedAssets.IsEmpty())
	{
		return nullptr;
	}

	AssetPickerSelectedAssets = SelectedAssets;
	
	return UToolMenus::Get()->GenerateWidget(SSVoiceCultureOpenAssetDialogUtils::ContextMenuName,
	                                         FToolMenuContext(AssetPickerCommands));
}

bool SSSVoiceDashboard::AreAnyAssetsSelected() const
{
	return !DelegateSelection.Execute().IsEmpty();
}

FReply SSSVoiceDashboard::OnClick_AutoPopulateMissingCulture()
{
	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		NSLOCTEXT("SSVoiceCultureEditor", "ConfirmAutoPopulate", "Are you sure you want to auto-populate this culture?")
	);
	if (Result == EAppReturnType::Yes)
	{
		FSSVoiceCultureUtils::AutoPopulateFromVoiceActor(FilterActorName->GetVoiceActorName());
	}

	return FReply::Handled();
}

FReply SSSVoiceDashboard::OnClick_RescanActors()
{
	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		NSLOCTEXT("SSVoiceCultureEditor", "ConfirmScanVoiceActorName",
		          "Are you sure you want to rescan all voice actor name ?\nThis operation cannot be undone.")
	);

	if (Result == EAppReturnType::Yes)
	{
		// Schedule the generation to run after the current UI event finishes
		FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
		{
			// Show progress UI
			FScopedSlowTask
				SlowTask(3.f, NSLOCTEXT("SSVoiceCultureEditor", "RescanningActors", "Rescanning actors..."));
			SlowTask.MakeDialog(true);

			// 1. Generate the actor list
			SlowTask.EnterProgressFrame(1.f, NSLOCTEXT("SSVoiceCultureEditor", "GeneratingJson", "Generating JSON..."));
			FSSVoiceCultureUtils::GenerateActorListJson();

			// 2. Load it back into UI or memory
			SlowTask.EnterProgressFrame(1.f, NSLOCTEXT("SSVoiceCultureEditor", "LoadingJson", "Loading actor list..."));

			// Assuming this is called in context where 'this' is valid (SWidget or similar)
			this->LoadActorListFromJson();

			// 3. Done
			SlowTask.EnterProgressFrame(1.f, NSLOCTEXT("SSVoiceCultureEditor", "Complete", "Done."));

			FSSVoiceCultureUI::NotifySuccess(NSLOCTEXT("SSVoiceCultureEditor", "ScanVoiceNameComplete",
			                                           "Actor voice scan completed."));
		}, TStatId(), nullptr, ENamedThreads::GameThread);
	}
	return FReply::Handled();
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildAssetList()
{
	AssetPickerConfig.bAddFilterUI = true;
	AssetPickerConfig.bCanShowFolders = false;
	AssetPickerConfig.bAllowDragging = false;
	AssetPickerConfig.SelectionMode = ESelectionMode::Multi;
	AssetPickerConfig.bShowBottomToolbar = true;
	AssetPickerConfig.bCanShowDevelopersFolder = true;
	AssetPickerConfig.bForceShowEngineContent = false;
	AssetPickerConfig.bForceShowPluginContent = false;
	AssetPickerConfig.bCanShowClasses = false;
	AssetPickerConfig.bCanShowRealTimeThumbnails = false;

	AssetPickerConfig.Filter = Filter;
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.GetCurrentSelectionDelegates.Add(&DelegateSelection);
	AssetPickerConfig.RefreshAssetViewDelegates.Add(&DelegateRefreshView);
	AssetPickerConfig.SetFilterDelegates.Add(&DelegateFilter);

	FSSVoiceCultureEditorEditorCommonCommands::Register();
	const auto& Commands = FSSVoiceCultureEditorEditorCommonCommands::Get();
	
	if (!UToolMenus::Get()->IsMenuRegistered(SSVoiceCultureOpenAssetDialogUtils::ContextMenuName))
	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu(SSVoiceCultureOpenAssetDialogUtils::ContextMenuName);

		FToolMenuSection& Section = Menu->AddSection("VoiceAsset",
		                                             NSLOCTEXT("SSVoiceCultureEditor", "VoiceAssetSection",
		                                                       "Voice Actions"));
		Section.AddMenuEntry(Commands.BrowseToAsset);
		Section.AddMenuEntry(Commands.AutoPopulate);
	}
	
	AssetPickerCommands = MakeShared<FUICommandList>();
	
	AssetPickerCommands->MapAction(
		Commands.BrowseToAsset,
		FExecuteAction::CreateSP(SharedThis(this), &SSSVoiceDashboard::BrowseToSelectedAssets)
	);

	AssetPickerCommands->MapAction(
		Commands.AutoPopulate,
		FExecuteAction::CreateSP(SharedThis(this), &SSSVoiceDashboard::AutoPopulateSelectedAssets)
	);

	AssetPickerConfig.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(
		this, &SSSVoiceDashboard::OnGetAssetContextMenu);

	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetSelected::CreateLambda([](const FAssetData& AssetData)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
	});

	const TSharedPtr<FFrontendFilterCategory> DefaultCategory = MakeShareable(
		new FFrontendFilterCategory(
			NSLOCTEXT("SSVoiceCultureEditor", "SSVoiceCulture_Filters", "SSVoiceCulture Filters"),
			FText::FromString(TEXT(""))
		)
	);

	FilterActorName = MakeShareable(new FSSVoiceFilterActorName(DefaultCategory));
	FilterMissingCulture = MakeShareable(new FSSVoiceFilterMissingCulture(DefaultCategory));
	FilterCompleteCulture = MakeShareable(new FSSVoiceFilterCompleteCulture(DefaultCategory));

	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterActorName.ToSharedRef());
	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterMissingCulture.ToSharedRef());
	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterCompleteCulture.ToSharedRef());

	const auto ContentBrowserView = USSVoiceCultureEditorSubsystem::GetVoiceContentBrowser().Get().
		CreateAssetPicker(AssetPickerConfig);

	// Force activate the filter
	FilterActorName->SetActive(true);

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4, 4, 4)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("SSVoiceCultureEditor", "VoiceAssetsHeader", "Voice Assets for Selected Actor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5.0f)
		[
			SNew(SButton)
			.Text(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateMissingCultures", "Auto Populate Missing Cultures"))
			.ToolTipText(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateAssetsInBrowserTooltip",
			                     "Auto populate all voice assets from content browser"))
			.OnClicked(this, &SSSVoiceDashboard::OnClick_AutoPopulateMissingCulture)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(3.0f)
		[
			SNew(SSeparator).Thickness(5.0f)
		]
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(5.0f)
		[
			ContentBrowserView
		];
}

void SSSVoiceDashboard::BrowseToSelectedAssets()
{
	// Ex: synchroniser Content Browser avec les assets sélectionnés
	TArray<UObject*> Objects;
	for (const FAssetData& Asset : AssetPickerSelectedAssets)
	{
		if (UObject* Obj = Asset.GetAsset())
		{
			Objects.Add(Obj);
		}
	}
	GEditor->SyncBrowserToObjects(Objects);
}

void SSSVoiceDashboard::AutoPopulateSelectedAssets()
{
	auto* Subsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
	if (!Subsystem) return;

	for (const FAssetData& Asset : AssetPickerSelectedAssets)
	{
		if (USSVoiceCultureSound* VoiceAsset = Cast<USSVoiceCultureSound>(Asset.GetAsset()))
		{
			FSSVoiceCultureUtils::AutoPopulateFromNaming(VoiceAsset, true, true);
		}
	}
}

void SSSVoiceDashboard::RefreshAssetsForSelectedActor()
{
	if (!SelectedActor.IsValid())
		return;

	const FString& TargetActor = *SelectedActor;

	// --
	const double ScanStartTime = FPlatformTime::Seconds();

	// Execute filter retrieve assets
	FilterActorName->SetVoiceActorName(TargetActor);
	FilterActorName->UpdateData();
	FilterMissingCulture->UpdateVoiceAssets(FilterActorName->GetAssets());
	FilterCompleteCulture->UpdateVoiceAssets(FilterActorName->GetAssets());

	// Refresh content browser from filter
	UpdateContentBrowser();

	const double ScanTime = FPlatformTime::Seconds() - ScanStartTime;
	UE_LOG(LogVoiceCultureEditor, Log, TEXT("Voice actor sound assets scanned in %.2f seconds."), ScanTime);
}

#undef LOCTEXT_NAMESPACE
