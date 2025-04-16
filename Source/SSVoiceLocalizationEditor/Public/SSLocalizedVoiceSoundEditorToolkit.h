/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceLocalizationStyle.h"
#include "Toolkits/AssetEditorToolkit.h"

class USSLocalizedVoiceSound;

/**
 * Defines editor commands for the SSLocalizedVoiceSound editor.
 * 
 * This includes toolbar actions like Play and Stop, as well as any custom hotkeys or menu entries
 * specific to the localized voice audio editor.
 * 
 * The commands are registered with the editor framework and used to populate toolbars and menus.
 */
class FSSLocalizedVoiceSoundEditorCommands : public TCommands<FSSLocalizedVoiceSoundEditorCommands>
{
public:
	/** Constructor */
	FSSLocalizedVoiceSoundEditorCommands()
		: TCommands<FSSLocalizedVoiceSoundEditorCommands>(
			TEXT("SSLocalizedVoiceSoundEditor"),
			NSLOCTEXT("Contexts", "SSLocalizedVoiceSoundEditor", "Localized Voice Audio Editor"),
			NAME_None,
			FSSVoiceLocalizationStyle::GetStyleSetName()) {}
	
	/** Plays the sound */
	TSharedPtr<FUICommandInfo> Play;
	
	/** Stops the currently playing sound */
	TSharedPtr<FUICommandInfo> Stop;

		
	/** Try auto fill sounds from asset registry */
	TSharedPtr<FUICommandInfo> AutoFill;
	
	/** Initialize commands */
	virtual void RegisterCommands() override;
};

/**
 * Custom asset editor toolkit for SSLocalizedVoiceSound.
 *
 * This class handles the editor UI and logic for localized voice audios:
 * - Manages layout and tab registration
 * - Hosts the Details panel for property editing
 * - Provides toolbar integration (Play, Stop, etc.)
 * - Handles preview playback within the editor
 *
 * Inherits from FAssetEditorToolkit for seamless integration with the asset editor framework.
 * Implements FNotifyHook to react to property changes in real time.
 */
class SSVOICELOCALIZATIONEDITOR_API FSSLocalizedVoiceSoundEditorToolkit : public FAssetEditorToolkit, public FNotifyHook
{
public:
	void Init(USSLocalizedVoiceSound* InAsset, const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost);

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	
private:

	TSharedPtr<IDetailsView> DetailsView;
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);

	static const FName DetailsTabID;
	
	/** Binds new graph commands to delegates */
	void BindGraphCommands();

	void ExtendToolbar();
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	
	void PlaySound();
	void StopSound();
	void AutoFillLocalizedVoices();
	
	TSharedPtr<FUICommandList> ToolbarCommands;
	USSLocalizedVoiceSound* Asset;
};