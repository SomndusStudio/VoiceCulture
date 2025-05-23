/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "SSVoiceCultureSound.h"
#include "UObject/Object.h"


/**
 * Implements asset type actions for the SSVoiceCultureSound.
 * 
 * This class defines how the SSVoiceCultureSound behaves in the Content Browser:
 * - Determines how the asset is categorized and displayed
 * - Adds context menu actions like Play/Stop
 * - Controls how the thumbnail and toolbar behave
 * 
 * It's registered with the AssetTools module to integrate with the Editor.
 */
class SSVOICECULTUREEDITOR_API FAssetTypeActions_SSVoiceCultureSound : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "SSVoiceCultureSound", "Voice Culture Audio"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 200, 0); }
	virtual UClass* GetSupportedClass() const override { return USSVoiceCultureSound::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Sounds; }

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	
protected:
	void ExecutePlay(TArray<TWeakObjectPtr<USSVoiceCultureSound>> Objects) const;
	void ExecuteStop() const;
	void ExecuteAutoPopulate(TArray<TWeakObjectPtr<USSVoiceCultureSound>> Objects) const;
	
	void PlaySound(TWeakObjectPtr<USSVoiceCultureSound> AssetPtr) const;
	bool IsSoundPlaying(const FAssetData& AssetData) const;
};
