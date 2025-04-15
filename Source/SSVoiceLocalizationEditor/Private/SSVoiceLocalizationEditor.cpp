#include "SSVoiceLocalizationEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_SSLocalizedVoiceAsset.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "SSVoiceLocalizationStyle.h"

#define LOCTEXT_NAMESPACE "FSSVoiceLocalizationEditorModule"

TSharedPtr<IAssetTypeActions> LocalizedVoiceAssetActions;

////////////////////////////////////////////////////////////////////
// Asset factories

USSLocalizedVoiceAsset_Factory::USSLocalizedVoiceAsset_Factory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = USSLocalizedVoiceAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USSLocalizedVoiceAsset_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                          EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(USSLocalizedVoiceAsset::StaticClass()));
	return NewObject<USSLocalizedVoiceAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}

////////////////////////////////////////////////////////////////////
// Module

void FSSVoiceLocalizationEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	LocalizedVoiceAssetActions = MakeShareable(new FAssetTypeActions_SSLocalizedVoiceAsset);
	AssetTools.RegisterAssetTypeActions(LocalizedVoiceAssetActions.ToSharedRef());

	// Editor icons
	FSSVoiceLocalizationStyle::Initialize();
}

void FSSVoiceLocalizationEditorModule::ShutdownModule()
{
	// Editor icons
	FSSVoiceLocalizationStyle::Shutdown();
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools") && LocalizedVoiceAssetActions.IsValid())
	{
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(LocalizedVoiceAssetActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSSVoiceLocalizationEditorModule, SSVoiceLocalizationEditor)