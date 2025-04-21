/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Dashboard/SSSVoiceDashboard.h"

#include "EditorStyleSet.h"
#include "SSVoiceCultureEditorSubsystem.h"
#include "SSVoiceCultureEditorFilters.h"
#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureSettings.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/JsonSerializer.h"
#include "Slate/SSVoiceCultureSlateComponents.h"
#include "Utils/SSVoiceCultureUI.h"
#include "Utils/SSVoiceCultureUtils.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

const FName SSSVoiceDashboard::OverviewTabId("Overview");
const FName SSSVoiceDashboard::VoiceActorTabName("Voice Actors");

void SSSVoiceDashboard::Construct(const FArguments& InArgs, const TSharedPtr<SWindow>& OwningWindow,
                                  const TSharedRef<SDockTab>& OwningTab)
{
	// Load report
	FSSVoiceCultureUtils::LoadSavedCultureReport(CultureReport);

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
		SNew(SVerticalBox)

		// --- Toolbar
		+ SVerticalBox::Slot().AutoHeight().Padding(4, 4)
		[
			BuildToolbar()
		]
		// --- Tabs
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			TabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef()
		]
	];

	RefreshCoverageSection();
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildToolbar()
{
	FToolBarBuilder ToolbarBuilder(TSharedPtr<FUICommandList>(), FMultiBoxCustomization::None);
	
	ToolbarBuilder.AddSeparator();
	
	ToolbarBuilder.AddWidget(
		SNew(SSSVoiceCulturePreviewLanguageSelector)
	);

	return ToolbarBuilder.MakeWidget();
}

USSVoiceCultureStrategy* SSSVoiceDashboard::GetStrategy() const
{
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
	return VLEditorSubsystem->GetActiveStrategy();
}

FText SSSVoiceDashboard::GetVoiceCultureText() const
{
	return GetStrategy()->DisplayMatchVoiceCulturePattern();
}

FText SSSVoiceDashboard::GetVoiceCultureExampleText() const
{
	return GetStrategy()->DisplayMatcVoiceCulturePatternExample();
}

FText SSSVoiceDashboard::GetMatchCulturePatternText() const
{
	return GetStrategy()->DisplayMatchCultureRulePattern();
}

FText SSSVoiceDashboard::GetMatchCulturePatternExampleText() const
{
	return GetStrategy()->DisplayMatchCultureRulePatternExample();
}

void SSSVoiceDashboard::OpenAutoPopulateConfirmationDialog(const FString& Culture)
{
	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		NSLOCTEXT("SSVoiceCultureEditor", "ConfirmAutoPopulateText",
		          "Are you sure you want to auto-populate all voice assets for culture '{0}'?\nThis operation cannot be undone."),
		FText::FromString(Culture)
	);

	if (Result == EAppReturnType::Yes)
	{
		UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoice] Auto-populate confirmed for culture: %s"), *Culture);

		AsyncTask(ENamedThreads::GameThread, [this, Culture]()
		{
			const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();
			
			int32 ModifiedCount = FSSVoiceCultureUtils::AutoPopulateCulture(Culture, EditorSettings->bAutoPopulateOverwriteExisting);

			UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoice] Modified voice: %d"), ModifiedCount);

			// Force regenerate report end refresh voice culture coverage
			OnGenerateReportClicked();
		});
	}
}

void SSSVoiceDashboard::LoadActorListFromJson()
{
	AllActorItems.Empty();

	const FString JsonPath = FPaths::ProjectSavedDir() / TEXT("SSVoiceCulture/VoiceActors.json");
	FString JsonRaw;

	if (!FFileHelper::LoadFileToString(JsonRaw, *JsonPath))
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] Failed to read actor list JSON at %s"), *JsonPath);
		return;
	}

	TSharedPtr<FJsonValue> RootValue;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);

	if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] Failed to parse actor list JSON"));
		return;
	}

	if (RootValue->Type != EJson::Array)
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] Expected root JSON to be an array"));
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
