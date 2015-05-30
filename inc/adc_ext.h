/*
 * adc_ext.h
 *
 *  Created on: 24 апр. 2015 г.
 *      Author: Roman
 */

#ifndef ADC_EXT_H_
#define ADC_EXT_H_

#include "stm32f2xx.h"
#include "ch.h"
#include "kl_lib_f2xx.h"
#include "hal.h"

#define VERSION     "v0.2"
#define ADC_SPI     SPI2
#define ADC_GPIO    GPIOB

#define ADC_CNV     12   //slave select Start conversion
#define ADC_CLK     13   //serial clock
#define ADC_MISO    14   //
#define ADC_MOSI    15      //!CS

#define ADC_DMA         STM32_DMA1_STREAM0
#define ADC_DMA_CHNL    3
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(ADC_DMA_CHNL) | \
                        DMA_PRIORITY_MEDIUM | \
                        STM32_DMA_CR_MSIZE_BYTE | \
                        STM32_DMA_CR_PSIZE_BYTE | \
                        STM32_DMA_CR_MINC | \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */

#define ADC_CONVERSATION_US  2

class eAdc_t {
private:
    Spi_t ISpi;
    void CskHi() { PinSet(ADC_GPIO, ADC_CNV); }
    void CskLo() { PinClear(ADC_GPIO, ADC_CNV); }

public:
    uint32_t Rslt;
    void Init();
    uint16_t Measure();
    // Inner use
    void IrqDmaHandler();
    void SpiEnabled();
};

extern eAdc_t Adc;
#endif /* ADC_EXT_H_ */
