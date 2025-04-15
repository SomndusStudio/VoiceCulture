#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SSVoiceLocalizationEditor.generated.h"

////////////////////////////////////////////////////////////////////
// Asset factories

UCLASS()
class SSVOICELOCALIZATIONEDITOR_API USSLocalizedVoiceAsset_Factory : public UFactory
{
    GENERATED_UCLASS_BODY()
    
public:
    
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                      UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ShouldShowInNewMenu() const override { return true; }
};

////////////////////////////////////////////////////////////////////
// Module

class FSSVoiceLocalizationEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
