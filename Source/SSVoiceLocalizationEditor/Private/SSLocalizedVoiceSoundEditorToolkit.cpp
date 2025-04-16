/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSLocalizedVoiceSoundEditorToolkit.h"

#include "SSLocalizedVoiceSound.h"
#include "SSVoiceLocalizationStyle.h"

#define LOCTEXT_NAMESPACE "SSLocalizedVoiceSoundEditor"

const FName FSSLocalizedVoiceSoundEditorToolkit::DetailsTabID(TEXT("SSLocalizedVoiceSoundEditor_Details"));

void FSSLocalizedVoiceSoundEditorCommands::RegisterCommands()
{
	UI_COMMAND(Play, "Play", "Play the localized voice", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Stop, "Stop", "Stop playback", EUserInterfaceActionType::Button, FInputChord());
}

void FSSLocalizedVoiceSoundEditorToolkit::Init(USSLocalizedVoiceSound* InAsset, const EToolkitMode::Type Mode,
                                               const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	Asset = InAsset;

	FSSLocalizedVoiceSoundEditorCommands::Register();

	BindGraphCommands();

	ExtendToolbar();

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("SSLocalizedVoiceSoundEditorLayout_v1")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split(FTabManager::NewSplitter()
			        ->SetOrientation(Orient_Horizontal)
			        ->SetSizeCoefficient(0.9f)
			        ->Split
			        (
				        FTabManager::NewStack()
				        ->SetSizeCoefficient(1.0f)
				        ->AddTab(DetailsTabID, ETabState::OpenedTab)
			        ))
		);

	InitAssetEditor(Mode, InitToolkitHost,
	                FName("SSLocalizedVoiceSoundEditorApp"),
	                Layout,
	                true, true, InAsset);
}

FName FSSLocalizedVoiceSoundEditorToolkit::GetToolkitFName() const
{
	return FName("SSLocalizedVoiceSoundEditor");
}

FText FSSLocalizedVoiceSoundEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("LocalizedVoiceSoundEditor", "Localized Voice Audio");
}

FString FSSLocalizedVoiceSoundEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("VoiceSound");
}

FLinearColor FSSLocalizedVoiceSoundEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Yellow;
}

void FSSLocalizedVoiceSoundEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(DetailsTabID,
	                                 FOnSpawnTab::CreateRaw(
		                                 this, &FSSLocalizedVoiceSoundEditorToolkit::SpawnDetailsTab))
	            .SetDisplayName(NSLOCTEXT("SSLocalizedVoiceSoundEditor", "DetailsTab", "Details"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FSSLocalizedVoiceSoundEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(DetailsTabID);
}

TSharedRef<SDockTab> FSSLocalizedVoiceSoundEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	if (Asset)
	{
		DetailsView->SetObject(Asset);
	}

	return SNew(SDockTab)
		.Label(NSLOCTEXT("SSLocalizedVoiceSoundEditor", "DetailsTabLabel", "Details"))
		.TabRole(ETabRole::PanelTab)
		[
			DetailsView.ToSharedRef()
		];
}

void FSSLocalizedVoiceSoundEditorToolkit::BindGraphCommands()
{
	const FSSLocalizedVoiceSoundEditorCommands& Commands = FSSLocalizedVoiceSoundEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.Play,
		FExecuteAction::CreateSP(this, &FSSLocalizedVoiceSoundEditorToolkit::PlaySound));

	ToolkitCommands->MapAction(
		Commands.Stop,
		FExecuteAction::CreateSP(this, &FSSLocalizedVoiceSoundEditorToolkit::StopSound));
}

void FSSLocalizedVoiceSoundEditorToolkit::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &FSSLocalizedVoiceSoundEditorToolkit::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}


void FSSLocalizedVoiceSoundEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Toolbar");
	{
		ToolbarBuilder.AddToolBarButton(FSSLocalizedVoiceSoundEditorCommands::Get().Play);

		ToolbarBuilder.AddToolBarButton(FSSLocalizedVoiceSoundEditorCommands::Get().Stop);
	}

	ToolbarBuilder.EndSection();
}

void FSSLocalizedVoiceSoundEditorToolkit::PlaySound()
{
	if (Asset)
	{
		if (USoundBase* Sound = Asset->GetPreviewLocalizedSound())
		{
			GEditor->PlayPreviewSound(Sound);
		}
	}
}

void FSSLocalizedVoiceSoundEditorToolkit::StopSound()
{
	GEditor->ResetPreviewAudioComponent();
}

#undef LOCTEXT_NAMESPACE
