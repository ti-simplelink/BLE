
/* Conditionally include headers for MSP432 LP. */
#ifdef __MSP432P401R__
#include <gpio.h>
#include <rom_map.h>
#endif //__MSP432P401R__

#include <Energia.h>

#include "BLEBoard.h"

void initBoard(void)
{
  /*
   * When a MSP432 and a CC2650 are stacked, pin 6.7 of the MSP is
   * connected to the reset pin of the CC2650. By default Energia
   * leaves pins unconfigured, so we manually set it to output high.
   * The CC2650's reset pin is active low.
   */
  pinMode(CC2650_RESET_PIN, OUTPUT);
  digitalWrite(CC2650_RESET_PIN, HIGH);

/*
 * Usually NPI's driver calls are enough to open the UART. Energia does pin
 * configuration on its own though, so we have to override that.
 */
#ifdef __MSP432P401R__
  UART_init();
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
                                                 GPIO_PIN2,
                                                 GPIO_PRIMARY_MODULE_FUNCTION);

  MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3,
                                                  GPIO_PIN3,
                                                  GPIO_PRIMARY_MODULE_FUNCTION);
#endif //__MSP432P401R__
}
