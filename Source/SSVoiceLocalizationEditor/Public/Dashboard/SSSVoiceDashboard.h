// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserDelegates.h"
#include "SSVoiceLocalizationTypes.h"


class FSSVoiceFilterCompleteCulture;
class FSSVoiceFilterMissingCulture;
class FSSVoiceFilterActorName;

class SSVOICELOCALIZATIONEDITOR_API SSSVoiceDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSSVoiceDashboard) {}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedPtr<SWindow>& OwningWindow, const TSharedRef<SDockTab>& OwningTab);

protected:
	/** Last loaded report */
	FSSVoiceCultureReport CultureReport;
	
private:
	static const FName OverviewTabId;
	static const FName VoiceActorTabName;
	
	TSharedPtr<FTabManager> TabManager;

	TSharedRef<SDockTab> SpawnDashboardTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnVoiceActorTab(const FSpawnTabArgs& SpawnTabArgs);
	
	TSharedRef<SWidget> BuildLeftPanel();
	TSharedRef<SWidget> BuildRightPanel();
	
	TSharedRef<SWidget> BuildCultureListWidget();
	TSharedRef<SWidget> BuildCoverageSection();
	
	////////////////////////////////////////////////////////////////////
	// Voice actor tab part
	struct FLocalizedVoiceAssetDisplayData
	{
		FString AssetName;
		FString AssetPath;
		TArray<FString> AvailableCultures;
		int32 TotalCultures = 0;
	};
	
	FARFilter Filter;
	FSetARFilterDelegate DelegateFilter;
	FRefreshAssetViewDelegate DelegateRefreshView;
	FGetCurrentSelectionDelegate DelegateSelection;
	
	TArray<FAssetData> AssetsVoiceActor;
	
	TSharedPtr<FSSVoiceFilterActorName> FilterActorName;
	TSharedPtr<FSSVoiceFilterMissingCulture> FilterMissingCulture;
	TSharedPtr<FSSVoiceFilterCompleteCulture> FilterCompleteCulture;
	
	TSharedRef<SWidget> BuildActorList();
	TSharedRef<SWidget> BuildAssetList();
	
	TSharedRef<ITableRow> GenerateActorRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	
	TSharedRef<ITableRow> GenerateVoiceSoundAssetRow(TSharedPtr<FLocalizedVoiceAssetDisplayData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	void ResetVoiceAssetsCache();
	void UpdateContentBrowser();
	void LoadActorListFromJson();
	
	// List of actor names extracted from LocalizedVoiceSound asset names
	TArray<TSharedPtr<FString>> AllActorItems;
	TArray<TSharedPtr<FString>> FilteredActorItems; // Liste filtrée
	
	// Widget instance for refreshing the list view when data changes
	TSharedPtr<SListView<TSharedPtr<FString>>> ActorListView;

	// Currently selected actor from the list
	TSharedPtr<FString> SelectedActor;
	
	// Search Actor
	TSharedPtr<SSearchBox> ActorSearchBox;
	FString ActorSearchFilter;

	void RefreshActorFilter();

	// Asset list côté droit
	TArray<TSharedPtr<FAssetData>> FilteredAssetsForActor;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;
	
	void RefreshAssetsForSelectedActor();
	
protected:

	void RebuildUI();
	
	void OpenAutoFillConfirmationDialog(const FString& Culture);
	
};
