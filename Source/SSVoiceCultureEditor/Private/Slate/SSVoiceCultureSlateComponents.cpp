/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureEditor/Public/Slate/SSVoiceCultureSlateComponents.h"

#include "DetailLayoutBuilder.h"
#include "EditorStyleSet.h"
#include "PropertyCustomizationHelpers.h"
#include "SGraphPanel.h"
#include "SSVoiceCultureSettings.h"
#include "SSVoiceStyleCompat.h"
#include "Utils/SSVoiceCultureUI.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

////////////////////////////////////////////////////////////////////
// Profile Selector

void SSSVoiceEditorProfileSelector::Construct(const FArguments& InArgs)
{
	OnProfileChangedDelegate = InArgs._OnProfileChanged;

	LoadProfiles();

	ChildSlot
	[
		SAssignNew(ComboBox, SComboBox<TSharedPtr<FSSVoiceStrategyProfile>>)
		.OptionsSource(&ProfileOptions)
		.OnGenerateWidget(this, &SSSVoiceEditorProfileSelector::GenerateProfileItem)
		.OnSelectionChanged(this, &SSSVoiceEditorProfileSelector::HandleProfileChanged)
		.InitiallySelectedItem(CurrentSelection)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return CurrentSelection.IsValid()
					       ? FText::FromString(CurrentSelection->ProfileName)
					       : NSLOCTEXT("SSVoiceCultureEditor", "SelectProfile", "Select profile...");
			})
		]
	];
}

void SSSVoiceEditorProfileSelector::LoadProfiles()
{
	const USSVoiceCultureEditorSettings* Settings = GetDefault<USSVoiceCultureEditorSettings>();
	if (!Settings)
		return;

	// Got all existing profiles
	for (const auto& Profile : Settings->StrategyProfiles)
	{
		ProfileOptions.Add(MakeShared<FSSVoiceStrategyProfile>(Profile));
	}

	// Default selection
	for (const auto& ProfileOption : ProfileOptions)
	{
		if (ProfileOption.Get()->ProfileName == Settings->ActiveVoiceProfileName)
			CurrentSelection = ProfileOption;
	}
}

TSharedRef<SWidget> SSSVoiceEditorProfileSelector::GenerateProfileItem(TSharedPtr<FSSVoiceStrategyProfile> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(InItem.IsValid() ? InItem->ProfileName : TEXT("Invalid")));
}

void SSSVoiceEditorProfileSelector::HandleProfileChanged(TSharedPtr<FSSVoiceStrategyProfile> NewSelection,
                                                      ESelectInfo::Type)
{
	CurrentSelection = NewSelection;

	if (OnProfileChangedDelegate.IsBound())
	{
		OnProfileChangedDelegate.Execute();
	}
}

TSharedPtr<FSSVoiceStrategyProfile> SSSVoiceEditorProfileSelector::GetSelectedProfile() const
{
	return CurrentSelection;
}

////////////////////////////////////////////////////////////////////
// Preview Language Selector
void SSSVoiceCulturePreviewLanguageSelector::Construct(const FArguments& InArgs)
{
	OnLanguageChanged = InArgs._OnLanguageChanged;

	Languages = MakeShared<TArray<TSharedPtr<FString>>>();
	BuildLanguageList();

	const FString CurrentLanguage = USSVoiceCultureSettings::GetSetting()->PreviewLanguage;

	// Find selected item
	for (const TSharedPtr<FString>& Lang : *Languages)
	{
		if (*Lang == CurrentLanguage)
		{
			SelectedLanguage = Lang;
			break;
		}
	}
	if (!SelectedLanguage.IsValid() && Languages->Num() > 0)
	{
		SelectedLanguage = (*Languages)[0];
	}

	// Listen to global updates
	USSVoiceCultureSettings::OnPreviewLanguageChanged.AddSP(
		this, &SSSVoiceCulturePreviewLanguageSelector::HandleGlobalLanguageChanged);

	ChildSlot
	[
		SNew(SHorizontalBox)

		//Label : "Preview Voice Language"
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("SSVoiceCultureEditor", "PreviewVoiceLanguageLabel", "Preview Voice Language:"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		// ComboBox
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SAssignNew(ComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(Languages.Get())
			.InitiallySelectedItem(SelectedLanguage)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
			{
				const FString CultureLabel = FSSVoiceCultureUI::CultureAsDisplay(*InItem);

				return SNew(STextBlock).Text(FText::FromString(CultureLabel));
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
			{
				if (NewSelection.IsValid())
				{
					const FString& NewLang = *NewSelection;
					const FString Current = USSVoiceCultureSettings::GetSetting()->PreviewLanguage;

					if (!Current.Equals(NewLang, ESearchCase::IgnoreCase))
					{
						USSVoiceCultureSettings::SetPreviewLanguage(NewLang);

						if (OnLanguageChanged.IsBound())
						{
							OnLanguageChanged.Execute(NewLang);
						}
					}
				}
			})
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!SelectedLanguage.IsValid())
					{
						return NSLOCTEXT("SSVoiceCulture", "NoCulture", "None");
					}
					// Use your centralized display function
					const FString DisplayText = FSSVoiceCultureUI::CultureAsDisplay(*SelectedLanguage);
					return FText::FromString(DisplayText);
				})
			]
		]
	];
}

void SSSVoiceCulturePreviewLanguageSelector::BuildLanguageList()
{
	Languages->Empty();

	const TSet<FString>& Cultures = USSVoiceCultureSettings::GetSetting()->SupportedVoiceCultures;
	for (const FString& Culture : Cultures)
	{
		Languages->Add(MakeShared<FString>(Culture));
	}

	// Fallback
	if (Languages->Num() == 0)
	{
		Languages->Add(MakeShared<FString>(TEXT("en")));
	}
}

void SSSVoiceCulturePreviewLanguageSelector::HandleGlobalLanguageChanged(const FString& NewLang)
{
	for (const auto& Lang : *Languages)
	{
		if (*Lang == NewLang)
		{
			SelectedLanguage = Lang;

			if (ComboBox.IsValid())
			{
				ComboBox->SetSelectedItem(Lang);
			}
			break;
		}
	}
}

USoundBase* USSVoiceCultureGraphNode::GetSoundBase()
{
	if (!SourceAsset) return nullptr;

	// Normalize culture code for consistent comparison
	const FString CultureCode = Culture.ToLower();

	// Look for the matching culture entry in the parent asset
	const FSSCultureAudioEntry* Entry = SourceAsset->VoiceCultures.FindByPredicate(
		[&](const FSSCultureAudioEntry& E)
		{
			return E.Culture.ToLower() == CultureCode;
		});

	// Return the associated sound if found
	if (!Entry)
	{
		return nullptr;
	}
	return Entry->Sound;
}

void USSVoiceCultureGraphNode::AllocateDefaultPins()
{
	// Even if no pins are used, call base or leave empty.
}


////////////////////////////////////////////////////////////////////
// SSSVoiceCultureGraphPanel

void SSSVoiceCultureGraphNodeVisual::Construct(const FArguments& InArgs, USSVoiceCultureGraphNode* InNode)
{
	GraphNode = InNode;

	ensure(GraphNode != nullptr);
	ensure(GraphNode->IsValidLowLevelFast());

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SSSVoiceCultureGraphNodeVisual::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	ensure(GraphNode);

	USSVoiceCultureGraphNode* VCGraphNode = Cast<USSVoiceCultureGraphNode>(GraphNode);

	auto* SoundBase = VCGraphNode->GetSoundBase();
	
	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.Node.Body"))
			.Padding(8)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(VCGraphNode->Culture))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(USoundBase::StaticClass())
					.ObjectPath(SoundBase ? SoundBase->GetPathName() : FString())
					.OnObjectChanged_Lambda([this, VCGraphNode](const FAssetData& NewAsset)
					{
						// Update node memory
						USoundBase* NewSound = Cast<USoundBase>(NewAsset.GetAsset());

						// Update actual VoiceCultures array
						FString TargetCulture = VCGraphNode->Culture.ToLower();

						TArray<FSSCultureAudioEntry>& Cultures = VCGraphNode->SourceAsset->VoiceCultures;

						// Try to find existing entry
						FSSCultureAudioEntry* Existing = Cultures.FindByPredicate([&](const FSSCultureAudioEntry& Entry)
						{
							return Entry.Culture.ToLower() == TargetCulture;
						});

						if (Existing)
						{
							Existing->Sound = NewSound;
						}
						else
						{
							// Add new entry
							FSSCultureAudioEntry NewEntry;
							NewEntry.Culture = VCGraphNode->Culture;
							NewEntry.Sound = NewSound;
							Cultures.Add(NewEntry);
						}

						// Mark dirty for save
						VCGraphNode->SourceAsset->MarkPackageDirty();

						// Refresh node
						UpdateGraphNode();
					})
					.DisplayUseSelected(true)
					.DisplayBrowse(true)
					.AllowClear(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
					[
						SNew(SButton)
						             .ButtonStyle(FAppStyle::Get(), "SimpleButton") // Flat button
						             .ContentPadding(4)
						             .ToolTipText(NSLOCTEXT("SSVoiceCulture", "PlaySound", "Play this sound"))
						             .OnClicked_Lambda([Sound = SoundBase]()
						             {
							             if (Sound)
							             {
								             GEditor->PlayPreviewSound(Sound);
							             }
							             return FReply::Handled();
						             })
						[
							SNew(SImage)
							.Image(SSVoiceStyleCompat::GetBrush("MediaAsset.AssetActions.Play.Small"))
						]
					]

					+ SHorizontalBox::Slot().AutoWidth().Padding(2, 0)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(4)
						.ToolTipText(NSLOCTEXT("SSVoiceCulture", "StopSound", "Stop sound preview"))
						.OnClicked_Lambda([]()
						{
							GEditor->ResetPreviewAudioComponent();
							return FReply::Handled();
						})
						[
							SNew(SImage)
							.Image(SSVoiceStyleCompat::GetBrush("MediaAsset.AssetActions.Stop.Small"))
						]
					]
				]
			]
		];
}


#undef LOCTEXT_NAMESPACE
