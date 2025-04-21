/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureSoundEditorToolkit.h"

#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureSettings.h"
#include "SSVoiceCultureSound.h"
#include "Slate/SSVoiceCultureSlateComponents.h"
#include "Utils/SSVoiceCultureUtils.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

const FName FSSVoiceCultureSoundEditorToolkit::DetailsTabID(TEXT("SSVoiceCultureSoundEditor_Details"));
const FName FSSVoiceCultureSoundEditorToolkit::GraphTabID(TEXT("SSVoiceCultureSoundEditor_Graph"));

void FSSVoiceCultureSoundEditorCommands::RegisterCommands()
{
	UI_COMMAND(Play, "Play", "Play the voice culture", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Stop, "Stop", "Stop playback", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AutoPopulate, "AutoPopulate", "Automatically populate voice culture entries based on naming convention",
	           EUserInterfaceActionType::Button, FInputChord());
}

void FSSVoiceCultureSoundEditorToolkit::Init(USSVoiceCultureSound* InAsset, const EToolkitMode::Type Mode,
                                             const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	Asset = InAsset;

	BindGraphCommands();

	ExtendToolbar();

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("SSVoiceCultureSoundEditorLayout_v1")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
#if ENGINE_MAJOR_VERSION < 5
			->Split(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
#endif
			->Split(FTabManager::NewSplitter()
			        ->SetOrientation(Orient_Horizontal)
			        ->SetSizeCoefficient(0.9f)
			        ->Split
			        (
				        FTabManager::NewStack()
				        ->SetSizeCoefficient(0.35f)
				        ->AddTab(DetailsTabID, ETabState::OpenedTab)
			        )
			        ->Split(
				        FTabManager::NewStack()
				        ->SetSizeCoefficient(0.65f)
				        ->AddTab(GraphTabID, ETabState::OpenedTab)
			        )
			)
		);

	InitAssetEditor(Mode, InitToolkitHost,
	                FName("SSVoiceCultureSoundEditorApp"),
	                Layout,
	                true, true, InAsset);
}

FName FSSVoiceCultureSoundEditorToolkit::GetToolkitFName() const
{
	return FName("SSVoiceCultureSoundEditor");
}

FText FSSVoiceCultureSoundEditorToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("SSVoiceCultureEditor", "VoiceCultureSoundEditor", "Voice Culture Audio");
}

FString FSSVoiceCultureSoundEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("VoiceSound");
}

FLinearColor FSSVoiceCultureSoundEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Yellow;
}

void FSSVoiceCultureSoundEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(DetailsTabID,
	                                 FOnSpawnTab::CreateRaw(
		                                 this, &FSSVoiceCultureSoundEditorToolkit::SpawnDetailsTab))
	            .SetDisplayName(NSLOCTEXT("SSVoiceCultureEditor", "DetailsTab", "Details"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(GraphTabID,
	                                 FOnSpawnTab::CreateRaw(this, &FSSVoiceCultureSoundEditorToolkit::SpawnGraphTab))
	            .SetDisplayName(NSLOCTEXT("SSVoiceCultureEditor", "GraphTab", "Graph"))
	            .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FSSVoiceCultureSoundEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(DetailsTabID);
}

TSharedRef<SDockTab> FSSVoiceCultureSoundEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->OnFinishedChangingProperties().AddSP(this, &FSSVoiceCultureSoundEditorToolkit::OnDetailsChanged);
	
	if (Asset)
	{
		DetailsView->SetObject(Asset);
	}

	return SNew(SDockTab)
		.Label(NSLOCTEXT("SSVoiceCultureSoundEditor", "DetailsTabLabel", "Details"))
		.TabRole(ETabRole::PanelTab)
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FSSVoiceCultureSoundEditorToolkit::SpawnGraphTab(const FSpawnTabArgs& Args)
{
	// Create a transient UEdGraph to display
	VoiceCultureGraph = NewObject<UEdGraph>(GetTransientPackage(), USSVoiceCultureGraph::StaticClass());
	VoiceCultureGraph->Schema = UEdGraphSchema::StaticClass(); // You can define a custom schema later
	
	// Convert each VoiceCulture entry into a graph node
	if (Asset)
	{
		const int32 PaddingY = 150; // Espace entre chaque node
		const int32 StartX = 100;
		const int32 StartY = 100;

		int32 Index = 0;
		for (const FString& Culture : USSVoiceCultureSettings::GetSetting()->SupportedVoiceCultures)
		{
			FSSCultureAudioEntry* Entry = Asset->VoiceCultures.FindByPredicate(
			[&](const FSSCultureAudioEntry& E)
			{
				return E.Culture.ToLower() == Culture.ToLower();
			});

			USoundBase* AssignedSound = Entry ? Entry->Sound : nullptr;
			
			USSVoiceCultureGraphNode* GraphNode = NewObject<USSVoiceCultureGraphNode>(VoiceCultureGraph);
			GraphNode->SetFlags(RF_Transactional);
			GraphNode->Rename(nullptr, VoiceCultureGraph);

			GraphNode->SourceAsset = Asset;
			GraphNode->Culture = Culture;

			GraphNode->NodePosX = StartX;                     // Every node in same column
			GraphNode->NodePosY = StartY + Index * PaddingY;  // Vertical padding

			UE_LOG(LogTemp, Warning, TEXT("Added node of class %s at pos %d, %d"),
			       *GraphNode->GetClass()->GetName(),
			       GraphNode->NodePosX,
			       GraphNode->NodePosY);

			VoiceCultureGraph->AddNode(GraphNode);

			Index++;
		}
	}

	// Create the GraphEditor
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = NSLOCTEXT("SSVoiceCultureEditor", "GraphCorner", "Voice Cultures");

	GraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(nullptr)
		.Appearance(AppearanceInfo)
		.GraphToEdit(VoiceCultureGraph)
		.IsEditable(false); // You can make this editable later


	return SNew(SDockTab)
		.Label(NSLOCTEXT("SSVoiceCultureEditor", "GraphTabLabel", "Graph"))
		.TabRole(ETabRole::PanelTab)
		[
			GraphEditor.ToSharedRef()
		];
}

void FSSVoiceCultureSoundEditorToolkit::RefreshGraphEditor()
{
	if (!GraphEditor.IsValid())
		return;

	UEdGraph* Graph = GraphEditor->GetCurrentGraph();
	if (!Graph)
		return;

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (!IsValid(Node))
			continue;

		// Cette ligne marque le noeud comme modifié, ce qui va forcer un redraw
		Node->Modify();

		// Optionnel: Notify the node that it changed
		Node->ReconstructNode();
	}

	Graph->NotifyGraphChanged();
}

void FSSVoiceCultureSoundEditorToolkit::OnDetailsChanged(const FPropertyChangedEvent& InEvent)
{
	if (!InEvent.Property)
		return;
	
	const FName PropertyName = InEvent.Property->GetFName();
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USSVoiceCultureSound, VoiceCultures))
	{
		RefreshGraphEditor();
	
		UE_LOG(LogTemp, Log, TEXT("[VoiceCulture] Graph nodes updated after property change."));
	}
}

void FSSVoiceCultureSoundEditorToolkit::BindGraphCommands()
{
	const FSSVoiceCultureSoundEditorCommands& Commands = FSSVoiceCultureSoundEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.Play,
		FExecuteAction::CreateSP(this, &FSSVoiceCultureSoundEditorToolkit::PlaySound));

	ToolkitCommands->MapAction(
		Commands.Stop,
		FExecuteAction::CreateSP(this, &FSSVoiceCultureSoundEditorToolkit::StopSound));

	ToolkitCommands->MapAction(
		FSSVoiceCultureSoundEditorCommands::Get().AutoPopulate,
		FExecuteAction::CreateSP(this, &FSSVoiceCultureSoundEditorToolkit::AutoPopulateCultures)
	);
}

void FSSVoiceCultureSoundEditorToolkit::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &FSSVoiceCultureSoundEditorToolkit::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}


void FSSVoiceCultureSoundEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Toolbar");
	{
		ToolbarBuilder.AddToolBarButton(FSSVoiceCultureSoundEditorCommands::Get().Play);
		ToolbarBuilder.AddToolBarButton(FSSVoiceCultureSoundEditorCommands::Get().Stop);
		ToolbarBuilder.AddToolBarButton(FSSVoiceCultureSoundEditorCommands::Get().AutoPopulate);

		ToolbarBuilder.AddSeparator();

		ToolbarBuilder.AddWidget(SNew(SSSVoiceCulturePreviewLanguageSelector));
	}

	ToolbarBuilder.EndSection();
}

void FSSVoiceCultureSoundEditorToolkit::PlaySound()
{
	if (Asset)
	{
		if (USoundBase* Sound = Asset->GetEditorPreviewSound())
		{
			GEditor->PlayPreviewSound(Sound);
		}
	}
}

void FSSVoiceCultureSoundEditorToolkit::StopSound()
{
	GEditor->ResetPreviewAudioComponent();
}

void FSSVoiceCultureSoundEditorToolkit::AutoPopulateCultures()
{
	if (!Asset)
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] AutoPopulate: Invalid asset"));
		return;
	}

	if (FSSVoiceCultureUtils::AutoPopulateFromNaming(Asset))
	{
		// Refresh details panels
		if (DetailsView.IsValid())
		{
			DetailsView->ForceRefresh();
		}

		// Refresh graphs
		RefreshGraphEditor();
		
		UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoice] AutoPopulate complete for '%s'"), *Asset->GetName());
	}
	else
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] AutoPopulate: No matching entries found for '%s'"),
		       *Asset->GetName());
	}
}

#undef LOCTEXT_NAMESPACE
