/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Utils/SSVoiceLocalizationUtils.h"

#include "JsonObjectConverter.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "SSVoiceLocalizationSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceAutofillStrategy.h"
#include "Settings/SSVoiceLocalizationEditorSettings.h"
#include "Sound/SoundCue.h"
#include "Utils/SSVoiceLocalizationUI.h"

void FSSVoiceLocalizationUtils::GenerateActorListJson()
{
	// 1. Prepare registry
	FAssetRegistryModule& AssetRegistry = USSVoiceLocalizationEditorSubsystem::GetAssetRegistryModule();

	FARFilter Filter;
	Filter.ClassPaths.Add(USSLocalizedVoiceSound::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game"));

	TArray<FAssetData> FoundAssets;
	AssetRegistry.Get().GetAssets(Filter, FoundAssets);

	// 2. Parse actor names
	TSet<FString> UniqueActors;

	for (const FAssetData& AssetData : FoundAssets)
	{
		const FString AssetName = AssetData.AssetName.ToString(); // e.g. LVA_NPC01_Hello
		TArray<FString> Parts;
		AssetName.ParseIntoArray(Parts, TEXT("_"));

		if (Parts.Num() >= 3)
		{
			UniqueActors.Add(Parts[1]); // Actor = 2nd part
		}
	}

	// 3. Convert to JSON array
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
