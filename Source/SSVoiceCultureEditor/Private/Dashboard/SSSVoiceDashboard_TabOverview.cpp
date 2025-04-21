/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "Dashboard/SSSVoiceDashboard.h"

#include "EditorStyleSet.h"
#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureEditorSubsystem.h"
#include "SSVoiceStyleCompat.h"
#include "Slate/SSVoiceCultureSlateComponents.h"
#include "Utils/SSVoiceCultureUI.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Utils/SSVoiceCultureUtils.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

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
			.TextStyle(SSVoiceStyleCompat::Get(), "HintText")
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
			.Style(SSVoiceStyleCompat::Get(), "DetailsView.Splitter")
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
			.Text(NSLOCTEXT("SSVoiceCultureEditor", "Profile", "Profile"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SAssignNew(ProfileSelector, SSSVoiceEditorProfileSelector)
			.OnProfileChanged(FSimpleDelegate::CreateLambda([this]()
			{
				auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
				VLEditorSubsystem->
					ChangeActiveProfileFromName(ProfileSelector->GetSelectedProfile().Get()->ProfileName);
			}))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			BuildKeyValueSection(
				NSLOCTEXT("SSVoiceCultureEditor", "VoiceCultureAsset", "Voice Culture Audio"),
				{
					TPair<FText, TAttribute<FText>>(
						NSLOCTEXT("SSVoiceCultureEditor", "Pattern", "Pattern:"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(this, &SSSVoiceDashboard::GetVoiceCultureText))
					),
					TPair<FText, TAttribute<FText>>(
						NSLOCTEXT("SSVoiceCultureEditor", "PatternExample", "Pattern (Example):"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(
								this, &SSSVoiceDashboard::GetVoiceCultureExampleText))
					)
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			BuildKeyValueSection(
				NSLOCTEXT("SSVoiceCultureEditor", "CultureSoundBase", "Culture Sound Base"),
				{
					TPair<FText, TAttribute<FText>>(
						NSLOCTEXT("SSVoiceCultureEditor", "Pattern", "Pattern:"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(this, &SSSVoiceDashboard::GetMatchCulturePatternText))
					),
					TPair<FText, TAttribute<FText>>(
						NSLOCTEXT("SSVoiceCultureEditor", "PatternExample", "Pattern (Example):"),
						TAttribute<FText>::Create(
							TAttribute<FText>::FGetter::CreateSP(
								this, &SSSVoiceDashboard::GetMatchCulturePatternExampleText))
					)
				}
			)
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildLeftPanel()
{
	return SNew(SScrollBox)
			// --- Profile Selector
			+ SScrollBox::Slot()
			[
				BuildProfileSectionWidget()
			];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildRightPanel()
{
	return SNew(SVerticalBox)
			// --- Coverage Overview
			+ SVerticalBox::Slot().AutoHeight().Padding(6)
			[
				BuildCoverageSection()
			];
}

TSharedRef<ITableRow> SSSVoiceDashboard::OnGenerateCultureRow(TSharedPtr<FSSVoiceCultureReportEntry> Entry,
                                                              const TSharedRef<STableViewBase>& OwnerTable)
{
	const float Progress = Entry->GetCoveragePercent();

	const FString CultureLabel = FSSVoiceCultureUI::CultureAsDisplay(Entry->Culture);

	return SNew(STableRow<TSharedPtr<FSSVoiceCultureReportEntry>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			// Culture name
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4)
			[
				SNew(SBox)
				.WidthOverride(150.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(CultureLabel))
					.MinDesiredWidth(60)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
			]
			// Progress bar with overlayed percentage text
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(4)
			[
				SNew(SOverlay)

				// Background: Progress bar
				+ SOverlay::Slot()
				[
					SNew(SProgressBar)
					.Percent(Progress)
				]

				// Foreground: Centered text on top
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::Format(
						NSLOCTEXT("SSVoiceCultureEditor", "CoverageSummaryWithPercent", "{0}%  ({1} / {2})"),
						FText::AsNumber(FMath::RoundToInt(Progress * 100.f)),
						FText::AsNumber(Entry->AssetsWithCulture),
						FText::AsNumber(Entry->TotalAssets)
					))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor::UseForeground()) // always visible text
				]
			]

			// AutoPopulate button
			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateCultureBtn", "Auto populate"))
				.OnClicked_Lambda([this, Culture = Entry->Culture]()
				{
					UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoiceCulture] Requested AutoPopulate for culture: %s"), *Culture);
					OpenAutoPopulateConfirmationDialog(Culture);
					return FReply::Handled();
				})
			]
		];
}

TSharedRef<SWidget> SSSVoiceDashboard::BuildCultureListWidget()
{
	return SAssignNew(CultureListView, SListView<TSharedPtr<FSSVoiceCultureReportEntry>>)
		.ItemHeight(30)
		.ListItemsSource(&CultureListData)
		.SelectionMode(ESelectionMode::None)
		.OnGenerateRow(this, &SSSVoiceDashboard::OnGenerateCultureRow);
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
				.Text(NSLOCTEXT("SSVoiceCultureEditor", "CultureCoverageTitle", "Voice Culture Coverage"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(SButton)
				.Text(NSLOCTEXT("SSVoiceCultureEditor", "RegenerateReportBtn", "Generate Report"))
				.ToolTipText(NSLOCTEXT("SSVoiceCultureEditor", "RegenerateReportTooltip",
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
	CultureListData.Reset();

	for (const FSSVoiceCultureReportEntry& Entry : CultureReport.Entries)
	{
		CultureListData.Add(MakeShared<FSSVoiceCultureReportEntry>(Entry));
	}

	if (CultureListView.IsValid())
	{
		CultureListView->RequestListRefresh();
	}
}

FReply SSSVoiceDashboard::OnGenerateReportClicked()
{
	// Create a slow task to inform the user (runs on Game Thread)
	TSharedPtr<FScopedSlowTask> SlowTask = MakeShared<FScopedSlowTask>(
		1.f,
		NSLOCTEXT("SSVoiceCultureEditor", "ScanningCultures", "Scanning voice cultures...")
	);

	SlowTask->MakeDialog(true);

	// Create a shared report object to pass between threads
	TSharedRef<FSSVoiceCultureReport, ESPMode::ThreadSafe> Report = MakeShared<
		FSSVoiceCultureReport, ESPMode::ThreadSafe>();

	// Launch the heavy operation asynchronously
	AsyncTask(ENamedThreads::GameThread, [this, Report, SlowTask]()
	{
		// Run the time-consuming report generation
		FSSVoiceCultureUtils::GenerateCultureCoverageReport(*Report);

		// Once complete, schedule the UI update on the Game Thread
		FFunctionGraphTask::CreateAndDispatchWhenReady([this, Report, SlowTask]()
		{
			// Update internal state with the generated report
			CultureReport = *Report;

			// Mark progress complete if needed
			if (SlowTask.IsValid())
			{
				SlowTask->EnterProgressFrame(1.f);
			}

			// Notify the user of success
			FSSVoiceCultureUI::NotifySuccess(
				NSLOCTEXT("SSVoiceCultureEditor", "ScanningCulturesSuccess", "Successfully scanned voice cultures."));

			// Refresh UI with new report data
			RefreshCoverageSection();
		}, TStatId(), nullptr, ENamedThreads::GameThread);
	});

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
