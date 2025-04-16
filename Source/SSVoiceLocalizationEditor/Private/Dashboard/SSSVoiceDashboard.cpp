// Fill out your copyright notice in the Description page of Project Settings.


#include "Dashboard/SSSVoiceDashboard.h"

#include "Misc/ScopedSlowTask.h"
#include "Slate/SSSVoiceSlateComponents.h"
#include "Utils/SSVoiceLocalizationUI.h"
#include "Utils/SSVoiceLocalizationUtils.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"

#define LOCTEXT_NAMESPACE "SVoiceDashboard"

void SSSVoiceDashboard::Construct(const FArguments& InArgs)
{
	// Load report
	FSSVoiceLocalizationUtils::LoadSavedCultureReport(CultureReport);

	ChildSlot
	[
		SNew(SSplitter)
		+ SSplitter::Slot()
		.Value(0.25f)
		[
			BuildLeftPanel()
		]
		+ SSplitter::Slot()
		.Value(0.75f)
		[
			BuildRightPanel()
		]
	];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildLeftPanel()
{
	return SNew(SScrollBox)

			// --- Profile Selector
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProfileSelector", "Profile Selector"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(SSSAutofillProfileSelector)
					.OnProfileChanged(FSimpleDelegate::CreateLambda([this]()
					{
						// Callback when profile change
						UE_LOG(LogTemp, Log, TEXT("[SSVoice] Profile changed from selector widget"));

						// TODO: Rafraîchir le contenu si besoin
					}))
				]
			]

			// --- Culture Filter
			+ SScrollBox::Slot().Padding(4)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CultureFilterTitle", "Culture Filter"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SNew(STextBlock)
					.Text(FText::FromString("→ [CheckBoxes per culture]"))
				]
			]

			// --- Cook Settings
			+ SScrollBox::Slot().Padding(4)
			[
				SNew(SExpandableArea)
				.AreaTitle(LOCTEXT("CookSettings", "Cooking Settings"))
				.BodyContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("→ [Cook culture selection]"))
				]
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildRightPanel()
{
	return SNew(SVerticalBox)

			// --- Coverage Overview
			+ SVerticalBox::Slot().AutoHeight().Padding(6)
			[
				BuildCoverageSection()
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(SSeparator)
			]

			// --- Asset List
			+ SVerticalBox::Slot().FillHeight(1.0f).Padding(8)
			[
				BuildAssetList()
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildAssetList()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AssetsHeader", "Localized Voice Assets"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(4)
		[
			SNew(STextBlock)
			.Text(FText::FromString("→ [ListView of assets filtered by culture/profile]"))
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildCultureListWidget()
{
	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);

	for (const FSSVoiceCultureReportEntry& Entry : CultureReport.Entries)
	{
		const float Progress = Entry.GetCoveragePercent();

		List->AddSlot().AutoHeight().Padding(4)
		[
			SNew(SHorizontalBox)

			// Culture name
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry.Culture))
				.MinDesiredWidth(60)
			]

			// Progress bar
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(4)
			[
				SNew(SProgressBar)
				.Percent(Progress)
			]

			// Percentage label
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					LOCTEXT("CoverageFormat", "{0}%"),
					FText::AsNumber(FMath::RoundToInt(Progress * 100.f))
				))
				.MinDesiredWidth(40)
			]

			// Autofill button
			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[
				SNew(SButton)
				.Text(LOCTEXT("AutofillCultureBtn", "Auto-fill"))
				.OnClicked_Lambda([Culture = Entry.Culture]()
				{
					// TODO : lancer l’autofill ciblé
					UE_LOG(LogTemp, Log, TEXT("[SSVoice] Requested autofill for culture: %s"), *Culture);
					return FReply::Handled();
				})
			]
		];
	}

	return List;
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildCoverageSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CultureCoverageTitle", "Voice Culture Coverage"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RegenerateReportBtn", "Generate Report"))
				.ToolTipText(LOCTEXT("RegenerateReportTooltip",
				                     "Scan all voice assets and generate the latest culture coverage report."))
				.OnClicked_Lambda([this]()
				{
					// 1. Créer le slow task pour l’utilisateur
					TSharedPtr<FScopedSlowTask> SlowTask = MakeShared<FScopedSlowTask>(
						1.f,
						LOCTEXT("ScanningCultures", "Scanning voice cultures...")
					);
					SlowTask->MakeDialog(true);

					// 2. Lancer l'opération async
					FSSVoiceLocalizationUtils::GenerateCultureCoverageReportAsync(
						[this, SlowTask](const FSSVoiceCultureReport& Report)
						{
							CultureReport = Report;

							// Fermer la tâche + reconstruire UI
							if (SlowTask.IsValid())
							{
								SlowTask->EnterProgressFrame(1.f); // facultatif ici
							}

							FSSVoiceLocalizationUI::NotifySuccess(NSLOCTEXT("SSVoice", "ScanningCulturesSuccess", "Scanning cultures with success"));
							
							// Rebuild UI
							this->Construct(SSSVoiceDashboard::FArguments());
						}
					);

					return FReply::Handled();
				})
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(SSeparator)
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				// Loop on each culture
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					BuildCultureListWidget()
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
