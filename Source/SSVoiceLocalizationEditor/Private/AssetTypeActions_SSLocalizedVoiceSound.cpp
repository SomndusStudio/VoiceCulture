/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "AssetTypeActions_SSLocalizedVoiceSound.h"

#include "EditorStyleSet.h"
#include "SSLocalizedVoiceSoundEditorToolkit.h"
#include "ToolMenuSection.h"
#include "Components/AudioComponent.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SSLocalizedVoiceSound::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<USSLocalizedVoiceSound>> Assets;
	for (UObject* Obj : InObjects)
	{
		if (USSLocalizedVoiceSound* VoiceSound = Cast<USSLocalizedVoiceSound>(Obj))
		{
			Assets.Add(VoiceSound);
		}
	}

	Section.AddMenuEntry(
		"PlayLocalizedVoice",
		LOCTEXT("PlayLocalizedVoice", "Play"),
		LOCTEXT("PlayLocalizedVoiceTooltip", "Play the localized voice audio."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MediaAsset.AssetActions.Play"),
		FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_SSLocalizedVoiceSound::ExecutePlay, Assets))
	);

	Section.AddMenuEntry(
		"StopLocalizedVoice",
		LOCTEXT("StopLocalizedVoice", "Stop"),
		LOCTEXT("StopLocalizedVoiceTooltip", "Stop playing the voice preview."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MediaAsset.AssetActions.Stop"),
		FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_SSLocalizedVoiceSound::ExecuteStop))
	);
}

void FAssetTypeActions_SSLocalizedVoiceSound::ExecutePlay(TArray<TWeakObjectPtr<USSLocalizedVoiceSound>> Objects) const
{
	for (auto& AssetPtr : Objects)
	{
		PlaySound(AssetPtr);
	}
}

void FAssetTypeActions_SSLocalizedVoiceSound::ExecuteStop() const
{
	if (GEditor)
	{
		GEditor->ResetPreviewAudioComponent();
	}
}

void FAssetTypeActions_SSLocalizedVoiceSound::PlaySound(TWeakObjectPtr<USSLocalizedVoiceSound> AssetPtr) const
{
	if (USSLocalizedVoiceSound* Asset = AssetPtr.Get())
	{
		if (USoundBase* Sound = Asset->GetPreviewLocalizedSound())
		{
			if (GEditor)
			{
				GEditor->PlayPreviewSound(Sound);
			}
		}
	}
}

bool FAssetTypeActions_SSLocalizedVoiceSound::IsSoundPlaying(const FAssetData& AssetData) const
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

TSharedPtr<SWidget> FAssetTypeActions_SSLocalizedVoiceSound::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	// Choose the button icon (Play or Stop) based on the sound status
	auto OnGetDisplayBrushLambda = [this, AssetData]() -> const FSlateBrush* {
		if (IsSoundPlaying(AssetData))
		{
			return FEditorStyle::GetBrush("MediaAsset.AssetActions.Stop.Large");
		}

		return FEditorStyle::GetBrush("MediaAsset.AssetActions.Play.Large");
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
			USSLocalizedVoiceSound* VoiceSound = Cast<USSLocalizedVoiceSound>(AssetData.GetAsset());
			PlaySound(VoiceSound);
		}
		return FReply::Handled();
	};

	// Dynamic tooltip text (Play or Stop)
	auto OnToolTipTextLambda = [this, AssetData]() -> FText
	{
		if (IsSoundPlaying(AssetData))
		{
			return LOCTEXT("Thumbnail_StopSoundToolTip", "Stop selected sound");
		}
		return LOCTEXT("Thumbnail_PlaySoundToolTip", "Play selected sound");
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
	.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
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

void FAssetTypeActions_SSLocalizedVoiceSound::OpenAssetEditor(const TArray<UObject*>& InObjects,
	TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		if (USSLocalizedVoiceSound* VoiceSound = Cast<USSLocalizedVoiceSound>(Object))
		{
			TSharedRef<FSSLocalizedVoiceSoundEditorToolkit> Editor = MakeShareable(new FSSLocalizedVoiceSoundEditorToolkit());
			Editor->Init(VoiceSound, EToolkitMode::Standalone, EditWithinLevelEditor);
		}
	}
}

#undef LOCTEXT_NAMESPACE
