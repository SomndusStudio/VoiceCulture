/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "AutomationBlueprintFunctionLibrary.h"
#include "SSVoiceCultureStatics.generated.h"

/**
 * Blueprint function library exposing runtime utilities for the Voice Culture system.
 *
 * Provides helper nodes that are aware of USSVoiceCultureSound assets and transparently
 * resolve them to the sound matching the current voice culture. When the input is any
 * other USoundBase subclass (SoundWave, SoundCue, MetaSound, ...), the helpers fall
 * back to the standard behavior, making them safe drop-in replacements for their
 * engine counterparts.
 *
 * Typical use case: querying the duration of a voice line at runtime after the player
 * has changed voice culture. The engine's native "Duration" node reads the cached
 * USoundBase::Duration property, which only reflects the value captured at load time
 * and does not follow runtime culture switches. The helpers in this library always
 * resolve the currently active culture sound before returning a value.
 *
 * All functions are static and callable from any Blueprint context.
 */
UCLASS()
class SSVOICECULTURE_API USSVoiceCultureStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns the duration of the given sound, resolving the current culture's sound if it is a Voice Culture Sound.
	 * Falls back to USoundBase::GetDuration() for any other sound type.
	 * Returns 0 if the sound is null.
	 */
	UFUNCTION(BlueprintPure, Category = "Voice Culture", meta = (DisplayName = "GetDuration (Runtime)", CompactNodeTitle = "Duration (Runtime)", DefaultToSelf = "false"))
	static float GetVoiceCultureAwareDuration(const USoundBase* Sound);

	/**
	 * Callable version (non-pure) of GetVoiceCultureAwareDuration.
	 * Use this variant when you want an explicit exec pin (e.g. to control evaluation order in a sequence).
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture", meta = (DisplayName = "GetDuration (Runtime)"))
	static float ResolveVoiceCultureAwareDuration(const USoundBase* Sound);
};
