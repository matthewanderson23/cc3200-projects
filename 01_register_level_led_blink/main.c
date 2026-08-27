

/**
 * main.c
 */


// =============================================================================
// SILICON FACTS - CC3200 device (SWRU367 Technical Reference Manual)
// =============================================================================

// -----------------------------------------------------------------------------
// APPS_RCM - clock and reset management (TRM ch. 15)
// -----------------------------------------------------------------------------
// APPS_RCM address
#define ARCM_BASE 0x44025000

// defines for the APPS_RCM register offsets
#define APPS_RCM_GPIO0CLKEN (0x50)
#define APPS_RCM_GPIO1CLKEN (0x58)
#define APPS_RCM_GPIO2CLKEN (0x60)
#define APPS_RCM_GPIO3CLKEN (0x68)

#define RUNCLKEN_BIT  (0)
#define SLPCLKEN_BIT  (8)
#define DSLPCLKEN_BIT (16)


// -----------------------------------------------------------------------------
// OCP_SHARED - I/O pad configuration (TRM ch. 16)
// -----------------------------------------------------------------------------

// defines for GPIO pins
#define GPIO_PAD_CONFIG_9  (0x4402E0C4)
#define GPIO_PAD_CONFIG_10 (0x4402E0C8)
#define GPIO_PAD_CONFIG_11 (0x4402E0CC)

// pad config value: GPIO output, push-pull, 2 mA drive
// - pin mux set to 0x0 (3:0 set to 0x0)
// - bit 4 disable open-drain mode
// - 7:5 drive strength set to:
//    111 = 14 mA
//    110 = 12 mA
//    101 = 10 mA
//    100 = 8 mA
//    011 = 6 mA
//    010 = 4 mA
//    001 = 2 mA
//    000 = Output driver not enabled
#define PAD_CONFIG_GPIO_OUTPUT_2MA  ( \
            (0 << 11) |\
            (0 << 10) |\
            (0 << 9) |\
            (0 << 8) |\
            (0x1 << 5) |\
            (0 << 4) |\
            (0x0 << 0) )


// -----------------------------------------------------------------------------
// GPIO ports (TRM ch. 5)
// -----------------------------------------------------------------------------
// defines for GPIO ports
#define GPIO_PORT_A0_ADDR_BASE 0x40004000
#define GPIO_PORT_A1_ADDR_BASE 0x40005000
#define GPIO_PORT_A2_ADDR_BASE 0x40006000
#define GPIO_PORT_A3_ADDR_BASE 0x40007000
// defines for GPIO I/O mode
#define GPIO_DATA_O_REG   0x000
#define GPIO_DIR_O_REG    0x400

#define GPIO_DIR_MODE_IN  (0x0)
#define GPIO_DIR_MODE_OUT (0x1)

#define GPIO_PIN_0_MASK (1 << 0)
#define GPIO_PIN_1_MASK (1 << 1)
#define GPIO_PIN_2_MASK (1 << 2)
#define GPIO_PIN_3_MASK (1 << 3)
#define GPIO_PIN_4_MASK (1 << 4)
#define GPIO_PIN_5_MASK (1 << 5)
#define GPIO_PIN_6_MASK (1 << 6)
#define GPIO_PIN_7_MASK (1 << 7)

#define GPIO_09 GPIO_PIN_1_MASK


// =============================================================================
// BOARD FACTS - CC3200 LaunchXL (SWRU372 User's Guide, Table 9)
// =============================================================================

#define LED_RED_PIN     GPIO_09
//#define LED_YELLOW  GPIO_10
//#define LED_GREEN   GPIO_11


// macro for hardware access
#define HWREG(x) (*((volatile unsigned long *)(x)))
#define GPIODATA_REG_MASK(port_base, pin_mask) \
    HWREG((port_base) + ((GPIO_DATA_O_REG + ((pin_mask) << 2))))

void GPIOPinWrite(unsigned long ulPort, unsigned char ucPinMask, unsigned long ulVal) {
    GPIODATA_REG_MASK(ulPort, ucPinMask) = ulVal;
}


// =============================================================================
// "Timer"
// =============================================================================
#define WAIT(count) \
    do {\
        volatile unsigned long _i = 0;\
        for (_i = 0; _i < (count); _i++) {}\
    } while(0)


// copied from TI's driverlib
// simple busy-wait that takes exactly 3 cycles
extern void CycleDelay3x(unsigned long ulCount);

__asm("    .sect \".text:CycleDelay3x\"\n"
      "    .clink\n"
      "    .thumbfunc CycleDelay3x\n"
      "    .thumb\n"
      "    .global CycleDelay3x\n"
      "CycleDelay3x:\n"
      "    subs r0, #1\n"
      "    bne.n CycleDelay3x\n"
      "    bx lr\n");




int main(void)
{
    // system init
    // ungate the clock for GPIO port A1
    HWREG(ARCM_BASE + APPS_RCM_GPIO1CLKEN) |= (1 << RUNCLKEN_BIT);

    // wait 3 sys clock cycles
    CycleDelay3x(1);

    // configure the pad for PAD9
    HWREG(GPIO_PAD_CONFIG_9) = PAD_CONFIG_GPIO_OUTPUT_2MA;

    // set the initial output level, writing GPIODATA
    GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, LED_RED_PIN, 0);

    // set the GPIO direction to output
    HWREG(GPIO_PORT_A1_ADDR_BASE + GPIO_DIR_O_REG) |= LED_RED_PIN;

    // while forever loop that turns on and off the red LED
    while (1) {
        // turn on red LED
        GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, LED_RED_PIN, LED_RED_PIN);
        // wait some amount of time
        CycleDelay3x(8000000);
        // turn off red LED
        GPIOPinWrite(GPIO_PORT_A1_ADDR_BASE, LED_RED_PIN, 0);
        // wait some amount of time
        CycleDelay3x(8000000);

    }
	return 0;
}
