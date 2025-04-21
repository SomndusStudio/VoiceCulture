# SS Voice Culture Plugin

**VoiceCulture** is a localization-ready voice-over plugin for Unreal Engine.  
It introduces a custom sound asset (`USSVoiceCultureSound`) that automatically plays the correct voice line based on the game's active language or culture.

It also provides powerful editor tools to track localization coverage, organize voice actors, and auto-populate missing audio using naming strategies.


## ✨ Features

- `USSVoiceCultureSound`: a culture-aware sound asset supporting multiple localized voice lines
- Automatic resolution of the correct voice asset at runtime (with fallback support)
- Full compatibility with native UE audio systems (`AudioComponent`, `AnimNotifies`, `Dialogue`, etc.)
- Custom editor dashboard with:
  - Culture coverage report and progress per language
  - Voice actor grouping via naming conventions
  - Filters for missing/complete cultures
  - Auto-population of missing voice lines
- Strategy system with pluggable profiles (supporting custom naming conventions)
- Blueprint & C++ APIs for full control


## 📦 Installation

### Manual

1. Clone or download this repository.
2. Copy the `VoiceCulture` plugin folder into your project's `Plugins/` directory.
3. Regenerate project files.
4. Build the project in Visual Studio or via Unreal Editor.


## 🧩 Plugin Modules

| Module Name            | Type     |
|------------------------|----------|
| `SSVoiceCulture`       | Runtime  |
| `SSVoiceCultureEditor` | Editor   |


## 🔧 Technical Details

- Written in C++ (UE5+)
- Supports Windows and macOS (Editor + Packaged Builds)
- Does not rely on networking
- Strategy logic can be extended in both Blueprint and C++


## 📚 Documentation

Full documentation available at:  
[https://somndus-studio.com/docs/category/voice-culture](https://somndus-studio.com/docs/category/voice-culture)


## 🧠 Use Case Examples

- Narrative games with multi-language dialogue
- Localized cutscenes or character banter
- Tools pipelines that manage large amounts of voice-over content

## 🏷 License

This plugin is released under the [MIT License](LICENSE) (or whatever license you choose).


## 👤 Author

**Isaac Schartier**  
[https://somndus-studio.com](https://somndus-studio.com)

For support: [support@somndus-studio.com](mailto:support@somndus-studio.com)
