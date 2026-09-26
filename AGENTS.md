# AGENTS.md

Conventions for agents working in this repo. See `README.md` for human-facing setup
and build instructions.

## What this repo is

CC3200 LaunchXL mini projects, built with Code Composer Studio 12.8.x. Each top-level
folder is a standalone CCS project. This repo is **not** a CCS workspace — workspaces
are disposable, machine-specific, and live elsewhere (`~/ccs-workspaces/`). Never
create one here, and never commit workspace state.

## Naming convention

Project folders are `<index>_<snake_case_title>`, where the index and title come from
the project matrix (see the roadmap table in `README.md`):

- Numbered projects use a zero-padded two-digit index: `01` through `10`.
- Phases of the real-world builds use their letter-number id: `A1`–`A4`, `B1`–`B4`,
  `C1`–`C4`.
- The title is the matrix title, lowercased, with spaces and hyphens collapsed to
  underscores and other punctuation dropped.

```
Register-level LED blink      ->  01_register_level_led_blink
I2C sensor logger over UART   ->  04_i2c_sensor_logger_over_uart
NTP-synced daily scheduler    ->  A2_ntp_synced_daily_scheduler
```

Set the same string as the project name inside CCS, so the folder name, the `<name>`
element in `.project`, and the `<project id="...">` prefix in `.cproject` all match.
When renaming an existing project, update all three and delete the stale `Debug/`
directory — it embeds the old name and CCS regenerates it on the next build.

## Portability rules

The repo must build on any machine with CCS and the CC3200 SDK installed. Absolute
paths break that, so:

- Reference the toolchain and SDK only through CCS path variables — `CG_TOOL_ROOT`
  for the compiler, `CC3200_SDK_ROOT` for the SDK. Never hardcode `/Users/...` or
  `/Applications/ti/...`.
- Give each project its own target configuration at `targetConfigs/*.ccxml`. Do not
  link to a shared one under `~/ti/CCSTargetConfigurations/` — a linked resource with
  an absolute `<location>` will not resolve on another machine.

  The New CCS Project wizard generates one when *Connection* is set; left blank, it
  does not. To add it afterwards, either copy another project's `.ccxml` (they hold
  no absolute paths and every project targets the same board), or:

  1. *File → New → Target Configuration File*, name it `CC3200_LaunchXL.ccxml`
  2. **Uncheck** *Use shared location*; *Location* `/<project>`
  3. In the editor: *Connection* `Stellaris In-Circuit Debug Interface`, *Board or
     Device* `CC3200` — save (the editor does not autosave)
  4. Move it into `targetConfigs/`

Before committing, verify no absolute paths crept into project metadata:

```sh
grep -rn -E '/(Users|Applications)/' --include='.project' --include='.cproject' \
  --include='.ccsproject' --include='*.prefs' .
```

A harmless exception: `<origin>` in `.ccsproject` records where a project template
came from and has no effect on the build.

## What to commit

Commit the project descriptors and sources: `.project`, `.cproject`, `.ccsproject`,
`.settings/`, `targetConfigs/`, `*.c`, `*.h`, and the linker command file `*.cmd`.

Do not commit:

- Build output — `Debug/`, `Release/`, `*.o`, `*.d`, `*.out`, `*.map`. CCS regenerates
  all of it, including the makefiles under `Debug/`.
- CCS workspace state — `.metadata/`, `RemoteSystemsTempFiles/`, `.jxbrowser.userdata/`,
  `AnalysisSolutionsTemp/`.
- Debug launch configurations — `.launches/`. CCS generates one on the first
  Run → Debug, and a new user should rebuild it themselves rather than inherit one.
- TI's stock SDK examples. Only Matthew's own projects belong here; TI examples are
  re-importable from `~/ti/CC3200SDK_1.5.0/cc3200-sdk/example/<name>/ccs/<name>.projectspec`.

`.gitignore` already covers the first two categories.

## Editing project metadata

`.project`, `.cproject`, and `.ccsproject` are XML that CCS owns. When editing them
outside the IDE, validate afterward and have CCS closed — it rewrites these files on
exit and will clobber changes made while it is running:

```sh
xmllint --noout <project>/.project <project>/.cproject
```

## Verification

There is no CI. Changes to project metadata are verified by importing into a CCS
workspace and running Project → Build Project, which requires the IDE and, for a
flash/debug run, the physical LaunchXL. Say what was and was not verified rather than
assuming a metadata edit builds.

## Project README template

Every project folder gets a `README.md` following this skeleton. Keep the headings
identical across projects so the repo reads as one body of work; sections that do not
apply shrink to a line or two rather than being dropped.

```markdown
# 01 — Register-Level LED Blink

One line: what it does, on what hardware.

## Objective
What you were proving. Straight from your matrix:
"control hardware without driverlib abstraction."

## Hardware
Board, MCU, which pin/peripheral, any jumper or wiring setup.

## Build and run
Toolchain and version, how to build, how to flash or debug.
Anything non-obvious about the target configuration.

## How it works
The narrative. Initialisation sequence and *why that order*.

## Register reference
The bit-level detail. One table per register.

## Gotchas
What cost you time and why.

## References
Document numbers and section numbers.
```
