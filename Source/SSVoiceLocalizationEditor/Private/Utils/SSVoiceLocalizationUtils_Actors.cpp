/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Utils/SSVoiceLocalizationUtils.h"

#include "SSVoiceLocalizationEditorSubsystem.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceAutofillStrategy.h"

void FSSVoiceLocalizationUtils::GenerateActorListJson()
{
	// 1. Use active strategy 
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
	auto* Strategy = VLEditorSubsystem->GetActiveStrategy();
	
	TSet<FString> UniqueActors;

	Strategy->ExecuteExtractActorNameFromAssetRegistry(UniqueActors);
	
	// 2. Convert to JSON array
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FString& Actor : UniqueActors)
	{
		JsonArray.Add(MakeShared<FJsonValueString>(Actor));
	}

	TSharedRef<FJsonValueArray> RootValue = MakeShared<FJsonValueArray>(JsonArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonArray, Writer);

	// 4. Save to file
	const FString OutputPath = FPaths::ProjectSavedDir() / TEXT("SSVoiceLocalization") / TEXT("VoiceActors.json");
	FFileHelper::SaveStringToFile(OutputString, *OutputPath);

	UE_LOG(LogTemp, Log, TEXT("[SSVoice] Saved voice actor list to %s"), *OutputPath);
}
