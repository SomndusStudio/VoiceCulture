#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "Modules/ModuleManager.h"
#include "SSVoiceCultureEditor.generated.h"

////////////////////////////////////////////////////////////////////
// Asset factories

UCLASS()
class SSVOICECULTUREEDITOR_API USSVoiceCultureSound_Factory : public UFactory
{
    GENERATED_UCLASS_BODY()
    
public:
    
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
                                      UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ShouldShowInNewMenu() const override { return true; }
};

////////////////////////////////////////////////////////////////////
// Module

class FSSVoiceCultureEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void FillSomndusStudioMenu(FMenuBuilder& MenuBuilder);
    void AddSomndusStudioMenu(FMenuBuilder& MenuBuilder);
    
    void OpenVoiceDashboardTab();
    
    TSharedRef<SDockTab> SpawnVoiceDashboardTab(const FSpawnTabArgs& Args);

private:
    TSharedPtr<FGraphPanelNodeFactory> GraphNodeFactory;
};
