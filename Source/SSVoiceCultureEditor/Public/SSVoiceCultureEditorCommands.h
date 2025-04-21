/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

class SSVOICECULTUREEDITOR_API FSSVoiceCultureEditorEditorCommonCommands : public TCommands<FSSVoiceCultureEditorEditorCommonCommands>
{
public:
	FSSVoiceCultureEditorEditorCommonCommands();

	virtual void RegisterCommands() override;
public:
	TSharedPtr<FUICommandInfo> BrowseToAsset;
	TSharedPtr<FUICommandInfo> AutoPopulate;
};
