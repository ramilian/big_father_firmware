/*
 * adc_ext.cpp
 *
 *  Created on: 24 ���. 2015 �.
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
    PinSetupAlterFunc(ADC_GPIO, ADC_SDO, omPushPull, pudNone, AF5);
    PinSet(ADC_GPIO, ADC_SDI);  //select CS MODE
    ADC_CNV_LOW();               // Idle mode
    // ==== DMA ====
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 1);
    dmaStreamEnable(ADC_DMA);
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    // Select baudrate (2.4MHz max): APB=120MHz => div = 64
    ISpi.Setup(ADC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv64, sbc16Bit);
    ISpi.SetModeRxOnly();
    ISpi.EnableRxDma();
    ISpi.Enable();


    // ==== Sampling timer ====
    SamplingTmr.Init(TIM2);
    SamplingTmr.SetUpdateFrequency(100/*10000*/); // Start Fsmpl value  tsamp = 0.1 msec
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
    dmaStreamDisable(ADC_DMA);
    App.SignalAdcRsltReady();
    SPI1->DR = 0;
    chSysUnlockFromIsr();
}

// CNV IRQ
void eAdc_t::IIrqHandler() {
    CskTmr.Disable();
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 1);
    dmaStreamEnable(ADC_DMA);
    ADC_CNV_LOW();
}
// Sampling IRQ
void eAdc_t::IIrqSmpHandler() {
    ADC_CNV_HI();
    CskTmr.Enable();
}



// ������� ���������� (�� ������������)
void eAdc_t::IIrqExtiHandler() {
    IrqSDO.CleanIrqFlag();
    IrqSDO.DisableIrq();
    PinSetupAlterFunc(ADC_GPIO, ADC_SDO, omPushPull, pudNone, AF5);

    (void)ADC_SPI->DR;  // Clear input register
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 1);
    dmaStreamEnable(ADC_DMA);
    ISpi.Enable();

//    LED1_TOGGLE();
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
    CH_IRQ_PROLOGUE();
    chSysLockFromIsr();
    Adc.IIrqExtiHandler();
    chSysUnlockFromIsr();
    CH_IRQ_EPILOGUE();
}
}
