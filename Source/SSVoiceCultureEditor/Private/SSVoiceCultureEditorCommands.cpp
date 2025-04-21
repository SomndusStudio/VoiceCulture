/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureEditorCommands.h"

#include "SSVoiceStyleCompat.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

FSSVoiceCultureEditorEditorCommonCommands::FSSVoiceCultureEditorEditorCommonCommands()
	: TCommands<FSSVoiceCultureEditorEditorCommonCommands>(
		TEXT("SSVoiceCultureCommands"),
		NSLOCTEXT("SSVoiceCultureEditor", "SSVoiceCultureCommands", "SS Voice Culture Commands"),
		NAME_None,
		SSVoiceStyleCompat::GetStyleSetName()) // Use Editor style
{
}

void FSSVoiceCultureEditorEditorCommonCommands::RegisterCommands()
{
	UI_COMMAND(BrowseToAsset, "Browse to Asset", "Locate the selected asset in the Content Browser",
	           EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AutoPopulate, "Auto Populate", "Auto-fill voice culture entries based on naming pattern",
	           EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
