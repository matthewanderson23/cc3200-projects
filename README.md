# CC3200 Projects

Mini projects for the TI **CC3200 LaunchXL** (SimpleLink Wi-Fi, Cortex-M4), built
with Code Composer Studio. Ordered simplest to most complex, working up from
register-level bare metal to a low-power sensor-to-cloud node.

Each top-level folder is a standalone CCS project. This repo holds the projects
themselves — **not** a CCS workspace. The workspace is disposable, machine-specific
state and is deliberately kept out of version control.

## Quick start (fresh CCS install)

1. Install **CCS 12.8.x**, and the **CC3200 SDK 1.5.0** into `~/ti`.
2. Clone this repo somewhere *outside* your CCS workspace.
3. Launch CCS and create a new workspace, e.g. `~/ccs-workspaces/cc3200`.
4. **File → Import → Code Composer Studio → CCS Projects**, set the search-directory
   to this repo, select the project, and **uncheck "Copy projects into workspace"**.
5. Connect the LaunchXL over USB with the **SOP jumper on position 0** (pins 2–3).
6. **Project → Build Project**, then **Run → Debug**. CCS generates the debug
   configuration on that first run — it is not committed.

## Roadmap

Project folders are named for their position in this roadmap.

| # | Project | Goal |
|---|---|---|
| 01 | `01_register_level_led_blink` | Control hardware without `driverlib` abstraction |
| 02 | `02_interrupt_driven_button_toggle` | Compare polling vs. interrupt-driven design |
| 03 | `03_manual_pwm_fade` | Generate PWM without a library call |
| 04 | `04_i2c_sensor_logger_over_uart` | Read a peripheral over a real bus and report results |
| 05 | `05_rtos_task_blink_button` | Replace the superloop with a real scheduler |
| 06 | `06_wifi_http_client` | Get the board talking to the internet |
| 07 | `07_local_rest_server_for_gpio_control` | Turn the board into a controllable network endpoint |
| 08 | `08_cloud_telemetry_publisher` | Publish live data to a cloud dashboard |
| 09 | `09_ota_firmware_update` | Simulate a real remote-update workflow |
| 10 | `10_end_to_end_low_power_iot_node` | Single-board, power-conscious sensor-to-cloud pipeline |

Three real-world builds layer on top of these:

- **A — Wi-Fi scheduled coffee machine** (`A1`–`A4`), builds on 01, 06, 08
- **B — FPV remote-control rover** (`B1`–`B4`), builds on 01, 03, 06, 07
- **C — Wi-Fi audio interface** (`C1`–`C4`), I2S in/out streamed over the network

## Prerequisites

| Component | Version | Expected location |
|---|---|---|
| Code Composer Studio | 12.8.x | `/Applications/ti/ccs1281` |
| CC3200 SDK | 1.5.0 | `~/ti/CC3200SDK_1.5.0/cc3200-sdk` |
| TI ARM codegen tools | 20.2.7.LTS | installed with CCS |

Projects reference the toolchain and SDK only through CCS path variables
(`CG_TOOL_ROOT`, and `CC3200_SDK_ROOT` where the SDK is used). `CC3200_SDK_ROOT`
resolves via `TI_PRODUCTS_DIR__TIREX`, which comes from the product discovery path
at *Preferences → Code Composer Studio → Products* (`~/ti`). No absolute paths are
committed, so the repo builds on any machine with the same components installed.

Each project carries its own target configuration under `targetConfigs/`, so no
shared, machine-local `.ccxml` is required.

## Keeping projects out of the CCS workspace

The rule of thumb: **if a project folder sits inside your workspace directory, it is
not under version control.** Worth a glance at the workspace folder now and then.

Which setting keeps a project here depends on how it starts:

**New project from scratch** — *File → New → CCS Project*:

1. *Target*: `CC3200`
2. *Connection*: `Stellaris In-Circuit Debug Interface` — generates
   `targetConfigs/CC3200.ccxml`; left blank, no `.ccxml` is created
3. **Uncheck** *Use default location* (checked by default — "default location" means
   inside the current workspace)
4. *Location*: `~/dev/cc3200-projects/<name>`

**Target configuration** — only if the project has no `targetConfigs/`, i.e. step 2
was left blank. *File → New → Target Configuration File*:

1. *File name*: `CC3200_LaunchXL.ccxml`
2. **Uncheck** *Use shared location* (checked, it writes to
   `~/ti/CCSTargetConfigurations/`, outside the repo)
3. *Location*: `/<name>`
4. *Finish*, then in the editor: *Connection* `Stellaris In-Circuit Debug Interface`,
   *Board or Device* `CC3200` — **save** (⌘S; the editor does not autosave)
5. Right-click the project → *New → Folder* → `targetConfigs`, drag the `.ccxml` in

Equivalent shortcut: copy another project's `targetConfigs/*.ccxml` — the file holds
no absolute paths and every project here targets the same board.

**Importing an existing project** — *File → Import → Code Composer Studio → CCS Projects*, then
**uncheck "Copy projects into workspace"**. Unchecked, CCS stores a pointer and edits
the files in place. Checked, it duplicates everything into the workspace and commits
here would capture nothing.

**Importing a TI SDK example** — these come from a `.projectspec` and CCS *always*
copies into the workspace, with no checkbox to stop it. To keep one under git: let it
import, quit CCS, move the folder into this repo, then re-import it by reference with
the copy box unchecked.

Two more conventions that keep projects portable:

- Give each project its own `targetConfigs/*.ccxml` rather than linking an external
  one, so nothing points outside the repo.
- Reference SDK files via `CC3200_SDK_ROOT`, never an absolute path. To set it:
  1. *Project → Properties → Resource → Linked Resources → Path Variables → New…*
     (per project — **not** *Preferences → General → Workspace → Linked Resources*,
     which is not committed)
  2. Name: `CC3200_SDK_ROOT`
  3. Location — type it, do **not** *Browse* (Browse writes an absolute path):
     `${TI_PRODUCTS_DIR__TIREX}/CC3200SDK_1.5.0/cc3200-sdk`
  4. Add to the include path at *Project → Properties → Build → ARM Compiler →
     Include Options*: `${CC3200_SDK_ROOT}/inc`

## What is not here

TI's stock SDK examples (`blinky`, etc.) are not committed — they are TI's code, and
are re-importable at any time from
`~/ti/CC3200SDK_1.5.0/cc3200-sdk/example/<name>/ccs/<name>.projectspec`.
