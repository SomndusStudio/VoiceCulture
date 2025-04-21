/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureEditorFilters.h"

#include "SSVoiceCultureEditorSubsystem.h"

////////////////////////////////////////////////////////////////////
// FSSVoiceFilterActorName

FSSVoiceFilterActorName::FSSVoiceFilterActorName(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

FString FSSVoiceFilterActorName::GetName() const
{
	return TEXT("Voice Actor Name");
}

FText FSSVoiceFilterActorName::GetDisplayName() const
{
	return FText::FromString(TEXT("Voice Actor Name"));
}

FText FSSVoiceFilterActorName::GetToolTipText() const
{
	return FText::FromString(TEXT("Show assets from selected voice actor"));
}

FLinearColor FSSVoiceFilterActorName::GetColor() const
{
	return FLinearColor(0.8f, 0.3f, 1.0f);
}

void FSSVoiceFilterActorName::ActiveStateChanged(bool bActive)
{
	FFrontendFilter::ActiveStateChanged(bActive);

	if (bActive)
	{
		UpdateData();
	}

	if (DelegateFilterChanged.IsBound())
	{
		DelegateFilterChanged.Broadcast(bActive);
	}
}

bool FSSVoiceFilterActorName::PassesFilter(const FContentBrowserItem& InItem) const
{
	FAssetData AssetData;
	if (!InItem.Legacy_TryGetAssetData(AssetData)) return false;

	return Assets.Contains(AssetData);
}

void FSSVoiceFilterActorName::SetVoiceActorName(FString InVoiceActorName)
{
	VoiceActorName = InVoiceActorName;
}

void FSSVoiceFilterActorName::UpdateData()
{
	TArray<FAssetData> AssetsUsed;
	USSVoiceCultureEditorSubsystem::GetAssetsFromVoiceActor(AssetsUsed, VoiceActorName);

	Assets.Reset();
	Assets.Append(AssetsUsed);
}

FSSDelegateFilterChanged& FSSVoiceFilterActorName::OnFilterChanged()
{
	return DelegateFilterChanged;
}


TSet<FAssetData> FSSVoiceFilterActorName::GetAssets()
{
	return Assets;
}

////////////////////////////////////////////////////////////////////
// FSSVoiceFilterMissingCulture

FSSVoiceFilterMissingCulture::FSSVoiceFilterMissingCulture(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

FString FSSVoiceFilterMissingCulture::GetName() const
{
	return TEXT("Missing Culture");
}

FText FSSVoiceFilterMissingCulture::GetDisplayName() const
{
	return FText::FromString(TEXT("Missing Culture"));
}

FText FSSVoiceFilterMissingCulture::GetToolTipText() const
{
	return FText::FromString(TEXT("Show assets with missing voice culture"));
}

FLinearColor FSSVoiceFilterMissingCulture::GetColor() const
{
	return FLinearColor::Red;
}

void FSSVoiceFilterMissingCulture::ActiveStateChanged(bool bActive)
{
	FFrontendFilter::ActiveStateChanged(bActive);

	if (bActive)
	{
		UpdateData();
	}

	if (DelegateFilterChanged.IsBound())
	{
		DelegateFilterChanged.Broadcast(bActive);
	}
}

bool FSSVoiceFilterMissingCulture::PassesFilter(const FContentBrowserItem& InItem) const
{
	FAssetData AssetData;
	if (!InItem.Legacy_TryGetAssetData(AssetData)) return false;

	return Assets.Contains(AssetData);
}

void FSSVoiceFilterMissingCulture::UpdateVoiceAssets(const TSet<FAssetData>& InVoiceAssets)
{
	VoiceAssets = InVoiceAssets;
}

void FSSVoiceFilterMissingCulture::UpdateData()
{
	TArray<FAssetData> AssetsUsed = VoiceAssets.Array();
	USSVoiceCultureEditorSubsystem::GetAssetsWithMissingCulture(AssetsUsed);

	Assets.Reset();
	Assets.Append(AssetsUsed);
}

FSSDelegateFilterChanged& FSSVoiceFilterMissingCulture::OnFilterChanged()
{
	return DelegateFilterChanged;
}

////////////////////////////////////////////////////////////////////
// FSSVoiceFilterCompleteCulture

FSSVoiceFilterCompleteCulture::FSSVoiceFilterCompleteCulture(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

FString FSSVoiceFilterCompleteCulture::GetName() const
{
	return TEXT("Complete Culture");
}

FText FSSVoiceFilterCompleteCulture::GetDisplayName() const
{
	return FText::FromString(TEXT("Complete Culture"));
}

FText FSSVoiceFilterCompleteCulture::GetToolTipText() const
{
	return FText::FromString(TEXT("Show assets with complete voice culture"));
}

FLinearColor FSSVoiceFilterCompleteCulture::GetColor() const
{
	return FLinearColor::Green;
}

void FSSVoiceFilterCompleteCulture::ActiveStateChanged(bool bActive)
{
	FFrontendFilter::ActiveStateChanged(bActive);

	if (bActive)
	{
		UpdateData();
	}

	if (DelegateFilterChanged.IsBound())
	{
		DelegateFilterChanged.Broadcast(bActive);
	}
}

bool FSSVoiceFilterCompleteCulture::PassesFilter(const FContentBrowserItem& InItem) const
{
	FAssetData AssetData;
	if (!InItem.Legacy_TryGetAssetData(AssetData)) return false;

	return Assets.Contains(AssetData);
}

void FSSVoiceFilterCompleteCulture::UpdateVoiceAssets(const TSet<FAssetData>& InVoiceAssets)
{
	VoiceAssets = InVoiceAssets;
}

void FSSVoiceFilterCompleteCulture::UpdateData()
{
	TArray<FAssetData> AssetsUsed = VoiceAssets.Array();
	USSVoiceCultureEditorSubsystem::GetAssetsWithCompleteCulture(AssetsUsed);

	Assets.Reset();
	Assets.Append(AssetsUsed);
}

FSSDelegateFilterChanged& FSSVoiceFilterCompleteCulture::OnFilterChanged()
{
	return DelegateFilterChanged;
}