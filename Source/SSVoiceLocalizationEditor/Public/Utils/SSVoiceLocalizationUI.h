// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class SSVOICELOCALIZATIONEDITOR_API FSSVoiceLocalizationUI
{
public:
	/** Shows a green success toast message */
	static void NotifySuccess(const FText& Message, float Duration = 3.0f);

	/** Shows a red failure toast message */
	static void NotifyFailure(const FText& Message, float Duration = 4.0f);

	/** Opens a reusable confirmation popup with Yes / Cancel */
	static void ConfirmDialog(
		const FText& Title,
		const FText& Message,
		FSimpleDelegate OnConfirm
	);
};
