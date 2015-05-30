/*
 * adc_ext.cpp
 *
 *  Created on: 24 апр. 2015 г.
 *      Author: Roman
 */

#include "adc_ext.h"
#include "cmd_uart.h"
#include "application.h"

eAdc_t Adc;

extern "C" {
// DMA irq
void SIrqDmaHandler(void *p, uint32_t flags) { Adc.IrqDmaHandler(); }

void TmrCnvCallback(void *p) {
    Adc.SpiEnabled();
}
} // extern c

void eAdc_t::Init() {
    PinSetupOut(ADC_GPIO, ADC_CNV, omPushPull, pudNone);
    PinSetupOut(ADC_GPIO, ADC_MOSI, omPushPull, pudNone);
    PinSetupAlterFunc(ADC_GPIO, ADC_CLK, omPushPull, pudNone, AF5);
    PinSetupAlterFunc(ADC_GPIO, ADC_MISO, omPushPull, pudNone, AF5);
    CskLo();
    PinSet(ADC_GPIO, ADC_MOSI);
    // ==== SPI ====    MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    // Select baudrate (2.4MHz max): APB=32MHz => div = 16
    ISpi.Setup(ADC_SPI, boMSB, cpolIdleLow, cphaFirstEdge, sbFdiv16);
    ISpi.SetModeRxOnly();
    ISpi.EnableRxDma();
    // ==== DMA ====
    dmaStreamAllocate     (ADC_DMA, IRQ_PRIO_MEDIUM, SIrqDmaHandler, NULL);
    dmaStreamSetPeripheral(ADC_DMA, &ADC_SPI->DR);
    dmaStreamSetMode      (ADC_DMA, ADC_DMA_MODE);
}

void eAdc_t::startADC_SPIMeasure() {
    CskHi();

    chSysLockFromIsr();
    chVTSetI(&TmrSPICnv, US2ST(ADC_CONVERSATION_US), TmrCnvCallback, nullptr);
    chSysUnlockFromIsr();
/*    (void)ADC_SPI->DR;  // Clear input register
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 3);
    dmaStreamSetMode(ADC_DMA, ADC_DMA_MODE);
    dmaStreamEnable(ADC_DMA);
    ISpi.Enable();
    CskLo();*/
}

void eAdc_t::SpiEnabled() {
//    (void)ADC_SPI->DR;  // Clear input register
    dmaStreamSetMemory0(ADC_DMA, &Adc.Rslt);
    dmaStreamSetTransactionSize(ADC_DMA, 3);
    dmaStreamSetMode(ADC_DMA, ADC_DMA_MODE);
    dmaStreamEnable(ADC_DMA);
    ISpi.Enable();
    CskLo();
}

uint16_t eAdc_t::Measure() {
    uint8_t b;
    uint32_t r = 0;
    b = ISpi.ReadWriteByte(0);
    r = b;
    b = ISpi.ReadWriteByte(0);
    r = (r << 8) | b;
    b = ISpi.ReadWriteByte(0);
    r = (r << 8) | b;
    r >>= 2;
    r &= 0xFFFF;
    return r;
}

void eAdc_t::IrqDmaHandler() {
    chSysLockFromIsr();
    ISpi.Disable();
//    dmaStreamClearInterrupt(ADC_DMA);
//    dmaWaitCompletion(ADC_DMA);
    dmaStreamDisable(ADC_DMA);
    Rslt = __REV(Rslt);
    Rslt >>= 10;
    Rslt &= 0xFFFF;
    (void)ADC_SPI->DR;
//    Uart.Printf("%u\r", Rslt);
    App.SignalAdcRsltReady();
    chSysUnlockFromIsr();
}
