/*
 * adc_f2.cpp
 *
 *  Created on: 25 ���. 2013 �.
 *      Author: kreyl
 */

#include "adc_f2.h"
#include "application.h"
#include "cmd_uart.h"

static Thread *PAdcThread;
extern App_t App;

// Wrapper for IRQ
extern "C" {
void AdcTxIrq(void *p, uint32_t flags) {
    chSysLockFromIsr();
    LED1_TOGGLE();
    dmaStreamDisable(ADC_DMA);

    // Resume thread if any
    if(PAdcThread != NULL) {
        if(PAdcThread->p_state == THD_STATE_SUSPENDED) chSchReadyI(PAdcThread);
        PAdcThread = NULL;
   }
    chEvtSignalI(App.PThread, EVTMSK_ADC_RSLT_READY);
    chSysUnlockFromIsr();
}
} // extern C

void Adc_t::Init() {
    PThread = chThdSelf();
    rccEnableADC1(FALSE);   // Enable digital clock
    SetupClk(adcDiv8);      // Setup ADCCLK
    SetChannelCount(ADC_CHANNEL_CNT);
    for(uint8_t i = 0; i < ADC_CHANNEL_CNT; i++) ChannelConfig(AdcChannels[i]);
    // ==== DMA ====
    // Here only unchanged parameters of the DMA are configured.
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_LOW, AdcTxIrq, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC1->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
}

void Adc_t::Measure() {
    // DMA
    dmaStreamSetMemory0(ADC_DMA, Result);
    dmaStreamSetTransactionSize(ADC_DMA, ADC_BUF_SZ);
    dmaStreamSetMode(ADC_DMA, ADC_DMA_MODE);
    dmaStreamEnable(ADC_DMA);
    // ADC
    ADC1->CR1 = ADC_CR1_SCAN;
    ADC1->CR2 = ADC_CR2_DMA | ADC_CR2_ADON;
    chSysLock();
    PAdcThread = chThdSelf();
    StartConversion();
    chSchGoSleepS(THD_STATE_SUSPENDED);
    chSysUnlock();
//    Uart.Printf("ADC Result = %d!!!!!!!!!!!!!!!!!!!!\r",(int32_t)IResult());
//    LED1_TOGGLE();
    ADC1->CR2 = 0;
}

void Adc_t::SetChannelCount(uint32_t Count) {
    chDbgAssert((Count > 0), "Adc Ch cnt=0", NULL);
    ADC1->SQR1 &= ~ADC_SQR1_L;  // Clear count
    ADC1->SQR1 |= (Count - 1) << 20;
}

void Adc_t::ChannelConfig(AdcChnl_t ACfg) {
    chDbgAssert((ACfg.N > 0), "Adc Ch N=0", NULL);
    uint32_t Offset;
    // Setup sampling time of the channel
    if(ACfg.N <= 9) {
        Offset = ACfg.N * 3;
        ADC1->SMPR2 &= ~((uint32_t)0b111 << Offset);    // Clear bits
        ADC1->SMPR2 |= ACfg.SampleTime << Offset;       // Set 0; // new bits
    }
    else {
        Offset = (ACfg.N - 10) * 3;
        ADC1->SMPR1 &= ~((uint32_t)0b111 << Offset);
        ADC1->SMPR1 |= ACfg.SampleTime << Offset;
    }
    // Put channel number in appropriate place in sequence
    if(ACfg.SeqIndx <= 6) {
        Offset = (ACfg.SeqIndx - 1) * 5;
        ADC1->SQR3 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR3 |= (uint32_t)(ACfg.N << Offset);
    }
    else if(ACfg.SeqIndx <= 12) {
        Offset = (ACfg.SeqIndx - 7) * 5;
        ADC1->SQR2 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR2 |= (uint32_t)(ACfg.N << Offset);
    }
    else if(ACfg.SeqIndx <= 16) {
        Offset = (ACfg.SeqIndx - 13) * 5;
        ADC1->SQR1 &= ~(uint32_t)(0b11111 << Offset);
        ADC1->SQR1 |= (uint32_t)(ACfg.N << Offset);
    }
}

void Adc_t::Disable() {
    ADC1->CR2 = 0;
    dmaStreamRelease(ADC_DMA);
}
