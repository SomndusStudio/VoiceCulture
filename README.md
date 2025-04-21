# SSVoiceCulture Plugin

**SSVoiceCulture** is a plugin for Unreal Engine 4.27 that enables advanced handling of localized voice audio, independently from Unreal's native text localization system.

It introduces a new sound asset (`USSVoiceCultureSound `) that inherits from `USoundBase`, allowing integration into any Unreal audio system while supporting per-language audio resolution.

---

## Features

- New asset type: `Voice Culture Sound ` (inherits from `USoundBase`)
- Supports multiple audio versions per language (based on culture codes)
- Runtime voice language selection, independent from game UI/text language
- In-editor playback with thumbnail and toolbar buttons (Play / Stop)
- Custom asset editor with details panel and audio preview
- Engine subsystem to manage current voice language at runtime
- Fully compatible with `AudioComponent`, `AnimNotifies`, `Montages`, etc.

---

## How It Works

1. Create a `Voice Culture Sound` asset from the Content Browser
2. Add one or more localized versions using culture codes and `SoundBase` references
3. Use the asset like any other `SoundBase` (play in code, Blueprint, etc.)
4. The correct audio is resolved at runtime based on the voice language

---

## Code Example

```cpp
// Play a localized voice from code
AudioComponent->SetSound(VoiceCultureSound);
AudioComponent->Play();

// Change the current voice language at runtime
auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceCultureSubsystem>();
Subsystem->SetCurrentVoiceCulture("fr"); // Switch to French

```

## License

MIT License © SCHARTIER Isaac
