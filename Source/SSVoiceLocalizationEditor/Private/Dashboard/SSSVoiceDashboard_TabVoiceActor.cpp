/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "EditorStyleSet.h"
#include "IContentBrowserSingleton.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "SSVoiceLocalizationFilters.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dashboard/SSSVoiceDashboard.h"

#include "Utils/SSVoiceLocalizationUtils.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SSVoiceLocalization"

TSharedRef<SDockTab> SSSVoiceDashboard::SpawnVoiceActorTab(const FSpawnTabArgs& SpawnTabArgs)
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
				.Text(LOCTEXT("ActorsHeader", "Voice Actors"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
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
			.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
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

TSharedRef<ITableRow> SSSVoiceDashboard::GenerateVoiceSoundAssetRow(TSharedPtr<FLocalizedVoiceAssetDisplayData> InItem,
                                                                    const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FLocalizedVoiceAssetDisplayData>>, OwnerTable)
		[
			// Drop shadow border
			SNew(SBorder)
			.Padding(1.5f)
			.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
			.BorderBackgroundColor(FLinearColor::White.CopyWithNewOpacity(0.4))
			[

				SNew(SOverlay)
				+ SOverlay::Slot().Padding(0, 4)
				[
					SNew(SBorder)
					.Padding(0)
					.BorderImage(FEditorStyle::GetBrush("WhiteBrush"))
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
										LOCTEXT("CultureInfo", "{0}/{1} ({2})"),
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
							.BorderImage(FEditorStyle::GetBrush("WhiteBrush"))
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


FReply SSSVoiceDashboard::OnClick_AutoFillMissingCulture()
{
	FSSVoiceLocalizationUtils::AutoPopulateFromVoiceActor(FilterActorName->GetVoiceActorName());
	
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

	const TSharedPtr<FFrontendFilterCategory> DefaultCategory = MakeShareable(
		new FFrontendFilterCategory(
			FText::FromString(TEXT("SSVoiceLocalization Filters")),
			FText::FromString(TEXT(""))
		)
	);

	FilterActorName = MakeShareable(new FSSVoiceFilterActorName(DefaultCategory));
	FilterMissingCulture = MakeShareable(new FSSVoiceFilterMissingCulture(DefaultCategory));
	FilterCompleteCulture = MakeShareable(new FSSVoiceFilterCompleteCulture(DefaultCategory));

	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterActorName.ToSharedRef());
	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterMissingCulture.ToSharedRef());
	AssetPickerConfig.ExtraFrontendFilters.Emplace(FilterCompleteCulture.ToSharedRef());

	const auto ContentBrowserView = USSVoiceLocalizationEditorSubsystem::GetVoiceContentBrowser().Get().
		CreateAssetPicker(AssetPickerConfig);
	
	// Force activate the filter
	FilterActorName->SetActive(true);

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4, 4, 4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("VoiceAssetsHeader", "Voice Assets for Selected Actor"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("RegenerateReportBtn", "Auto Fill Missing Culture"))
			.ToolTipText(LOCTEXT("AutoFillAssetsInBrowserTooltip",
			                     "Auto populate all voice assets from content browser"))
			.OnClicked(this, &SSSVoiceDashboard::OnClick_AutoFillMissingCulture)
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
	UE_LOG(LogTemp, Display, TEXT("Voice actor sound assets scanned in %.2f seconds."), ScanTime);
}

#undef LOCTEXT_NAMESPACE
