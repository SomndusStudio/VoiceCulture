/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"

class SSVOICECULTUREEDITOR_API FSSVoiceCultureUI
{
public:
	/** Shows a green success toast message */
	static void NotifySuccess(const FText& Message, float Duration = 3.0f);

	/** Shows a red failure toast message */
	static void NotifyFailure(const FText& Message, float Duration = 4.0f);

	static FString CultureAsDisplay(const FString& CultureCode);
};
