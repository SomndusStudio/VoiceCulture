#include "SSVoiceLocalizationEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_SSLocalizedVoiceSound.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "SSVoiceLocalizationStyle.h"
#include "Dashboard/SSSVoiceDashboard.h"
#include "Settings/SSVoiceLocalizationEditorSettings.h"

#define LOCTEXT_NAMESPACE "FSSVoiceLocalizationEditorModule"

TSharedPtr<IAssetTypeActions> LocalizedVoiceSoundActions;

////////////////////////////////////////////////////////////////////
// Asset factories

USSLocalizedVoiceSound_Factory::USSLocalizedVoiceSound_Factory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = USSLocalizedVoiceSound::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USSLocalizedVoiceSound_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                          EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(USSLocalizedVoiceSound::StaticClass()));
	return NewObject<USSLocalizedVoiceSound>(InParent, Class, Name, Flags | RF_Transactional, Context);
}

////////////////////////////////////////////////////////////////////
// Module

TSharedRef<SDockTab> FSSVoiceLocalizationEditorModule::SpawnVoiceDashboardTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(NSLOCTEXT("SSVoice", "VoiceDashboardTabLabel", "Voice Dashboard"));
	
	TSharedRef<SSSVoiceDashboard> DashboardWidget = MakeShared<SSSVoiceDashboard>();
	
	DashboardWidget->Construct(SSSVoiceDashboard::FArguments(), Args.GetOwnerWindow(), NewTab);

	NewTab->SetContent(DashboardWidget);

	return NewTab;
}

void FSSVoiceLocalizationEditorModule::OpenVoiceDashboardTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("SSVoiceDashboard"));
}

void FSSVoiceLocalizationEditorModule::FillSomndusStudioMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("SSVoice", "OpenVoiceDashboard", "Voice Dashboard"),
		NSLOCTEXT("SSVoice", "OpenVoiceDashboardTooltip", "Open the Voice Localization Dashboard."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FSSVoiceLocalizationEditorModule::OpenVoiceDashboardTab))
	);
}

void FSSVoiceLocalizationEditorModule::AddSomndusStudioMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddSubMenu(
		NSLOCTEXT("SSVoice", "SomndusStudioMenuLabel", "Somndus Studio"),
		NSLOCTEXT("SSVoice", "SomndusStudioMenuTooltip", "Custom tools by Somndus Studio."),
		FNewMenuDelegate::CreateRaw(this, &FSSVoiceLocalizationEditorModule::FillSomndusStudioMenu),
		false, // no icon
		FSlateIcon()
	);
}

void FSSVoiceLocalizationEditorModule::StartupModule()
{
	// Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", 
			"Plugins", 
			"Voice Editor Settings",
			NSLOCTEXT("SSVoice", "VoiceAutofillSettingsName", "Voice Autofill Settings"),
			NSLOCTEXT("SSVoice", "VoiceAutofillSettingsDescription", "Configure voice autofill profiles for localized audio."),
			GetMutableDefault<USSVoiceLocalizationEditorSettings>()
		);
	}
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	LocalizedVoiceSoundActions = MakeShareable(new FAssetTypeActions_SSLocalizedVoiceSound);
	AssetTools.RegisterAssetTypeActions(LocalizedVoiceSoundActions.ToSharedRef());

	// Dashboard
	static const FName VoiceDashboardTabName("SSVoiceDashboard");

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(VoiceDashboardTabName, FOnSpawnTab::CreateRaw(this, &FSSVoiceLocalizationEditorModule::SpawnVoiceDashboardTab))
		.SetDisplayName(NSLOCTEXT("SSVoice", "VoiceDashboardTabTitle", "Voice Dashboard"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	// Create/Check Somndus Studio menu
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FSSVoiceLocalizationEditorModule::AddSomndusStudioMenu));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	
	// Editor icons
	FSSVoiceLocalizationStyle::Initialize();
}

void FSSVoiceLocalizationEditorModule::ShutdownModule()
{
	// Settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Voice Editor Settings");
	}
	
	// Editor icons
	FSSVoiceLocalizationStyle::Shutdown();

	// Dashboard
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("SSVoiceDashboard");
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && LocalizedVoiceSoundActions.IsValid())
	{
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(LocalizedVoiceSoundActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSSVoiceLocalizationEditorModule, SSVoiceLocalizationEditor)