/*
 * application.cpp
 *
 *  Created on: 21 дек. 2014 г.
 *      Author: Roman
 */

#include "application.h"
#include "cmd_uart.h"
#include "evt_mask.h"
#include "main.h"


#include <cstdlib>


extern eAdc_t Adc;



void App_t::Init() {
    PThread = chThdSelf();
    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(10000); // Start Fsmpl value  tsamp = 0.1 msec
    SamplingTmr.EnableIrq(TIM2_IRQn, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {

/*    int16_t x0 = 0;
    static int32_t summ = 0;
    static int32_t count = 0;
//    result = nullptr;
    x0 = (int16_t)(Adc.Rslt & ResolutionMask);             //value
    summ += x0;
    count++;
    if(count==10){
        summ /= count;
        Uart.Printf("***BIG FATHER: Measure = %d\r", summ);
        summ = 0;
        count = 0;
    }*/


        int32_t adcMeasure = -1;
        int32_t measureCount = -1;
        if ((measureCount = App.calculationADC_Rslt(&adcMeasure)) == 10) {
            Uart.Printf("***BIG FATHER: Measure = %d\r", adcMeasure);
        }
    }
    if(EvtMsk & EVTMSK_UART_RX_POLL) { Uart.PollRx(); } // Check if new cmd received
}

int32_t App_t::calculationADC_Rslt(int32_t *result)
{
    int32_t retval = -1;
    int16_t x0 = 0;
    static int32_t summ = 0;
    static int32_t count = 0;
    x0 = (int16_t)(Adc.Rslt & ResolutionMask);             //value
    summ += x0;
    count++;
    retval = count;
    if(count==10){
        summ /= count;
        result[0] = summ;
        summ = 0;
        count = 0;
    }
    return retval;
}

void App_t::OnUartCmd(Cmd_t *PCmd) {
    Uart.Printf("%S\r", PCmd->Name);
    uint32_t dw32 __attribute__((unused));  // May be unused in some cofigurations
    if(PCmd->NameIs("#Ping")) Uart.Ack(OK);
    else if (PCmd->NameIs("#Ledon")) {LED1_ON();}
    else if (PCmd->NameIs("#Ledoff")) {LED1_OFF();}
    else if(*PCmd->Name == '#') Uart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}


#if 1 // ============================= IRQ =====================================
// Sampling IRQ: output y0 and start new measurement. ADC will inform app when completed.



void App_t::IIrqHandler() {
    Adc.startADC_SPIMeasure();
//    CskHi();
//    chVTSetI(&TmrSPICnv, US2ST(ADC_CONVERSATION_US), TmrCnvCallback, nullptr);
}


#if 1 // ==== Sampling Timer =====
extern "C" {
void TIM2_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    if(TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        App.IIrqHandler();
    }
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}
#endif

#endif

