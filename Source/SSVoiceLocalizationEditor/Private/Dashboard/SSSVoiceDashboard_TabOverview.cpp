/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "Dashboard/SSSVoiceDashboard.h"

#include "EditorStyleSet.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "Slate/SSSVoiceSlateComponents.h"
#include "Utils/SSVoiceLocalizationUI.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Utils/SSVoiceLocalizationUtils.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SSVoiceLocalization"

////////////////////////////////////////////////////////////////////
// Utilities
TSharedRef<SWidget> BuildKeyValueRow(FText Label, TAttribute<FText> Value)
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]

		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0)
		[
			SNew(STextBlock)
			.Text(Value)
			.TextStyle(FAppStyle::Get(), "HintText")
		];
}

TSharedRef<SWidget> BuildKeyValueSectionContent(const TArray<TPair<FText, TAttribute<FText>>>& Rows)
{
	TSharedRef<SVerticalBox> Container = SNew(SVerticalBox);

	for (const auto& Row : Rows)
	{
		Container->AddSlot()
		         .AutoHeight().Padding(4, 2)
		[
			BuildKeyValueRow(Row.Key, Row.Value)
		];
	}

	return Container;
}

TSharedRef<SWidget> BuildKeyValueSection(FText SectionTitle, TArray<TPair<FText, TAttribute<FText>>> Rows)
{
	return SNew(SVerticalBox)

			// Section title
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 4)
			[
				SNew(STextBlock)
				.Text(SectionTitle)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
			]

			// Dynamic rows
			+ SVerticalBox::Slot().AutoHeight()
			[
				BuildKeyValueSectionContent(Rows)
			];
}

////////////////////////////////////////////////////////////////////
// Overview tab

TSharedRef<SDockTab> SSSVoiceDashboard::SpawnDashboardTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(3.0f)
			.Style(FEditorStyle::Get(), "DetailsView.Splitter")
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().Padding(10.f)
				[
					BuildLeftPanel()
				]
			]
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().Padding(10.f)
				[
					BuildRightPanel()
				]
			]
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildProfileSectionWidget()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Profile", "Profile"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SAssignNew(ProfileSelector, SSSAutofillProfileSelector)
			.OnProfileChanged(FSimpleDelegate::CreateLambda([this]()
			{
				auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
				VLEditorSubsystem->
					ChangeActiveProfileFromName(ProfileSelector->GetSelectedProfile().Get()->ProfileName);

				RefreshProfileSection();
			}))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			BuildKeyValueSection(
				LOCTEXT("LocalizedVoiceSound", "Localized Voice Sound"),
				{
					TPair<FText, TAttribute<FText>>(
						LOCTEXT("Pattern", "Pattern:"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(this, &SSSVoiceDashboard::GetLocalizedVoiceText))
					),
					TPair<FText, TAttribute<FText>>(
						LOCTEXT("PatternExample", "Pattern (Example):"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(
								this, &SSSVoiceDashboard::GetLocalizedVoiceExampleText))
					)
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			BuildKeyValueSection(
				LOCTEXT("LocalizedVoiceSound", "Culture Audio"),
				{
					TPair<FText, TAttribute<FText>>(
						LOCTEXT("Pattern", "Pattern:"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(this, &SSSVoiceDashboard::GetMatchCulturePatternText))
					),
					TPair<FText, TAttribute<FText>>(
						LOCTEXT("PatternExample", "Pattern (Example):"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(
								this, &SSSVoiceDashboard::GetMatchCulturePatternExampleText))
					)
				}
			)
		];
}

void SSSVoiceDashboard::RefreshProfileSection()
{
	ProfileSectionWidget->SetContent(BuildProfileSectionWidget());
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildLeftPanel()
{
	return SNew(SScrollBox)
			// --- Profile Selector
			+ SScrollBox::Slot()
			[
				SAssignNew(ProfileSectionWidget, SBox)
				[
					BuildProfileSectionWidget()
				]
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildRightPanel()
{
	return SNew(SVerticalBox)
			// --- Coverage Overview
			+ SVerticalBox::Slot().AutoHeight().Padding(6)
			[
				SAssignNew(CoverageSectionWidget, SBox)
				[
					BuildCoverageSection()
				]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(SSeparator)
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
					LOCTEXT("CoverageSummaryWithPercent", "{0}%  ({1} / {2})"),
					FText::AsNumber(FMath::RoundToInt(Progress * 100.f)),
					FText::AsNumber(Entry.AssetsWithCulture),
					FText::AsNumber(Entry.TotalAssets)
				))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.MinDesiredWidth(70)
			]

			// Autofill button
			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[
				SNew(SButton)
				.Text(LOCTEXT("AutofillCultureBtn", "Auto-fill"))
				.OnClicked_Lambda([this, Culture = Entry.Culture]()
				{
					// TODO : lancer l’autofill ciblé
					UE_LOG(LogTemp, Log, TEXT("[SSVoice] Requested autofill for culture: %s"), *Culture);

					OpenAutoFillConfirmationDialog(Culture);

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
				.OnClicked(this, &SSSVoiceDashboard::OnGenerateReportClicked)
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

void SSSVoiceDashboard::RefreshCoverageSection()
{
	CoverageSectionWidget->SetContent(BuildCoverageSection());
}

FReply SSSVoiceDashboard::OnGenerateReportClicked()
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

			FSSVoiceLocalizationUI::NotifySuccess(
				NSLOCTEXT("SSVoice", "ScanningCulturesSuccess", "Scanning cultures with success"));

			// Refresh coverage UI
			RefreshCoverageSection();
		}
	);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
