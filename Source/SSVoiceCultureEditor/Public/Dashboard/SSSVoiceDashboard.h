/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserDelegates.h"
#include "IContentBrowserSingleton.h"
#include "SSVoiceCultureEditorTypes.h"


class SSSVoiceEditorProfileSelector;
class FSSVoiceFilterCompleteCulture;
class FSSVoiceFilterMissingCulture;
class FSSVoiceFilterActorName;

class SSVOICECULTUREEDITOR_API SSSVoiceDashboard : public SCompoundWidget
{
public:

	// ------------------------
	// Slate Setup
	// ------------------------

	SLATE_BEGIN_ARGS(SSSVoiceDashboard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<SWindow>& OwningWindow, const TSharedRef<SDockTab>& OwningTab);

protected:
	// ------------------------
	// Display Data Structs
	// ------------------------

	/** Display data for the voice asset list (right panel) */
	struct FLocalizedVoiceAssetDisplayData
	{
		FString AssetName;
		FString AssetPath;
		TArray<FString> AvailableCultures;
		int32 TotalCultures = 0;
	};
	
	// ------------------------
	// Culture Report State
	// ------------------------

	/** Last loaded culture report */
	FSSVoiceCultureReport CultureReport;

	// ------------------------
	// UI Composition
	// ------------------------

	TSharedRef<SWidget> BuildLeftPanel();
	TSharedRef<SWidget> BuildRightPanel();

	// ------------------------
	// Profile Section (top left)
	// ------------------------
	
	TSharedPtr<SSSVoiceEditorProfileSelector> ProfileSelector;
	TSharedRef<SWidget> BuildProfileSectionWidget();

	// ------------------------
	// Coverage Section (middle left)
	// ------------------------

	TArray<TSharedPtr<FSSVoiceCultureReportEntry>> CultureListData;
	TSharedPtr<SListView<TSharedPtr<FSSVoiceCultureReportEntry>>> CultureListView;

	TSharedRef<SWidget> BuildCoverageSection();
	TSharedRef<SWidget> BuildCultureListWidget();
	TSharedRef<ITableRow> OnGenerateCultureRow(TSharedPtr<FSSVoiceCultureReportEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable);
	FReply OnGenerateReportClicked();
	
	void RefreshCoverageSection();
	// ------------------------
	// Button Events (top left)
	// ------------------------

	FReply OnClick_AutoPopulateMissingCulture();
	FReply OnClick_RescanActors();

	// ------------------------
	// Voice Actor Tab (right panel)
	// ------------------------

	TSharedRef<SWidget> BuildActorList();
	TSharedRef<SWidget> BuildAssetList();
	TSharedRef<ITableRow> GenerateActorRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateVoiceSoundAssetRow(TSharedPtr<FLocalizedVoiceAssetDisplayData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TArray<TSharedPtr<FString>> AllActorItems;
	TArray<TSharedPtr<FString>> FilteredActorItems;
	TSharedPtr<SListView<TSharedPtr<FString>>> ActorListView;

	TSharedPtr<FString> SelectedActor;

	TSharedPtr<SSearchBox> ActorSearchBox;
	FString ActorSearchFilter;

	TArray<TSharedPtr<FAssetData>> FilteredAssetsForActor;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;

	void RefreshActorFilter();
	void RefreshAssetsForSelectedActor();
	void LoadActorListFromJson();

	// ------------------------
	// Voice Actor Tab - Filters
	// ------------------------

	FARFilter Filter;
	FSetARFilterDelegate DelegateFilter;
	FRefreshAssetViewDelegate DelegateRefreshView;
	FGetCurrentSelectionDelegate DelegateSelection;

	TSharedPtr<FSSVoiceFilterActorName> FilterActorName;
	TSharedPtr<FSSVoiceFilterMissingCulture> FilterMissingCulture;
	TSharedPtr<FSSVoiceFilterCompleteCulture> FilterCompleteCulture;

	// ------------------------
	// Voice Actor Tab - Display Data
	// ------------------------

	// Asset picker config (setup rules, filters, etc.)
	FAssetPickerConfig AssetPickerConfig;

	// Commands and menu logic
	TSharedPtr<FUICommandList> AssetPickerCommands;
	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& AssetDatas);
	TArray<FAssetData> AssetPickerSelectedAssets;

	// UI actions for selected assets
	bool AreAnyAssetsSelected() const;
	void BrowseToSelectedAssets();
	void AutoPopulateSelectedAssets();

	// Asset browser update
	void UpdateContentBrowser();
	
	// ------------------------
	// UI Logic & Misc
	// ------------------------

	USSVoiceCultureStrategy* GetStrategy() const;
	void OpenAutoPopulateConfirmationDialog(const FString& Culture);

	// ------------------------
	// Toolbar
	// ------------------------
	TSharedRef<SWidget> BuildToolbar();
	
	// ------------------------
	// Tab Management
	// ------------------------

	TSharedPtr<FTabManager> TabManager;
	TSharedRef<SDockTab> SpawnDashboardTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnVoiceActorTab(const FSpawnTabArgs& SpawnTabArgs);

	static const FName OverviewTabId;
	static const FName VoiceActorTabName;

	// ------------------------
	// Localized Info (Bottom UI)
	// ------------------------

	FText GetLocalizedVoiceText() const;
	FText GetLocalizedVoiceExampleText() const;
	FText GetMatchCulturePatternText() const;
	FText GetMatchCulturePatternExampleText() const;
};
