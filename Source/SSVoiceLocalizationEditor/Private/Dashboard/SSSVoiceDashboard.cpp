/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Dashboard/SSSVoiceDashboard.h"

#include "EditorStyleSet.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "SSVoiceLocalizationFilters.h"
#include "SSVoiceLocalizationSettings.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/JsonSerializer.h"
#include "Utils/SSVoiceLocalizationUI.h"
#include "Utils/SSVoiceLocalizationUtils.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"

#define LOCTEXT_NAMESPACE "SSVoiceLocalization"

const FName SSSVoiceDashboard::OverviewTabId("Overview");
const FName SSSVoiceDashboard::VoiceActorTabName("Voice Actors");

void SSSVoiceDashboard::Construct(const FArguments& InArgs, const TSharedPtr<SWindow>& OwningWindow,
                                  const TSharedRef<SDockTab>& OwningTab)
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

	TabManager->RegisterTabSpawner(VoiceActorTabName,
	                               FOnSpawnTab::CreateSP(this, &SSSVoiceDashboard::SpawnVoiceActorTab))
	          .SetDisplayName(FText::FromString("Voice Actors"))
	          .SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	// Generate whole layout
	ChildSlot
	[
		TabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef()
	];
}

USSVoiceAutofillStrategy* SSSVoiceDashboard::GetStrategy() const
{
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
	return VLEditorSubsystem->GetActiveStrategy();
}

void SSSVoiceDashboard::RebuildUI()
{
	// this->Construct(SSSVoiceDashboard::FArguments());
	Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
}

FText SSSVoiceDashboard::GetLocalizedVoiceText() const
{
	return GetStrategy()->DisplayMatchLocalizedVoicePattern();
}

FText SSSVoiceDashboard::GetLocalizedVoiceExampleText() const
{
	return GetStrategy()->DisplayMatchLocalizedVoicePatternExample();
}

FText SSSVoiceDashboard::GetMatchCulturePatternText() const
{
	return GetStrategy()->DisplayMatchCultureRulePattern();
}

FText SSSVoiceDashboard::GetMatchCulturePatternExampleText() const
{
	return GetStrategy()->DisplayMatchCultureRulePatternExample();
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
