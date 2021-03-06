/*
 * application.h
 *
 *  Created on: 21 ���. 2014 �.
 *      Author: Roman
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "cmd_uart.h"
#include "kl_lib_f2xx.h"
#include "evt_mask.h"
#include "adc_ext.h"
#include "adc_f2.h"

#define LED_BLINK_MS  500
#define ADC_MEASURE_MS 1000
class App_t {
private:
    uint16_t ResolutionMask = 0xFFFF;


public:
    Thread *PThread;
    bool isLedToggle;

    void Init();
    void SignalAdcRsltReady() { chEvtSignalI(PThread, EVTMSK_ADC_READY); }
    // Leds
    void LedBlink(uint32_t Duration_ms);
    // Events
    void OnUartCmd(Cmd_t *PCmd);
    // Inner use
    void ITask();
    void IIrqHandler();
    App_t(): ResolutionMask(0xFFFF), PThread(nullptr), isLedToggle(false) {}
    VirtualTimer TmrUartRx;
    VirtualTimer TmrLedBlink;
    VirtualTimer TmrAdcMeasure;


};
extern App_t App;


#endif /* APPLICATION_H_ */
