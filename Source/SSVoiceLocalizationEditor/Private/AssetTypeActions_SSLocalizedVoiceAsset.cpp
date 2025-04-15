// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_SSLocalizedVoiceAsset.h"

#include "ToolMenuSection.h"
#include "Components/AudioComponent.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SSLocalizedVoiceAsset::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<USSLocalizedVoiceAsset>> Assets;
	for (UObject* Obj : InObjects)
	{
		if (USSLocalizedVoiceAsset* VoiceAsset = Cast<USSLocalizedVoiceAsset>(Obj))
		{
			Assets.Add(VoiceAsset);
		}
	}

	Section.AddMenuEntry(
		"PlayLocalizedVoice",
		LOCTEXT("PlayLocalizedVoice", "Play"),
		LOCTEXT("PlayLocalizedVoiceTooltip", "Play the localized voice asset."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MediaAsset.AssetActions.Play"),
		FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_SSLocalizedVoiceAsset::ExecutePlay, Assets))
	);

	Section.AddMenuEntry(
		"StopLocalizedVoice",
		LOCTEXT("StopLocalizedVoice", "Stop"),
		LOCTEXT("StopLocalizedVoiceTooltip", "Stop playing the voice preview."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MediaAsset.AssetActions.Stop"),
		FUIAction(FExecuteAction::CreateSP(this, &FAssetTypeActions_SSLocalizedVoiceAsset::ExecuteStop))
	);
}

void FAssetTypeActions_SSLocalizedVoiceAsset::ExecutePlay(TArray<TWeakObjectPtr<USSLocalizedVoiceAsset>> Objects) const
{
	for (auto& AssetPtr : Objects)
	{
		PlaySound(AssetPtr);
	}
}

void FAssetTypeActions_SSLocalizedVoiceAsset::ExecuteStop() const
{
	if (GEditor)
	{
		GEditor->ResetPreviewAudioComponent();
	}
}

void FAssetTypeActions_SSLocalizedVoiceAsset::PlaySound(TWeakObjectPtr<USSLocalizedVoiceAsset> AssetPtr) const
{
	if (USSLocalizedVoiceAsset* Asset = AssetPtr.Get())
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

bool FAssetTypeActions_SSLocalizedVoiceAsset::IsSoundPlaying(USoundBase* Sound) const
{
	UAudioComponent* PreviewComp = GEditor->GetPreviewAudioComponent();
	return PreviewComp && PreviewComp->Sound == Sound && PreviewComp->IsPlaying();
}

TSharedPtr<SWidget> FAssetTypeActions_SSLocalizedVoiceAsset::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	USSLocalizedVoiceAsset* VoiceAsset = Cast<USSLocalizedVoiceAsset>(AssetData.GetAsset());

	// Choose the button icon (Play or Stop) based on the sound status
	auto OnGetDisplayBrushLambda = [this, VoiceAsset]() -> const FSlateBrush* {
		if (IsSoundPlaying(VoiceAsset->GetPreviewLocalizedSound()))
		{
			return FEditorStyle::GetBrush("MediaAsset.AssetActions.Stop.Large");
		}

		return FEditorStyle::GetBrush("MediaAsset.AssetActions.Play.Large");
	};

	// When you click the button
	auto OnClickedLambda = [this, VoiceAsset]() -> FReply
	{
		if (IsSoundPlaying(VoiceAsset->GetPreviewLocalizedSound()))
		{
			ExecuteStop();
		}
		else
		{
			PlaySound(VoiceAsset);
		}
		return FReply::Handled();
	};

	// Dynamic tooltip text (Play or Stop)
	auto OnToolTipTextLambda = [this, VoiceAsset]() -> FText
	{
		if (IsSoundPlaying(VoiceAsset->GetPreviewLocalizedSound()))
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
	auto OnGetVisibilityLambda = [this, Box, VoiceAsset]() -> EVisibility
	{
		if (Box.IsValid() && (Box->IsHovered() || IsSoundPlaying(VoiceAsset->GetPreviewLocalizedSound())))
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
#undef LOCTEXT_NAMESPACE
