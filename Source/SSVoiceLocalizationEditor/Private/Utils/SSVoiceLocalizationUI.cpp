// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/SSVoiceLocalizationUI.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

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
