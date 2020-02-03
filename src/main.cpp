
#include "ch.h"
#include "hal.h"
#include "cmsis_os.h"
#include "usbcfg.h"
#include "RF24.h"
#include "RF24HALChibios.h"
#include "RFBL.h"

#ifdef DD
extern const SerialConfig sd1_config;
#endif

void Timer1_Callback  (void const *arg) {
	palToggleLine(LINE_LED1);
};
osTimerDef (Timer1, Timer1_Callback);

uint8_t RF24HAL_Chibios_place[sizeof(RF24HAL_Chibios)];
uint8_t RF24_place[sizeof(RF24)];
uint8_t RFBL_place[sizeof(RFBL)];


int main () {
	/*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
	halInit();
	osKernelInitialize();
	
#ifdef DD
	/*
	* Activates the serial driver 1 using the driver default configuration.
	*/
	sdStart(&SD1, &sd1_config);
#endif

	auto id2 = osTimerCreate (osTimer(Timer1), osTimerPeriodic, nullptr);
	if (id2 != nullptr)  {
		// Periodic timer created
	}
	osTimerStart(id2, 1000);
	  
	/*
	* Initializes a serial-over-USB CDC driver.
	*/
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);
	/*
	* Activates the USB driver and then the USB bus pull-up on D+.
	* Note, a delay is inserted in order to not have to disconnect the cable
	* after a reset.
	*/
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1500);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	auto radio_hal = new(RF24HAL_Chibios_place) RF24HAL_Chibios();
	auto radio = new(RF24_place) RF24(radio_hal);
	auto rf_bl = new(RFBL_place) RFBL(radio);

	//	chnWrite(&SDU1, (const uint8_t*)"Started\n\r", 6);

	while(1) {
		osDelay(1000);
	}
}
