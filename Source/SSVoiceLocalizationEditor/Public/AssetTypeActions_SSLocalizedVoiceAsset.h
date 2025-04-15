// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "SSLocalizedVoiceAsset.h"
#include "UObject/Object.h"


/**
 * 
 */
class SSVOICELOCALIZATIONEDITOR_API FAssetTypeActions_SSLocalizedVoiceAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "SSLocalizedVoiceAsset", "Localized Voice Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 200, 0); }
	virtual UClass* GetSupportedClass() const override { return USSLocalizedVoiceAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Sounds; }

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section) override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

protected:
	void ExecutePlay(TArray<TWeakObjectPtr<USSLocalizedVoiceAsset>> Objects) const;
	void ExecuteStop() const;

	void PlaySound(TWeakObjectPtr<USSLocalizedVoiceAsset> AssetPtr) const;
	bool IsSoundPlaying(USoundBase* Sound) const;
};
