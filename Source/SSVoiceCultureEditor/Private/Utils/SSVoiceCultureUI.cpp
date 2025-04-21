/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "Utils/SSVoiceCultureUI.h"

#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Internationalization/Culture.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

void FSSVoiceCultureUI::NotifySuccess(const FText& Message, float Duration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = Duration;
	Info.bUseLargeFont = false;
	Info.Image = FCoreStyle::Get().GetBrush("Icons.Success");
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FSSVoiceCultureUI::NotifyFailure(const FText& Message, float Duration)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = Duration;
	Info.bUseLargeFont = false;
	Info.Image = FCoreStyle::Get().GetBrush("Icons.Error");
	FSlateNotificationManager::Get().AddNotification(Info);
}

FString FSSVoiceCultureUI::CultureAsDisplay(const FString& CultureCode)
{
	const FCulturePtr Culture = FInternationalization::Get().GetCulture(CultureCode);
	const FString CultureLabel = Culture.IsValid()
									 ? FString::Printf(TEXT("%s (%s)"), *Culture->GetNativeName(), *CultureCode)
									 : CultureCode;

	return CultureLabel;
}

#undef LOCTEXT_NAMESPACE
