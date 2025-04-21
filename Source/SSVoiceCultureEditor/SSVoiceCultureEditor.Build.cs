// Copyright (C) 2020-2025 Schartier Isaac
// Official Documentation: https://www.somndus-studio.com

using UnrealBuildTool;

public class SSVoiceCultureEditor : ModuleRules
{
    public SSVoiceCultureEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "AssetTools", "Engine", 
                "PropertyEditor", "EditorStyle", "EditorSubsystem",
                "SSVoiceCulture",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                
                "DeveloperSettings",
                "UnrealEd",
                "EditorStyle",
                "GraphEditor",
                "Projects",
                "AssetTools",
                "ToolMenus",
                "InputCore", 
                "JsonUtilities", "Json", "WorkspaceMenuStructure",
                "ContentBrowser",
                "ContentBrowserData",
            }
        );
    }
}