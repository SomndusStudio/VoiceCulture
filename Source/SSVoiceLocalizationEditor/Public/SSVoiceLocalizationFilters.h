/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "FrontendFilterBase.h"
#include "UObject/Object.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FSSDelegateFilterChanged, const bool bActive);

class FSSVoiceFilterActorName final : public FFrontendFilter
{
public:
	explicit FSSVoiceFilterActorName(TSharedPtr<FFrontendFilterCategory> InCategory);
	virtual FString GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTipText() const override;
	virtual FLinearColor GetColor() const override;
	virtual void ActiveStateChanged(bool bActive) override;
	virtual bool PassesFilter(const FContentBrowserItem& InItem) const override;
	
	void SetVoiceActorName(FString InVoiceActorName);
	void UpdateData();

	FSSDelegateFilterChanged& OnFilterChanged();

	TSet<FAssetData> GetAssets();
private:

	FString VoiceActorName;
	
	FSSDelegateFilterChanged DelegateFilterChanged;
	TSet<FAssetData> Assets;
};

class FSSVoiceFilterMissingCulture final : public FFrontendFilter
{
public:
	explicit FSSVoiceFilterMissingCulture(TSharedPtr<FFrontendFilterCategory> InCategory);
	virtual FString GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTipText() const override;
	virtual FLinearColor GetColor() const override;
	virtual void ActiveStateChanged(bool bActive) override;
	virtual bool PassesFilter(const FContentBrowserItem& InItem) const override;

	void UpdateVoiceAssets(const TSet<FAssetData>& InVoiceAssets);
	void UpdateData();

	FSSDelegateFilterChanged& OnFilterChanged();

private:

	FSSDelegateFilterChanged DelegateFilterChanged;
	TSet<FAssetData> VoiceAssets;
	TSet<FAssetData> Assets;
};

class FSSVoiceFilterCompleteCulture final : public FFrontendFilter
{
public:
	explicit FSSVoiceFilterCompleteCulture(TSharedPtr<FFrontendFilterCategory> InCategory);
	virtual FString GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTipText() const override;
	virtual FLinearColor GetColor() const override;
	virtual void ActiveStateChanged(bool bActive) override;
	virtual bool PassesFilter(const FContentBrowserItem& InItem) const override;

	void UpdateVoiceAssets(const TSet<FAssetData>& InVoiceAssets);
	void UpdateData();

	FSSDelegateFilterChanged& OnFilterChanged();

private:

	FSSDelegateFilterChanged DelegateFilterChanged;
	TSet<FAssetData> VoiceAssets;
	TSet<FAssetData> Assets;
};