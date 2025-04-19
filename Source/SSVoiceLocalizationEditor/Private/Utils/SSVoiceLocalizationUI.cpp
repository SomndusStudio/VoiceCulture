/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "Utils/SSVoiceLocalizationUI.h"

#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "SSVoiceLocalization"

void FSSVoiceLocalizationUI::NotifySuccess(const FText& Message, float Duration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = Duration;
	Info.bUseLargeFont = false;
	Info.Image = FCoreStyle::Get().GetBrush("NotificationList.SuccessImage");
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FSSVoiceLocalizationUI::NotifyFailure(const FText& Message, float Duration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = Duration;
	Info.bUseLargeFont = false;
	Info.Image = FCoreStyle::Get().GetBrush("NotificationList.FailImage");
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FSSVoiceLocalizationUI::ConfirmDialog(const FText& Title, const FText& Message, FSimpleDelegate OnConfirm)
{
	FSlateApplication::Get().PushMenu(
		FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef(),
		FWidgetPath(),
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(STextBlock)
				.Text(Message)
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(4).HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("Confirm_Yes", "Yes"))
					.OnClicked_Lambda([OnConfirm]()
					{
						OnConfirm.ExecuteIfBound();
						FSlateApplication::Get().DismissAllMenus();
						return FReply::Handled();
					})
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(2)
				[
					SNew(SButton)
					.Text(LOCTEXT("Confirm_Cancel", "Cancel"))
					.OnClicked_Lambda([]()
					{
						FSlateApplication::Get().DismissAllMenus();
						return FReply::Handled();
					})
				]
			]
		],
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect::ContextMenu
	);
}

#undef LOCTEXT_NAMESPACE