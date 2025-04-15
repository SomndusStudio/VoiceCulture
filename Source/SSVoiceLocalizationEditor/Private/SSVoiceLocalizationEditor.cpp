#include "SSVoiceLocalizationEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_SSLocalizedVoiceSound.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "SSVoiceLocalizationStyle.h"

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

void FSSVoiceLocalizationEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	LocalizedVoiceSoundActions = MakeShareable(new FAssetTypeActions_SSLocalizedVoiceSound);
	AssetTools.RegisterAssetTypeActions(LocalizedVoiceSoundActions.ToSharedRef());

	// Editor icons
	FSSVoiceLocalizationStyle::Initialize();
}

void FSSVoiceLocalizationEditorModule::ShutdownModule()
{
	// Editor icons
	FSSVoiceLocalizationStyle::Shutdown();
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && LocalizedVoiceSoundActions.IsValid())
	{
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(LocalizedVoiceSoundActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSSVoiceLocalizationEditorModule, SSVoiceLocalizationEditor)