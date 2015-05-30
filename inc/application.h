/*
 * application.h
 *
 *  Created on: 21 дек. 2014 г.
 *      Author: Roman
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "cmd_uart.h"
#include "kl_lib_f2xx.h"
#include "evt_mask.h"
#include "adc_ext.h"

#define FSR_MIN_1LSB  4.999847      // 1 LSB - 152.6 uV

class App_t {
private:
//    bool tl;
    Timer_t SamplingTmr;
    uint16_t ResolutionMask = 0xFFFF;
    uint16_t SignMask = 0x8000;
    void CskHi() { PinSet(ADC_GPIO, ADC_CNV); }
    void CskLo() { PinClear(ADC_GPIO, ADC_CNV); }
public:
    Thread *PThread;
    VirtualTimer TmrUartRx;
    VirtualTimer TmrSPICnv;
    void Init();
    void OnUartCmd(Cmd_t *PCmd);
    void ITask();
    int32_t calculationADC_Rslt(int32_t *result);
    void IIrqHandler();
    void IIrqHandlerTimConv();
    void SignalAdcRsltReady() { chEvtSignalI(PThread, EVTMSK_ADC_READY); }

    App_t(): SamplingTmr(), /*ADC_Tmr(),*/ ResolutionMask(0xFFFF), PThread(nullptr) {}
};
extern App_t App;



#endif /* APPLICATION_H_ */
