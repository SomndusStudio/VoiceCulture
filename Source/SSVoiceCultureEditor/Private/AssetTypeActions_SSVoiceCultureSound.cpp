/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "AssetTypeActions_SSVoiceCultureSound.h"

#include "EditorStyleSet.h"
#include "SSVoiceCultureSoundEditorToolkit.h"
#include "SSVoiceStyleCompat.h"
#include "ToolMenuSection.h"
#include "Components/AudioComponent.h"
#include "Utils/SSVoiceCultureUI.h"
#include "Utils/SSVoiceCultureUtils.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SSVoiceCultureSound::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<USSVoiceCultureSound>> Assets;
	for (UObject* Obj : InObjects)
	{
		if (USSVoiceCultureSound* VoiceSound = Cast<USSVoiceCultureSound>(Obj))
		{
			Assets.Add(VoiceSound);
		}
	}

	const auto& Commands = FSSVoiceCultureSoundEditorCommands::Get();

	// Command action
	TSharedPtr<FUICommandList> TempCommandList = MakeShared<FUICommandList>();

	TempCommandList->MapAction(
		Commands.Play,
		FExecuteAction::CreateSP(this, &FAssetTypeActions_SSVoiceCultureSound::ExecutePlay, Assets)
	);
	TempCommandList->MapAction(
		Commands.Stop,
		FExecuteAction::CreateSP(this, &FAssetTypeActions_SSVoiceCultureSound::ExecuteStop)
	);
	TempCommandList->MapAction(
		Commands.AutoPopulate,
		FExecuteAction::CreateSP(this, &FAssetTypeActions_SSVoiceCultureSound::ExecuteAutoPopulate, Assets)
	);

	// Command entry
	Section.AddMenuEntryWithCommandList(Commands.Play, TempCommandList);
	Section.AddMenuEntryWithCommandList(Commands.Stop, TempCommandList);
	Section.AddMenuEntryWithCommandList(Commands.AutoPopulate, TempCommandList);
}

void FAssetTypeActions_SSVoiceCultureSound::ExecutePlay(TArray<TWeakObjectPtr<USSVoiceCultureSound>> Objects) const
{
	for (auto& AssetPtr : Objects)
	{
		PlaySound(AssetPtr);
	}
}

void FAssetTypeActions_SSVoiceCultureSound::ExecuteStop() const
{
	if (GEditor)
	{
		GEditor->ResetPreviewAudioComponent();
	}
}

void FAssetTypeActions_SSVoiceCultureSound::ExecuteAutoPopulate(TArray<TWeakObjectPtr<USSVoiceCultureSound>> Objects) const
{
	int CountPopulateAsset = 0;
	for (auto& AssetPtr : Objects)
	{
		if (USSVoiceCultureSound* Asset = AssetPtr.Get())
		{
			if (FSSVoiceCultureUtils::AutoPopulateFromNaming(Asset, false, false))
			{
				CountPopulateAsset++;
			}
		}
	}
	if (CountPopulateAsset > 0)
	{
		FSSVoiceCultureUI::NotifySuccess(FText::Format(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateBatchSuccess",
			          "Auto-populate completed: {0} asset(s) updated."),
			FText::AsNumber(CountPopulateAsset)
		));
	}
	else
	{
		FSSVoiceCultureUI::NotifyFailure(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateBatchFailure", "No assets could be auto-populated.")
		);
	}
}

void FAssetTypeActions_SSVoiceCultureSound::PlaySound(TWeakObjectPtr<USSVoiceCultureSound> AssetPtr) const
{
	if (USSVoiceCultureSound* Asset = AssetPtr.Get())
	{
		if (USoundBase* Sound = Asset->GetEditorPreviewSound())
		{
			if (GEditor)
			{
				GEditor->PlayPreviewSound(Sound);
			}
		}
	}
}

bool FAssetTypeActions_SSVoiceCultureSound::IsSoundPlaying(const FAssetData& AssetData) const
{
	const UAudioComponent* PreviewComp = GEditor->GetPreviewAudioComponent();
	if (PreviewComp && PreviewComp->Sound && PreviewComp->IsPlaying())
	{
		if (PreviewComp->Sound->GetFName() == AssetData.AssetName)
		{
			if (PreviewComp->Sound->GetOutermost()->GetFName() == AssetData.PackageName)
			{
				return true;
			}
		}
	}

	return false;
}

TSharedPtr<SWidget> FAssetTypeActions_SSVoiceCultureSound::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	// Choose the button icon (Play or Stop) based on the sound status
	auto OnGetDisplayBrushLambda = [this, AssetData]() -> const FSlateBrush*
	{
		if (IsSoundPlaying(AssetData))
		{
			return SSVoiceStyleCompat::GetBrush("MediaAsset.AssetActions.Stop.Large");
		}

		return SSVoiceStyleCompat::GetBrush("MediaAsset.AssetActions.Play.Large");
	};

	// When you click the button
	auto OnClickedLambda = [this, AssetData]() -> FReply
	{
		if (IsSoundPlaying(AssetData))
		{
			ExecuteStop();
		}
		else
		{
			USSVoiceCultureSound* VoiceSound = Cast<USSVoiceCultureSound>(AssetData.GetAsset());
			PlaySound(VoiceSound);
		}
		return FReply::Handled();
	};

	// Dynamic tooltip text (Play or Stop)
	auto OnToolTipTextLambda = [this, AssetData]() -> FText
	{
		if (IsSoundPlaying(AssetData))
		{
			return NSLOCTEXT("SSVoiceCultureEditor", "Thumbnail_StopSoundToolTip", "Stop selected sound");
		}
		return NSLOCTEXT("SSVoiceCultureEditor", "Thumbnail_PlaySoundToolTip", "Play selected sound");
	};

	// Create an SBox centered on the thumbnail
	TSharedPtr<SBox> Box;
	SAssignNew(Box, SBox)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.Padding(FMargin(2));


	// Display the button only when: the mouse is hovering over it OR the sound is currently playing.
	auto OnGetVisibilityLambda = [this, Box, AssetData]() -> EVisibility
	{
		if (Box.IsValid() && (Box->IsHovered() || IsSoundPlaying(AssetData)))
		{
			return EVisibility::Visible;
		}
		return EVisibility::Hidden;
	};

	// Create Widget UI
	TSharedPtr<SButton> Widget;
	SAssignNew(Widget, SButton)
	.ButtonStyle(SSVoiceStyleCompat::Get(), "HoverHintOnly")
	.ToolTipText_Lambda(OnToolTipTextLambda)
	.Cursor(EMouseCursor::Default)
	.ForegroundColor(FSlateColor::UseForeground())
	.IsFocusable(false)
	.OnClicked_Lambda(OnClickedLambda)
	.Visibility_Lambda(OnGetVisibilityLambda)
	[
		SNew(SImage)
		.Image_Lambda(OnGetDisplayBrushLambda)
	];

	Box->SetContent(Widget.ToSharedRef());
	Box->SetVisibility(EVisibility::Visible);

	return Box;
}

void FAssetTypeActions_SSVoiceCultureSound::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                            TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		if (USSVoiceCultureSound* VoiceSound = Cast<USSVoiceCultureSound>(Object))
		{
			TSharedRef<FSSVoiceCultureSoundEditorToolkit> Editor = MakeShareable(
				new FSSVoiceCultureSoundEditorToolkit());
			Editor->Init(VoiceSound, EToolkitMode::Standalone, EditWithinLevelEditor);
		}
	}
}

#undef LOCTEXT_NAMESPACE
