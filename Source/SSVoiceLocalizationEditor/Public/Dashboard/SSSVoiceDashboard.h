// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceLocalizationTypes.h"


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
	
	TSharedRef<SWidget> BuildAssetSection();

	TSharedRef<SWidget> BuildActorList();
	TSharedRef<SWidget> BuildAssetList();
	
	TSharedRef<ITableRow> GenerateActorRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	
protected:

	void RebuildUI();
	
	void OpenAutoFillConfirmationDialog(const FString& Culture);

	// List of actor names extracted from LocalizedVoiceSound asset names
	TArray<TSharedPtr<FString>> AllActorItems;
	TArray<TSharedPtr<FString>> FilteredActorItems; // Liste filtrée
	
	// Widget instance for refreshing the list view when data changes
	TSharedPtr<SListView<TSharedPtr<FString>>> ActorListView;

	// Currently selected actor from the list
	TSharedPtr<FString> SelectedActor;
	
	void LoadActorListFromJson();

	// Search Actor
	TSharedPtr<SSearchBox> ActorSearchBox;
	FString ActorSearchFilter;

	void RefreshActorFilter();

	// Asset list côté droit
	TArray<TSharedPtr<FAssetData>> FilteredAssetsForActor;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;

	// Chargement
	struct FLocalizedVoiceAssetDisplayData
	{
		FString AssetName;
		FString AssetPath;
		TArray<FString> AvailableCultures;
		int32 TotalCultures = 0;
	};
	TArray<TSharedPtr<FLocalizedVoiceAssetDisplayData>> VoiceAssetCards;
	TSharedPtr<SListView<TSharedPtr<FLocalizedVoiceAssetDisplayData>>> VoiceAssetListView;
	
	void RefreshAssetsForSelectedActor();
};
