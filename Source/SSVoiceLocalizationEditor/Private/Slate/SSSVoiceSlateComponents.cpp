// Fill out your copyright notice in the Description page of Project Settings.


#include "SSVoiceLocalizationEditor/Public/Slate/SSSVoiceSlateComponents.h"

#define LOCTEXT_NAMESPACE "SSVoiceProfileSelector"

////////////////////////////////////////////////////////////////////
// Autofill Profile Selector

void SSSAutofillProfileSelector::Construct(const FArguments& InArgs)
{
	OnProfileChangedDelegate = InArgs._OnProfileChanged;

	LoadProfiles();

	ChildSlot
	[
		SAssignNew(ComboBox, SComboBox<TSharedPtr<FSSVoiceAutofillProfile>>)
		.OptionsSource(&ProfileOptions)
		.OnGenerateWidget(this, &SSSAutofillProfileSelector::GenerateProfileItem)
		.OnSelectionChanged(this, &SSSAutofillProfileSelector::HandleProfileChanged)
		.InitiallySelectedItem(CurrentSelection)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return CurrentSelection.IsValid()
					? FText::FromString(CurrentSelection->ProfileName)
					: LOCTEXT("SelectProfile", "Select profile...");
			})
		]
	];
}

void SSSAutofillProfileSelector::LoadProfiles()
{
	const USSVoiceLocalizationEditorSettings* Settings = GetDefault<USSVoiceLocalizationEditorSettings>();
	if (!Settings)
		return;

	for (const FSSVoiceAutofillProfile& Profile : Settings->AutofillProfiles)
	{
		ProfileOptions.Add(MakeShared<FSSVoiceAutofillProfile>(Profile));
	}

	if (ProfileOptions.Num() > 0)
	{
		CurrentSelection = ProfileOptions[0];
	}
}

TSharedRef<SWidget> SSSAutofillProfileSelector::GenerateProfileItem(TSharedPtr<FSSVoiceAutofillProfile> InItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(InItem.IsValid() ? InItem->ProfileName : TEXT("Invalid")));
}

void SSSAutofillProfileSelector::HandleProfileChanged(TSharedPtr<FSSVoiceAutofillProfile> NewSelection, ESelectInfo::Type)
{
	CurrentSelection = NewSelection;

	if (OnProfileChangedDelegate.IsBound())
	{
		OnProfileChangedDelegate.Execute();
	}
}

TSharedPtr<FSSVoiceAutofillProfile> SSSAutofillProfileSelector::GetSelectedProfile() const
{
	return CurrentSelection;
}

#undef LOCTEXT_NAMESPACE