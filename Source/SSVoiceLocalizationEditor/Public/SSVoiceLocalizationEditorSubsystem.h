/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "SSVoiceLocalizationTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/SSVoiceAutofillStrategy.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceLocalizationEditorSubsystem.generated.h"

/**
 * Editor subsystem responsible for managing voice localization editor tools and settings.
 *
 * This includes profile management, asset scanning, autofill strategies, and integration with the asset registry.
 * Only available in the editor (not included in builds).
 */
UCLASS()
class SSVOICELOCALIZATIONEDITOR_API USSVoiceLocalizationEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

protected:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	
	/**
	 * Returns a reference to the Asset Registry module.
	 * Used for querying and scanning assets in the editor.
	 */
	static FAssetRegistryModule& GetAssetRegistryModule();

	/**
	 * Changes the currently active voice localization profile by name.
	 * Will refresh internal strategy and update editor behavior accordingly.
	 *
	 * @param ProfileName The name of the profile to activate.
	 */
	void ChangeActiveProfileFromName(FString ProfileName);

	
	/**
	 * Called when the voice profile name has changed (e.g., from project settings).
	 * Used to refresh the strategy or respond to user interaction.
	 */
	void OnVoiceProfileNameChange();

	/**
	 * Refreshes the currently active autofill strategy based on the selected profile.
	 *
	 * @return A pointer to the updated voice autofill strategy.
	 */
	USSVoiceAutofillStrategy* RefreshStrategy();
	
	/**
	 * Returns the currently active strategy used to autofill voice data.
	 * Strategy is based on the selected voice localization profile.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	USSVoiceAutofillStrategy* GetActiveStrategy();

	/**
	 * Returns the currently active voice localization profile, as defined in settings.
	 */
	const FSSVoiceAutofillProfile* GetActiveProfile() const;

	/**
	 * Retrieves a specific voice localization profile by its name.
	 *
	 * @param ProfileName Name of the profile to retrieve.
	 * @return Pointer to the corresponding profile, or nullptr if not found.
	 */
	const FSSVoiceAutofillProfile* GetProfileFromName(FString ProfileName) const;

	/**
	 * Returns whether the subsystem has been properly initialized and is ready for use.
	 */
	bool IsReady() const;

	/**
	 * Retrieves all Voice Actor assets associated with a given voice actor name.
	 *
	 * @param Assets Array to populate with matching assets.
	 * @param VoiceActorName Name of the voice actor to filter by.
	 * @param bShowSlowTask Whether to show a slow task progress dialog during the scan.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	static void GetAssetsFromVoiceActor(TArray<FAssetData>& Assets, FString VoiceActorName, const bool bShowSlowTask = true);

	/**
	 * Retrieves all localized voice assets that match the culture filtering criteria.
	 *
	 * @param Assets Array to populate with results.
	 * @param bCompleteCulture If true, includes only assets with complete culture data.
	 * @param bShowSlowTask Whether to display a progress dialog.
	 */
	static void GetAssetsWithCulture(TArray<FAssetData>& Assets, const bool bCompleteCulture = true, const bool bShowSlowTask = true);

	/**
	 * Retrieves voice assets that are missing one or more required cultures.
	 *
	 * @param Assets Array to populate with incomplete assets.
	 * @param bShowSlowTask Whether to show a progress dialog.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
	static void GetAssetsWithMissingCulture(TArray<FAssetData>& Assets, const bool bShowSlowTask = true);

	/**
	 * Retrieves voice assets that have all required cultures (complete).
	 *
	 * @param Assets Array to populate with complete assets.
	 * @param bShowSlowTask Whether to show a progress dialog.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Localization")
    static void GetAssetsWithCompleteCulture(TArray<FAssetData>& Assets, const bool bShowSlowTask = true);

	
	/**
	 * Returns a reference to the Content Browser module, scoped for voice localization tools.
	 * Used to extend or filter the content browser based on voice-related workflows.
	 */
	static FContentBrowserModule& GetVoiceContentBrowser();
	
private:
	
	/** Cached pointer to the active voice autofill strategy. */
	UPROPERTY(Transient)
	USSVoiceAutofillStrategy* CachedStrategy;
};
