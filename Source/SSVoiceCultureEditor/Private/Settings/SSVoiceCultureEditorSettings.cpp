/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceCultureEditorSettings.h"

#include "Settings/SSVoiceCultureStrategy_Default.h"

USSVoiceCultureEditorSettings::USSVoiceCultureEditorSettings()
{
	FallbackProfile = FSSVoiceStrategyProfile();
	FallbackProfile.ProfileName = "Fallback_Default";
	FallbackProfile.StrategyClass = USSVoiceCultureStrategy_Default::StaticClass();

	// Only add default profile if list is empty (to avoid duplication after reload)
	if (StrategyProfiles.Num() == 0)
	{
		FSSVoiceStrategyProfile DefaultProfile;
		DefaultProfile.ProfileName = TEXT("Default");
		DefaultProfile.StrategyClass = USSVoiceCultureStrategy_Default::StaticClass();

		// Configure additional default values here if needed
		// DefaultProfile.SomeOption = true;
		StrategyProfiles.Add(DefaultProfile);

		// Also set it as active by default
		ActiveVoiceProfileName = DefaultProfile.ProfileName;
	}
}

const USSVoiceCultureEditorSettings* USSVoiceCultureEditorSettings::GetSetting()
{
	return GetDefault<USSVoiceCultureEditorSettings>();
}

USSVoiceCultureEditorSettings* USSVoiceCultureEditorSettings::GetMutableSetting()
{
	return GetMutableDefault<USSVoiceCultureEditorSettings>();
}

