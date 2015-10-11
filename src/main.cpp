/*
 * File:   main.cpp
 * Author: Roman
 * Project: Big Father
 *
 * Created on: 21 дек. 2014 г.
 */

/*
 * Freq of external crystal  is set in clocking.h file via define!!!!!!
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
void TmrLedBlinkCallback(void *p) {
    chSysLockFromIsr();
    if (App.isLedToggle){
        LED1_TOGGLE();
    }
    chVTSetI(&App.TmrLedBlink, MS2ST(UART_RX_POLLING_MS), TmrLedBlinkCallback, nullptr);
    chSysUnlockFromIsr();
}

/*
 * Application entry point.
 */
int main() {

    // ==== Setup clock ====
    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(120);  // Setup Flash Latency for clock in MHz
        // 8 MHz/4 = 2; 2*192 = 384; 384/8 = 64 (preAHB divider); 384/8 = 48 (USB clock)
    // 8 MHz/2 = 4; 4*180 = 720; 720/6 = 90 (preAHB divider); 720/8 = 48 (USB clock)
    Clk.SetupPLLDividers(2, 180, pllSysDiv6, 15);
        // 90/1 = 90 MHz core clock.    (90|45 MHz) APB1|APB2 = AHB/apbDiv APB1 & APB2 clock derive on AHB clock
    Clk.SetupBusDividers(ahbDiv1, apbDiv1, apbDiv2);
    if((ClkResult = Clk.SwitchToPLL()) == 0) Clk.HSIDisable();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();

    // ==== Init hardware ====
    // Leds
    PinSetupOut(LEDS_GPIO, LED1_PIN, omPushPull);
    Uart.Init(115200);
    Uart.Printf("\rBF is started at AHB=%uMHz APB1=%uMHz  APB2=%uMHz\r", Clk.AHBFreqHz/1000000, Clk.APB1FreqHz/1000000, Clk.APB2FreqHz/1000000);
    if(ClkResult != 0) { Uart.Printf("\rXTAL failure\r"); }
    else { LED1_ON(); }

    App.Init(); // Init main thread
    Adc.Init(); // Init ADC thread
    chThdSleepMilliseconds(1000);

    chSysLock();
    chVTSetI(&App.TmrUartRx,    MS2ST(UART_RX_POLLING_MS), TmrUartRxCallback, nullptr);
    chVTSetI(&App.TmrLedBlink,    MS2ST(LED_BLINK_MS), TmrLedBlinkCallback, nullptr);
    chSysUnlock();
    LED1_OFF();
    // Main thread
    while(TRUE){ App.ITask(); }
}



