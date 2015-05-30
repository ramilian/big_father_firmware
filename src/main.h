/*
 * main.h
 *
 *  Created on: 21 дек. 2014 г.
 *      Author: Roman
 */

#ifndef MAIN_H_
#define MAIN_H_



#define VERSION     "v0.2"
//===== Leds =====
#define LEDS_GPIO   GPIOA
#define LED1_PIN    0

#define LED1_ON()   PinSet  (LEDS_GPIO, LED1_PIN)
#define LED1_OFF()   PinClear(LEDS_GPIO, LED1_PIN)


#endif /* MAIN_H_ */
