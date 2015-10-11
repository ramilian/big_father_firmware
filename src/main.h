/*
 * main.h
 *
 *  Created on: 21 дек. 2014 г.
 *      Author: Roman
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "ch.h"
#include "kl_lib_f2xx.h"

#include "cmd_uart.h"
#include "evt_mask.h"

#define LEDS_GPIO           GPIOA
#define LED1_PIN            0

#define LED1_ON()           PinSet  (LEDS_GPIO, LED1_PIN)
#define LED1_OFF()          PinClear(LEDS_GPIO, LED1_PIN)
#define LED1_TOGGLE()         PinToggle  (LEDS_GPIO, LED1_PIN);


#endif /* MAIN_H_ */
