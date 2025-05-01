/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphNode.h"
#include "SSVoiceCultureEditorLog.h"
#include "Widgets/SCompoundWidget.h"
#include "Settings/SSVoiceCultureEditorSettings.h"

#include "SSVoiceCultureSlateComponents.generated.h"

////////////////////////////////////////////////////////////////////
// Profile Selector

/**
 * Reusable dropdown widget to select an Profile
 */
class SSVOICECULTUREEDITOR_API SSSVoiceEditorProfileSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSSVoiceEditorProfileSelector) {}
	SLATE_EVENT(FSimpleDelegate, OnProfileChanged)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

	/** Returns currently selected profile (can be null) */
	TSharedPtr<FSSVoiceStrategyProfile> GetSelectedProfile() const;

private:
	void LoadProfiles();

	TSharedRef<SWidget> GenerateProfileItem(TSharedPtr<FSSVoiceStrategyProfile> InItem);
	void HandleProfileChanged(TSharedPtr<FSSVoiceStrategyProfile> NewSelection, ESelectInfo::Type);

	TArray<TSharedPtr<FSSVoiceStrategyProfile>> ProfileOptions;
	TSharedPtr<FSSVoiceStrategyProfile> CurrentSelection;
	TSharedPtr<SComboBox<TSharedPtr<FSSVoiceStrategyProfile>>> ComboBox;

	FSimpleDelegate OnProfileChangedDelegate;
};

DECLARE_DELEGATE_OneParam(FOnPreviewLanguageChangedDelegate, const FString& /*NewLanguage*/)

////////////////////////////////////////////////////////////////////
// SSSVoiceCulturePreviewLanguageSelector

/**
 * Widget for selecting the voice preview language, bound to USSVoiceCultureSettings.
 */
class SSVOICECULTUREEDITOR_API SSSVoiceCulturePreviewLanguageSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSSVoiceCulturePreviewLanguageSelector) {}
	SLATE_EVENT(FOnPreviewLanguageChangedDelegate, OnLanguageChanged)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	// Delegate called when user changes language
	FOnPreviewLanguageChangedDelegate OnLanguageChanged;

	// List of languages
	TSharedPtr<TArray<TSharedPtr<FString>>> Languages;

	// Currently selected language (for display binding)
	TSharedPtr<FString> SelectedLanguage;

	// Combo box widget
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBox;

	// Refreshes the list of available cultures from settings
	void BuildLanguageList();

	// Respond to global changes
	void HandleGlobalLanguageChanged(const FString& NewLang);
};

////////////////////////////////////////////////////////////////////
// SSSVoiceCultureGraphPanel

UCLASS()
class USSVoiceCultureGraphNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	TObjectPtr<USSVoiceCultureSound> SourceAsset = nullptr;
	
	UPROPERTY()
	FString Culture;

	/**
	 * Returns the associated USoundBase for this graph node, based on its culture setting.
	 *
	 * This function searches the parent voice culture asset for a matching entry corresponding
	 * to the culture defined in this node. If found, it returns the associated sound.
	 *
	 * @return The localized USoundBase for this node's culture, or nullptr if not found.
	 */
	USoundBase* GetSoundBase();
	
	// Required override
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override
	{
		return FText::FromString(Culture);
	}

	virtual void AllocateDefaultPins() override;
};

UCLASS()
class USSVoiceCultureGraph : public UEdGraph
{
	GENERATED_BODY()
};

class SSSVoiceCultureGraphNodeVisual : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SSSVoiceCultureGraphNodeVisual) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, USSVoiceCultureGraphNode* InNode);

	// SGraphNode override
	virtual void UpdateGraphNode() override;
};

class FSSVoiceCultureGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override;
};