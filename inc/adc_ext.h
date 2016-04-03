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

#define FSAMPL_ADC      500/*00*/     // /*10000*/ Hz  Start Fsmpl value  tsamp = 0.1 msec
#define FSAMPL_CNV      500000   // request 2 usec

#define ADC_SPI     SPI2
#define ADC_GPIO    GPIOB

#define ADC_CNV         12   //slave select Start conversion
#define ADC_SCLK        13   //serial clock
#define ADC_SDO         14   //
#define ADC_SDI         15      //!CS

#define ADC_CNV_HI()    PinSet  (ADC_GPIO, ADC_CNV)
#define ADC_CNV_LOW()   PinClear(ADC_GPIO, ADC_CNV)

#define EADC_DMA         STM32_DMA1_STREAM3
#define EADC_DMA_CHNL    0
#define EADC_DMA_MODE    STM32_DMA_CR_CHSEL(EADC_DMA_CHNL) | \
                        DMA_PRIORITY_MEDIUM | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */


class eAdc_t {
private:

    void CskHi() { PinSet(ADC_GPIO, ADC_CNV); }
    void CskLo() { PinClear(ADC_GPIO, ADC_CNV); }

public:
    Spi_t ISpi;
    Thread *PThread;
    uint32_t Rslt;
    int16_t count_measure;
    Timer_t CskTmr, SamplingTmr;
    IrqPin_t IrqSDO;
    void Init();
    // Inner use
    void IrqDmaHandler();
    void IIrqHandler();
    void IIrqSmpHandler();
    void IIrqExtiHandler();
//    eAdc_t(): measureRslt(0), PThread(nullptr), Rslt(0), count_measure(0) {}
};

extern eAdc_t Adc;
#endif /* ADC_EXT_H_ */
