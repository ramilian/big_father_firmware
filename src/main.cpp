/*
 * File:   main.cpp
 * Author: Roman
 * Project: Big Father
 *
 * Created on: 21 дек. 2014 г.
 */

#include "ch.h"
#include "hal.h"
#include "main.h"
#include "evt_mask.h"
#include "kl_lib_f2xx.h"
#include "cmd_uart.h"
#include "application.h"
#include "adc_ext.h"

App_t App;

void TmrUartRxCallback(void *p) {
    chSysLockFromIsr();
    chEvtSignalI(App.PThread, EVTMSK_UART_RX_POLL);
    chVTSetI(&App.TmrUartRx, MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chSysUnlockFromIsr();
}

/*
 * Application entry point.
 */
int main() {

/*
    Clk.UpdateFreqValues();

    // ==== Setup clock ====

    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(12);  // Setup Flash Latency for clock in MHz
    // 8 MHz/4 = 2; 2*192 = 384; 384/8 = 48 (preAHB divider); 384/8 = 48 (USB clock)
    Clk.SetupPLLDividers(4, 192, pllSysDiv8, 8);
    // 48/4 = 12 MHz core clock. APB1 & APB2 clock derive on AHB clock
    Clk.SetupBusDividers(ahbDiv4, apbDiv1, apbDiv1);
    if((ClkResult = Clk.SwitchToPLL()) == 0) Clk.HSIDisable();
*/

    Clk.UpdateFreqValues();

    // ==== Init OS ====
    halInit();
    chSysInit();
    PinSetupOut(LEDS_GPIO, LED1_PIN, omPushPull);

    // ==== Init hardware ====
    Adc.Init();
    Uart.Init(115200);
//    Uart.Printf("***BIG FATHER: version = %s\r", VERSION);

    App.Init();

    chSysLock();
    chVTSetI(&App.TmrUartRx,    MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chSysUnlock();

    // Main thread
    while(TRUE){
        App.ITask();
    }

}



