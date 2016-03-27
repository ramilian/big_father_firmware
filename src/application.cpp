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
extern Adc_t iAdc;
void App_t::Init() {
    PThread = chThdSelf();
}

void App_t::ITask() {
    uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
    if(EvtMsk & EVTMSK_ADC_READY) {
        static int32_t summ = 0;
        static int32_t count = 0;
        int16_t x0 = (int16_t)(Adc.Rslt & ResolutionMask);             //value
        summ += (int32_t)x0;
        count++;
        if(count==10){
            summ /= count;
            Uart.Printf("%d\r", summ);
            summ = 0;
            count = 0;
        }
        Adc.Rslt =0;
    }
    if(EvtMsk & EVTMSK_UART_RX_POLL) { Uart.PollRx(); } // Check if new cmd received
    if(EvtMsk & EVTMSK_ADC_RSLT_READY) {
/*
        Uart.Printf("ADC1 = %d  ADC2 = %d  ADC3 = %d  ADC4 = %d\r",   (int32_t)iAdc.Result[0], \
                                                                      (int32_t)iAdc.Result[1], \
                                                                      (int32_t)iAdc.Result[2], \
                                                                      (int32_t)iAdc.Result[3]);
*/
    }
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
