# Quick

A Flipper Zero external application for fast access to all saved NFC cards and IR remotes in one place, regardless of their source.

## Features

- **NFC** — browse and emulate saved `.nfc` cards directly without opening the native NFC app
- **IR** — browse saved `.ir` remotes, view all signals, and hold OK to transmit continuously
- **Favourites** — persist selected files to SD card; survive restarts
- **Tick marks** — `[+]` / `[ ]` in the file browser to toggle favourites in place
- **Sorted lists** — all entries sorted alphabetically

## Requirements

- Flipper Zero running [Momentum firmware](https://github.com/Next-Flip/Momentum-Firmware)
- `ufbt` installed

## Build & Deploy

```bash
cd Quick
ufbt          # build only
ufbt launch   # build and deploy over USB
```



## File Locations (on SD card)

| Purpose | Path |
|---|---|
| NFC favourites list | `/ext/apps_data/quick/nfc_favourites.txt` |
| IR favourites list | `/ext/apps_data/quick/ir_favourites.txt` |
