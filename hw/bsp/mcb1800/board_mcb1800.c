/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2018, hathach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "chip.h"
#include "../board.h"

#define LED_PORT  6
#define LED_PIN   24

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+
/* System configuration variables used by chip driver */
const uint32_t ExtRateIn = 0;
const uint32_t OscRateIn = 12000000;

static const PINMUX_GRP_T pinmuxing[] =
{
	/* Board LEDs */
	{0xD, 10, (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0xD, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0xD, 12, (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0xD, 13, (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0xD, 14, (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0x9, 0,  (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC0)},
	{0x9, 1,  (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC0)},
	{0x9, 2,  (SCU_MODE_INBUFF_EN | SCU_MODE_INACT | SCU_MODE_FUNC0)},

	/*  I2S  */
	{0x3, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 0,  (SCU_PINIO_FAST | SCU_MODE_FUNC4)},
	{0x7, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 2,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},
	{0x7, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC2)},
	{0x6, 1,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},
};

/* Pin clock mux values, re-used structure, value in first index is meaningless */
static const PINMUX_GRP_T pinclockmuxing[] =
{
	{0, 0,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0)},
	{0, 1,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0)},
	{0, 2,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0)},
	{0, 3,  (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_FUNC0)},
};


/*------------------------------------------------------------------*/
/* BOARD API
 *------------------------------------------------------------------*/
// Invoked by startup code
void SystemInit(void)
{
  /* Setup system level pin muxing */
  Chip_SCU_SetPinMuxing(pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

  /* Clock pins only, group field not used */
  for (uint32_t i = 0; i < (sizeof(pinclockmuxing) / sizeof(pinclockmuxing[0])); i++)
  {
    Chip_SCU_ClockPinMuxSet(pinclockmuxing[i].pinnum, pinclockmuxing[i].modefunc);
  }

  Chip_SetupXtalClocking();
}

void board_init(void)
{
  SystemCoreClockUpdate();

#if CFG_TUSB_OS == OPT_OS_NONE
  // 1ms tick timer
  SysTick_Config(SystemCoreClock / 1000);
#endif

  Chip_GPIO_Init(LPC_GPIO_PORT);

  //------------- LED -------------//
  /* Port and bit mapping for LEDs on GPIOs */
  const uint8_t ledports[] = {6, 6, 6, 6, 6, 4, 4, 4};
  const uint8_t ledbits[] = {24, 25, 26, 27, 28, 12, 13, 14};

  for (uint32_t i = 0; i < (sizeof(ledports) / sizeof(ledports[0])); i++) 
  {
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, ledports[i], ledbits[i]);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_PORT, LED_PIN, false);
  }

#if 0
  //------------- BUTTON -------------//
  for(uint8_t i=0; i<BOARD_BUTTON_COUNT; i++)
  {
    scu_pinmux(buttons[i].mux_port, buttons[i].mux_pin, GPIO_NOPULL, FUNC0);
    GPIO_SetDir(buttons[i].gpio_port, TU_BIT(buttons[i].gpio_pin), 0);
  }

  //------------- UART -------------//
  scu_pinmux(BOARD_UART_PIN_PORT, BOARD_UART_PIN_TX, MD_PDN, FUNC1);
  scu_pinmux(BOARD_UART_PIN_PORT, BOARD_UART_PIN_RX, MD_PLN | MD_EZI | MD_ZI, FUNC1);

  UART_CFG_Type UARTConfigStruct;
  UART_ConfigStructInit(&UARTConfigStruct);
  UARTConfigStruct.Baud_rate   = CFG_UART_BAUDRATE;
  UARTConfigStruct.Clock_Speed = 0;

  UART_Init(BOARD_UART_PORT, &UARTConfigStruct);
  UART_TxCmd(BOARD_UART_PORT, ENABLE); // Enable UART Transmit
#endif

  //------------- USB -------------//
  enum {
    USBMODE_DEVICE = 2,
    USBMODE_HOST   = 3
  };

  enum {
    USBMODE_VBUS_LOW  = 0,
    USBMODE_VBUS_HIGH = 1
  };

  // USB0
#if CFG_TUSB_RHPORT0_MODE
  Chip_USB0_Init();

  // Reset controller
  LPC_USB0->USBCMD_D |= 0x02;
  while( LPC_USB0->USBCMD_D & 0x02 ) {}

  // Set mode
  #if CFG_TUSB_RHPORT0_MODE & OPT_MODE_HOST
    LPC_USB0->USBMODE_H = USBMODE_HOST | (USBMODE_VBUS_HIGH << 5);
  #else // TODO OTG
    LPC_USB0->USBMODE_D = USBMODE_DEVICE;
    LPC_USB0->OTGSC = (1<<3) | (1<<0) /*| (1<<16)| (1<<24)| (1<<25)| (1<<26)| (1<<27)| (1<<28)| (1<<29)| (1<<30)*/;
  #endif
#endif

  // USB1
#if CFG_TUSB_RHPORT1_MODE
  Chip_USB1_Init();

  // Reset controller
  LPC_USB1->USBCMD_D |= 0x02;
  while( LPC_USB1->USBCMD_D & 0x02 ) {}

  // Set mode
  #if CFG_TUSB_RHPORT1_MODE & OPT_MODE_HOST
    LPC_USB1->USBMODE_H = USBMODE_HOST | (USBMODE_VBUS_HIGH << 5);
  #else // TODO OTG
    LPC_USB1->USBMODE_D = USBMODE_DEVICE;
  #endif

  // USB1 as fullspeed
  LPC_USB1->PORTSC1_D |= (1<<24);
#endif
}

//------------- LED -------------//
void board_led_write(bool state)
{
  Chip_GPIO_SetPinState(LPC_GPIO_PORT, LED_PORT, LED_PIN, state);
}

//------------- Buttons -------------//
/*
static bool button_read(uint8_t id)
{
  (void) id;
//  return !TU_BIT_TEST( GPIO_ReadValue(buttons[id].gpio_port), buttons[id].gpio_pin ); // button is active low
  return false;
}
*/

uint32_t board_button_read(void)
{
  uint32_t result = 0;

//  for(uint8_t i=0; i<BOARD_BUTTON_COUNT; i++) result |= (button_read(i) ? TU_BIT(i) : 0);

  return result;
}


//------------- UART -------------//
int board_uart_read(uint8_t* buf, int len)
{
  //return UART_ReceiveByte(BOARD_UART_PORT);
  (void) buf;
  (void) len;
  return 0;
}

int board_uart_write(void const * buf, int len)
{
  //UART_Send(BOARD_UART_PORT, &c, 1, BLOCKING);
  (void) buf;
  (void) len;
  return 0;
}

/*------------------------------------------------------------------*/
/* TUSB HAL MILLISECOND
 *------------------------------------------------------------------*/
#if CFG_TUSB_OS == OPT_OS_NONE

volatile uint32_t system_ticks = 0;

void SysTick_Handler (void)
{
  system_ticks++;
}

uint32_t board_millis(void)
{
  return system_ticks;
}

#endif
