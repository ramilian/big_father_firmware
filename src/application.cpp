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
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {
        LED1_ON();
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
    if(PCmd->NameIs("#Ping") || PCmd->NameIs("#ping")) Uart.Ack(OK);
    else if (PCmd->NameIs("#Ledon") || PCmd->NameIs("#ledon")) {LED1_ON();}
    else if (PCmd->NameIs("#Ledoff") || PCmd->NameIs("#ledoff")) {LED1_OFF();}
    else if (PCmd->NameIs("#Ledtoggle") || PCmd->NameIs("#ledtoggle")) {isLedToggle = !isLedToggle;}
    else if(*PCmd->Name == '#') Uart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
}
