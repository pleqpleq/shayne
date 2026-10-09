# Supleq Smart Clicks — Geometry Dash Geode Mod

This ZIP is the **full source code and all 109 processed WAV samples**, not an installable `.geode` binary yet. This environment does not contain a Windows Geode compiler / Geode SDK, so the binary cannot be built or verified here.

## What it does

- **Up Arrow**: 16 different press samples and 16 different release samples from 0–8.6 sec in your new recording.
- **Up Arrow spam**: 24 different press and 23 release samples from the fast tapping after 8.6 sec. Switches automatically when a rolling 4–5-tap window measures **greater than 8 CPS**, without waiting a full second.
- **Spacebar**: 15 press and 15 release samples from `space.mp3`, separate from Up Arrow.
- Each sound is selected at random, never the previous sample twice in a row. Each release uses the same speed category as its press.
- Plays in levels, not menus, and does not change GD input behavior. Ignores key auto-repeat.
- Sounds are cropped, high-pass filtered, and given fades to reduce microphone rumble and pops. They are bundled as PCM WAV to minimize playback latency.

## Build: easier option — GitHub Actions (no compiler needed on your PC)

1. Extract this ZIP to a folder. The folder contains `mod.json`, `CMakeLists.txt`, `src`, `resources`, and a hidden-ish `.github` folder.
2. Make a new empty GitHub repository; upload **the contents of this folder** to the repository root, including `.github/workflows/build.yml`. Avoid uploading an extra surrounding directory.
3. Commit to the `main` branch. In the repository open **Actions > Build Supleq Smart Clicks (Windows) > Run workflow** if it has not started automatically. GitHub Actions may require enabling for a new repository.
4. Wait for the build job to finish successfully; open its **Artifacts** section and download `Supleq-Smart-Clicks-Windows`.
5. Extract that artifact ZIP to get `supleq.smart-clicks.geode` (the name may include the version).
6. Install Geode, then copy the **.geode file**, not this source ZIP, to your GD installation's `geode/mods` folder. Find the GD folder with Steam > Geometry Dash > Manage > Browse Local Files. Restart Geometry Dash.
7. Open **Geode > Installed Mods > Supleq Smart Clicks > Settings**. Set volume and threshold as you like. Turn off other click-sound mods if they double-play.

Latest Geode installer: https://geode-sdk.org/install
Geode mod SDK and guide: https://docs.geode-sdk.org/getting-started/create-mod/

## Build: on Windows with Geode SDK already installed

From the extracted project directory, after setting up the official Geode SDK and CLI:

```powershell
geode build
```

The generated `.geode` file will be in the build output, or automatically installed if your Geode CLI is configured with a game profile.

## Troubleshooting

- No sounds: check mod is enabled, open a level (the mod does not play in menus), check volume sliders, and check for audio load errors in Geode's log.
- Can't install ZIP: only the compiled **`.geode`** file is an installable mod; the provided ZIP is its source.
- Double click sounds: disable another click-sound mod (e.g. Click Sounds Full) or its click sounds.
- Fast samples never play: spam Up Arrow more than 8 CPS for at least 4 distinct presses; keyboard auto-repeat is deliberately ignored.
- Spacebar sounds never play: use the physical Space key, not mouse or a rebound non-space key.
- Mod won't load: it targets Windows Geometry Dash **2.2081** and Geode **5.10.1**. Update GD/Geode or rebuild against a supported target.

This implementation has not been run inside GD; source and algorithms are provided for compilation and testing on Windows.
