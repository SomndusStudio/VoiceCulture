/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "SSVoiceCultureEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_SSVoiceCultureSound.h"
#include "EdGraphUtilities.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "SSVoiceCultureSettings.h"
#include "SSVoiceCultureEditorStyle.h"
#include "SSVoiceCultureSoundEditorToolkit.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dashboard/SSSVoiceDashboard.h"
#include "Settings/SSVoiceCultureEditorSettings.h"
#include "Slate/SSVoiceCultureSlateComponents.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

TSharedPtr<IAssetTypeActions> VoiceCultureSoundActions;

////////////////////////////////////////////////////////////////////
// Asset factories

USSVoiceCultureSound_Factory::USSVoiceCultureSound_Factory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = USSVoiceCultureSound::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USSVoiceCultureSound_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                          EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(USSVoiceCultureSound::StaticClass()));
	return NewObject<USSVoiceCultureSound>(InParent, Class, Name, Flags | RF_Transactional, Context);
}

////////////////////////////////////////////////////////////////////
// Module

TSharedRef<SDockTab> FSSVoiceCultureEditorModule::SpawnVoiceDashboardTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(NSLOCTEXT("SSVoiceCultureEditor", "VoiceCultureDashboardTabLabel", "Voice Culture Dashboard"));
	
	TSharedRef<SSSVoiceDashboard> DashboardWidget = MakeShared<SSSVoiceDashboard>();
	
	DashboardWidget->Construct(SSSVoiceDashboard::FArguments(), Args.GetOwnerWindow(), NewTab);

	NewTab->SetContent(DashboardWidget);

	return NewTab;
}

void FSSVoiceCultureEditorModule::OpenVoiceDashboardTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("SSVoiceDashboard"));
}

void FSSVoiceCultureEditorModule::FillSomndusStudioMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("SSVoiceCultureEditor", "OpenVoiceCultureDashboard", "Voice Culture Dashboard"),
		NSLOCTEXT("SSVoiceCultureEditor", "OpenVoiceCultureDashboardTooltip", "Open the Voice Culture Dashboard."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FSSVoiceCultureEditorModule::OpenVoiceDashboardTab))
	);
}

void FSSVoiceCultureEditorModule::AddSomndusStudioMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddSubMenu(
		NSLOCTEXT("SSVoiceCultureEditor", "SomndusStudioMenuLabel", "Somndus Studio"),
		NSLOCTEXT("SSVoiceCultureEditor", "SomndusStudioMenuTooltip", "Custom tools by Somndus Studio."),
		FNewMenuDelegate::CreateRaw(this, &FSSVoiceCultureEditorModule::FillSomndusStudioMenu),
		false, // no icon
		FSlateIcon()
	);
}

void FSSVoiceCultureEditorModule::StartupModule()
{
	// Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", 
			"Plugins", 
			"Voice Editor Settings",
			NSLOCTEXT("SSVoiceCultureEditor", "VoiceEditorSettingsName", "Voice Editor Settings"),
			NSLOCTEXT("SSVoiceCultureEditor", "VoiceEditorSettingsDescription", "Configure voice editor settings for audio culture."),
			GetMutableDefault<USSVoiceCultureEditorSettings>()
		);
	}
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register commands globally
	FSSVoiceCultureSoundEditorCommands::Register();

	// Initialize and register the visual node factory
	GraphNodeFactory = MakeShared<FSSVoiceCultureGraphNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphNodeFactory);

	// Register asset type actions (for context menus, asset editor etc.)
	VoiceCultureSoundActions = MakeShareable(new FAssetTypeActions_SSVoiceCultureSound);
	AssetTools.RegisterAssetTypeActions(VoiceCultureSoundActions.ToSharedRef());

	// Dashboard
	static const FName VoiceDashboardTabName("SSVoiceDashboard");

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(VoiceDashboardTabName, FOnSpawnTab::CreateRaw(this, &FSSVoiceCultureEditorModule::SpawnVoiceDashboardTab))
		.SetDisplayName(NSLOCTEXT("SSVoiceCultureEditor", "VoiceDashboardTabTitle", "Voice Dashboard"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	// Create/Check Somndus Studio menu
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FSSVoiceCultureEditorModule::AddSomndusStudioMenu));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	
	// Editor icons
	FSSVoiceCultureStyle::Initialize();
}

void FSSVoiceCultureEditorModule::ShutdownModule()
{
	// Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Voice Editor Settings");
	}

	if (GraphNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphNodeFactory.ToSharedRef());
		GraphNodeFactory.Reset();
	}
	
	// Editor icons
	FSSVoiceCultureStyle::Shutdown();

	// Dashboard
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("SSVoiceDashboard");
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && VoiceCultureSoundActions.IsValid())
	{
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(VoiceCultureSoundActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSSVoiceCultureEditorModule, SSVoiceCultureEditor)