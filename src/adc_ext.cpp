/*
 * adc_ext.cpp
 *
 *  Created on: 24 апр. 2015 г.
 *      Author: Roman
 */

#include "adc_ext.h"
#include "cmd_uart.h"
#include "application.h"
#include "main.h"

eAdc_t Adc;

extern "C" {
// DMA irq
void SIrqDmaHandler(void *p, uint32_t flags) { Adc.IrqDmaHandler(); }
} // extern c

void eAdc_t::Init() {
    PThread = chThdSelf();
    PinSetupOut(ADC_GPIO, ADC_CNV, omPushPull, pudNone);
    PinSetupOut(ADC_GPIO, ADC_SDI, omPushPull, pudNone);
    PinSetupAlterFunc(ADC_GPIO, ADC_SCLK, omPushPull, pudNone, AF5);
//    PinSetupAlterFunc(ADC_GPIO, ADC_SDO, omPushPull, pudNone, AF5);
    PinSet(ADC_GPIO, ADC_SDI);  //select CS MODE
    ADC_CNV_LOW();               // Idle mode
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    // Select baudrate (2.4MHz max): APB=32MHz => div = 16
    ISpi.Setup(ADC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv2, sbc16Bit);
//    ISpi.Enable();
    //    ISpi.SetModeRxOnly();
    ISpi.EnableRxDma();
    // ==== DMA ====
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 3);
    dmaStreamEnable(ADC_DMA);


    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(5/*10000*/); // Start Fsmpl value  tsamp = 0.1 msec
    SamplingTmr.EnableIrq(TIM2_IRQn, IRQ_PRIO_MEDIUM);
    SamplingTmr.EnableIrqOnUpdate();
    SamplingTmr.Enable();

    CskTmr.Init(TIM5);
    CskTmr.SetUpdateFrequency(500000); // Start Fsmpl value  tsamp = 2 usec
    CskTmr.EnableIrq(TIM5_IRQn, IRQ_PRIO_MEDIUM);
    CskTmr.EnableIrqOnUpdate();
    CskTmr.Disable();
}


void eAdc_t::IrqDmaHandler() {
    chSysLockFromIsr();
    ISpi.Disable();
    dmaStreamDisable(ADC_DMA);
    App.SignalAdcRsltReady();
    chSysUnlockFromIsr();
}

// CNV IRQ
void eAdc_t::IIrqHandler() {
    CskTmr.Disable();
    CskTmr.SetUpdateFrequency(1/*500000*/);
    ADC_CNV_LOW();
    //Uart.Printf("CNV timer IRQ\r");
//    Adc.StartDMAMeasure();
}
// Sampling IRQ
void eAdc_t::IIrqSmpHandler() {

    ADC_CNV_HI();
    PinSetupOut(ADC_GPIO, ADC_SDO, omPushPull, pudPullUp);

    ISpi.Disable();
    IrqSDO.Setup(ADC_GPIO, ADC_SDO,ttFalling);
    IrqSDO.EnableIrq(IRQ_PRIO_MEDIUM);

    CskTmr.SetUpdateFrequency(500000);
    CskTmr.Enable();
    LED1_TOGGLE();
    //Uart.Printf("Sampling timer IRQ\r");
}

void eAdc_t::IIrqExtiHandler() {
    IrqSDO.CleanIrqFlag();
    IrqSDO.DisableIrq();
    PinSetupAlterFunc(ADC_GPIO, ADC_SDO, omPushPull, pudNone, AF5);
    ISpi.Enable();
}

extern "C" {
void TIM2_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    if(TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        Adc.IIrqSmpHandler();
    }
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
void TIM5_IRQHandler(void) {
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    if(TIM5->SR & TIM_SR_UIF) {
        TIM5->SR &= ~TIM_SR_UIF;
        TIM5->CCER;
        Adc.IIrqHandler();
    }
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
void EXTI15_10_IRQHandler(void) {
    Uart.Printf("EXTI IRQ\r");
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    Adc.IIrqExtiHandler();
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}
