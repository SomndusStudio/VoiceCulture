/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "SSVoiceCultureEditorStyle.h"
#include "Toolkits/AssetEditorToolkit.h"

class USSVoiceCultureSound;

/**
 * Defines editor commands for the SSVoiceCultureSound editor.
 * 
 * This includes toolbar actions like Play and Stop, as well as any custom hotkeys or menu entries
 * specific to the voice culture audio editor.
 * 
 * The commands are registered with the editor framework and used to populate toolbars and menus.
 */
class FSSVoiceCultureSoundEditorCommands : public TCommands<FSSVoiceCultureSoundEditorCommands>
{
public:
	/** Constructor */
	FSSVoiceCultureSoundEditorCommands()
		: TCommands<FSSVoiceCultureSoundEditorCommands>(
			TEXT("SSVoiceCultureSoundEditor"),
			NSLOCTEXT("Contexts", "SSVoiceCultureSoundEditor", "Voice Culture Audio Editor"),
			NAME_None,
			FSSVoiceCultureStyle::GetStyleSetName()) {}
	
	/** Plays the sound */
	TSharedPtr<FUICommandInfo> Play;
	
	/** Stops the currently playing sound */
	TSharedPtr<FUICommandInfo> Stop;
	
	/** Try auto fill sounds from asset registry */
	TSharedPtr<FUICommandInfo> AutoPopulate;
	
	/** Initialize commands */
	virtual void RegisterCommands() override;
};

/**
 * Custom asset editor toolkit for SSVoiceCultureSound.
 *
 * This class handles the editor UI and logic for voice culture audios:
 * - Manages layout and tab registration
 * - Hosts the Details panel for property editing
 * - Provides toolbar integration (Play, Stop, etc.)
 * - Handles preview playback within the editor
 *
 * Inherits from FAssetEditorToolkit for seamless integration with the asset editor framework.
 * Implements FNotifyHook to react to property changes in real time.
 */
class SSVOICECULTUREEDITOR_API FSSVoiceCultureSoundEditorToolkit : public FAssetEditorToolkit, public FNotifyHook
{
public:
	void Init(USSVoiceCultureSound* InAsset, const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost);

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
	TSharedRef<SDockTab> SpawnGraphTab(const FSpawnTabArgs& Args);

	TSharedPtr<SGraphEditor> GraphEditor;
	UEdGraph* VoiceCultureGraph = nullptr;
	
	static const FName DetailsTabID;
	static const FName GraphTabID;

	void RefreshGraphEditor();
	void OnDetailsChanged(const FPropertyChangedEvent& InEvent);
	
	/** Binds new graph commands to delegates */
	void BindGraphCommands();

	void ExtendToolbar();
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	
	void PlaySound();
	void StopSound();
	void AutoPopulateCultures();
	
	TSharedPtr<FUICommandList> ToolbarCommands;
	USSVoiceCultureSound* Asset = nullptr;
};