/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureEditorLog.h"
#include "Utils/SSVoiceCultureUtils.h"

#include "SSVoiceCultureEditorSubsystem.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceCultureStrategy.h"

void FSSVoiceCultureUtils::GenerateActorListJson()
{
	// Step 1: Use active strategy to extract actor names from the asset registry
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
	auto* Strategy = VLEditorSubsystem->GetActiveStrategy();
	
	TSet<FString> UniqueActors;

	Strategy->ExecuteExtractActorNameFromAssetRegistry(UniqueActors);

	// Step 2: Convert the list of actor names to a JSON array
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FString& Actor : UniqueActors)
	{
		JsonArray.Add(MakeShared<FJsonValueString>(Actor));
	}

	TSharedRef<FJsonValueArray> RootValue = MakeShared<FJsonValueArray>(JsonArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonArray, Writer);

	// Step 3: Save the JSON string to a file
	const FString OutputPath = FPaths::ProjectSavedDir() / TEXT("SSVoiceCulture") / TEXT("VoiceActors.json");
	FFileHelper::SaveStringToFile(OutputString, *OutputPath);

	UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoice] Saved voice actor list to %s"), *OutputPath);
}
