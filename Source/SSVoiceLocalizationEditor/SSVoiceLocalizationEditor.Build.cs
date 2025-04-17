using UnrealBuildTool;

public class SSVoiceLocalizationEditor : ModuleRules
{
    public SSVoiceLocalizationEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "AssetTools", "Engine", 
                "PropertyEditor", "EditorStyle", "EditorSubsystem",
                "SSVoiceLocalization",
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
                "JsonUtilities", "Json",
            }
        );
    }
}