/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


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

	// Got all existing profiles
	for (const auto& Profile : Settings->AutofillProfiles)
	{
		ProfileOptions.Add(MakeShared<FSSVoiceAutofillProfile>(Profile));
	}

	// Default selection
	for (const auto& ProfileOption : ProfileOptions)
	{
		if (ProfileOption.Get()->ProfileName == Settings->ActiveVoiceProfileName)
			CurrentSelection = ProfileOption;
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