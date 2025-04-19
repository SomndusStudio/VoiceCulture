/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceLocalizationEditorSettings.h"

#include "Settings/SSVoiceAutofillStrategy_Default.h"

USSVoiceLocalizationEditorSettings::USSVoiceLocalizationEditorSettings()
{
	FallbackProfile = FSSVoiceAutofillProfile();
	FallbackProfile.ProfileName = "Fallback_Default";
	FallbackProfile.StrategyClass = USSVoiceAutofillStrategy_Default::StaticClass();

	// Only add default profile if list is empty (to avoid duplication after reload)
	if (AutofillProfiles.Num() == 0)
	{
		FSSVoiceAutofillProfile DefaultProfile;
		DefaultProfile.ProfileName = TEXT("Default");
		DefaultProfile.StrategyClass = USSVoiceAutofillStrategy_Default::StaticClass();

		// Configure additional default values here if needed
		// DefaultProfile.SomeOption = true;
		AutofillProfiles.Add(DefaultProfile);

		// Also set it as active by default
		ActiveVoiceProfileName = DefaultProfile.ProfileName;
	}
}

const USSVoiceLocalizationEditorSettings* USSVoiceLocalizationEditorSettings::GetSetting()
{
	return GetDefault<USSVoiceLocalizationEditorSettings>();
}

USSVoiceLocalizationEditorSettings* USSVoiceLocalizationEditorSettings::GetMutableSetting()
{
	return GetMutableDefault<USSVoiceLocalizationEditorSettings>();
}

