# motor-gba (working title)

A tiny 2D game engine for the Game Boy Advance, with a Dear ImGui editor.

> **Status: early work in progress.** Nothing in this repository is finished
> yet. The layout below is the plan, not a description of what already works.

## Goals

- **Simple and calm.** The engine is small on purpose and has hard limits
  (for example 256 objects and a 240x160 screen), so it does not do the work
  for you.
- **One portable core.** The same C code runs on the GBA and inside the editor.
- **Comfortable editing.** The editor uses Dear ImGui menus.
- **Built from a phone.** Termux for editing, GitHub Actions for compiling.

## Planned layout

```
motor-gba/
├── README.md
├── .gitignore
├── .github/workflows/
│   ├── build-gba.yml        builds the .gba ROM
│   └── build-android.yml    builds the editor APK
├── core/                    the engine (plain C, no SDL, no GBA registers)
│   ├── engine.h             limits and shared types
│   ├── objects.c / .h       object table
│   ├── log.c / .h           in-memory log
│   └── render.c / .h        draws into a 240x160 framebuffer
├── hal/                     platform layer
│   ├── hal.h                what every platform must provide
│   ├── gba/                 crt0.s, gba.ld, hal_gba.c
│   └── sdl/                 hal_sdl.c
├── game/
│   └── game.c               your game logic
├── editor/                  Dear ImGui + SDL2 tool (C++)
│   ├── main.cpp
│   ├── panels.cpp
│   └── imgui/               vendored copy of Dear ImGui
├── assets/                  PNG images and scenes
├── tools/
│   └── png2gba.py           converts PNG files to GBA data
├── android/                 Gradle project that packages the editor
└── build/                   local output (not committed)
```

## How the pieces fit

The core draws every frame into a 240x160 framebuffer (BGR555 pixels, like the
GBA bitmap mode 3). The platform layer in `hal/` shows that framebuffer:
`hal/gba` writes it to video memory, and `hal/sdl` shows it in a window. The
editor wraps the same core and adds Dear ImGui panels (object list, log,
preview), so what you see in the editor is what the console will run.

Dear ImGui lives only in the editor. It does not run on the GBA.

## Building

Planned: GitHub Actions builds both the ROM (`clang` + `lld`) and the Android
APK (Gradle + NDK). Instructions will be added once the workflows exist.

## Code conventions

- Names and comments are written in English.
- `core/`, `hal/` and `game/` are C. `editor/` is C++.
- No floating point in code that runs on the GBA: positions are integers.

## License

To be decided.

---

## Resumen en español

Un motor 2D chiquito para la Game Boy Advance, con un editor hecho con
Dear ImGui. Lo está haciendo un paisano, todo desde el celular: Termux para
escribir el código y GitHub Actions para compilarlo.

Todavía está en pañales: lo que ves arriba es el plan, no lo que ya funciona.
La idea es que sea sencillo, tranquilo y con límites a propósito, para que el
motor no te haga el trabajo.

El código y los comentarios están en inglés para que cualquiera pueda
entender cómo se hizo, pero si eres de los paisanos, aquí también eres
bienvenido.
