# 01 — Register-Level LED Blink

Blinks the red LED at a constant rate forever.

## Objective
Control hardware without driverlib abstraction.

## Hardware
LaunchpadXL has GPIO_09 connected to the red LED. No further wiring setup.

## Build and run
Toolchain and version, how to build, how to flash or debug.
Anything non-obvious about the target configuration.

| Component | Version |
| --- | --- |
| IDE |  Code Composer Studio 12.8.1.00005 |
| Compiler |  TI ARM C/C++ Compiler v20.2.7.LTS (armcl) |
| Debug Server | 12.8.0.3471 |
| SDK | unused |

Click the Build icon in Code Composer Studio at the top to compile, and then the Debug icon to flash and run the debugger.

Had to reverse-engineer which debug config to use. In the end it was the Stellaris debug interface which Claude was able to figure out when we plugged it in via USB.

## How it works
The narrative. Initialisation sequence and *why that order*.

The classic LED blink program uses a superloop. Initialization first, then loop forever. The loop is straightforward: toggle the LED pin high, busy-wait, toggle low, busy-wait, repeat. The initialization was less straightforward for someone who hadn't done embedded from scratch in a while.

Claude did the heavy lifting to reverse engineer the `blinky` example from TI. Their example uses their `driverlib` abstraction layer, which went unused in this mini-project in order to practice the fundamentals. Claude prompting pointed to specific sections in each of the reference documents to uncover each piece of the puzzle.

Before coding anything, however, I needed to figure out what the red LED was connected to. The documentation for the LaunchpadXL showed it was connected to GPIO_09. Then I had to trace that back to find the GPIO port and pin that that particular GPIO pin is connected to in hardware: Port A1, pin 1.

Then, in the initialization steps, a few system-level inits had to be done first: clocks, and GPIO init and configuration.

The CC3200 has several sleep modes, so in normal "run" mode, the clocks have to be enabled properly. It's the only thing that keeps track of system time when the program is running, and to blink the LED at a steady rate.

Then, I had to make sure that the physical chip pad was getting muxed to the correct peripheral. This was done with the `GPIO_PAD_CONFIG_n` registers. This is also where GPIO drive strength is set. For the CC3200 it was fine to use the lowest current value (2 mA).

After that, I set the initial state of the pin to 0 (see Gotcha #3 below) before setting the pin direction to output. This is key: because the pin starts driving the instant its direction is set, whatever is already in GPIODATA is what appears on the pin. That's the last initialization step.

In the superloop, all that is needed is the same GPIO write expression used when setting the initial state, and a busy-wait. The busy-wait was copied from TI's `blinky` example to, again, avoid the use of `driverlib`. The reference manual says that after setting the clocks there has to be a 3-cycle clock tick before proceeding. This busy-wait method (using the `__asm` keyword) was reused for the while-forever loop's busy-wait in between LED toggles.

## Register reference
The bit-level detail. One table per register.

## Gotchas
What cost you time and why.

1. Understanding how to convert a register `#define` into an address.

It's the basics, but worth mentioning that pointers are extremely important! You can have a bunch of `#define` macros defining registers, but until you cast them to something understandable by the compiler, they are just hex values. The fix is making the values into something you can dereference, so that you can write a value at the location pointed to by the pointer. The following macro allows you to directly read, modify, and write bits in the register.

```c
#define HWREG(x) (*((volatile unsigned long *)(x)))
```

2. Read-modify-write to preserve state of other bits in a register.

```c
// set bit 3 of REGISTER_1, don't touch anything else
HWREG(REGISTER_1) |= (1 << 3);

// clear bit 5 of REGISTER_1, don't touch anything else
HWREG(REGISTER_1) &= ~(1 << 5)
```

3. `GPIODATA` register allows modification to the GPIO pins without RMW, saving CPU cycles.

`GPIODATA` is a special register (offset from the port base address) in that setting and clearing multiple GPIO pins can be done in one instruction. Section 5.2.1.2 of the Technical Reference Manual explains it, but in essence, the idea is to compute the register as usual (port base address + `GPIODATA` reg offset), but then you need to offset also the pin mask of the pins you want to modify, and shift them left by 2 bits.

```c
#define GPIODATA_REG_MASK(port_base, pin_mask) \
    HWREG((port_base) + ((GPIO_DATA_O_REG + ((pin_mask) << 2))))
```

That's what allows you to do multiple writes in one instruction.

```c
void GPIOPinWrite(
    unsigned long ulPort,
    unsigned char ucPinMask,
    unsigned long ulVal
) {
    GPIODATA_REG_MASK(ulPort, ucPinMask) = ulVal;
}
// create a mask where bits 1 and 3 are 1, everything else 0
uint8_t pin_mask = (1 << 3) | (1 << 1);
// clear both pins
GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, pin_mask, 0);
// set both pins
GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, pin_mask, pin_mask);
// set pin 3, and clears pin 1
GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, pin_mask, (1 << 3));
```

4. Pin muxing in order to set the physical chip pad to the correct peripheral inside the Cortex-M4.

## References

### SWRU367D — CC3200 Technical Reference Manual

| Section | Used for |
|---|---|
| §1 | Cortex-M4 core runs at 80 MHz — basis for the delay calibration |
| §5.2.1.2 Data Register Operation | GPIODATA masked addressing; address bits [9:2] as a write mask |
| §5.5 GPIO_REGISTER_MAP Registers | GPIO port base addresses; clock-must-be-enabled and 3-cycle settling note |
| §5.5.1 GPIO Register Description | Table 5-3, register offsets |
| §5.5.1.1 GPIODATA | data register bit definitions |
| §5.5.1.2 GPIODIR | direction register; 1 = output |
| Table 15-2 Peripheral Macro Table | mapping peripherals to RCM registers |
| Table 15-3 PRCM Registers | ARCM register offsets |
| §15.3.2 Application Reset-Clock Manager | which block owns GPIO clock gating |
| §15.4.5 Clock Control | clock gating concept |
| §15.7.1 PRCM Register Description | register-level detail |
| Table 15-19 GPIO1CLKEN Field Descriptions | RUN / SLEEP / DEEPSLEEP enable bits |
| Table 16-6 GPIO/Pins Available for Application | GPIO number to device pin |
| Table 16-7 Pin Multiplexing | pad mode values per pin |
| §16.8.1 Pad Configuration Registers for Application Pins | pad config register addresses and fields |
| §16.8.4 CC3200 Pin-mux Examples | worked pin-mux examples |

### SWRU372C — CC3200 LaunchXL User's Guide

| Section | Used for |
|---|---|
| Jumpers, Switches, and LEDs (p.8) | board overview |
| Table 9. LEDs | D7 = red = GPIO_09, glows when logic-1 |

### SPNU151 — TI ARM Optimizing C/C++ Compiler User's Guide

| Section | Used for |
|---|---|
| §3.10 Use Caution With asm Statements in Optimized Code | why the delay is written in assembly |
| §5.10 The `__asm` Statement | inline assembly syntax |
| §5.14 ARM Instruction Intrinsics | Table 5-3, intrinsic availability |

### CC3200 SDK 1.5.0 — read as reference only

| File | Used for |
|---|---|
| `inc/hw_types.h` | `HWREG` accessor pattern |
| `inc/hw_memmap.h` | `ARCM_BASE`, `OCP_SHARED_BASE` |
| `inc/hw_apps_rcm.h` | RCM register offsets, cross-check |
| `inc/hw_ocp_shared.h` | pad config register full 12-bit field layout |
| `driverlib/prcm.c` | `PRCM_PeriphRegsList` — confirms port A1 maps to offset 0x58 |
| `driverlib/pin.c` | `g_ulPinToPadMap` — confirms pin 64 maps to pad 9 |
| `driverlib/utils.c` | `UtilsDelay` — assembly template and 3 cycles/loop |