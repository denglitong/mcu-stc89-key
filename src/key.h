//
// Created by Litong Deng on 2022/10/13.
//

#ifndef MCU_STC89_KEY_H
#define MCU_STC89_KEY_H

#include <8051.h>

void get_single_input_key_with_delay();

void get_single_input_key_with_interrupt();

void get_matrix_input_key_with_interrupt();

void InterruptTime0_key() __interrupt(1);

void InterruptTime1_key() __interrupt(3);

void turn_on_led_master_switch();

void turn_off_all_leds();

void switch_single_led(unsigned char i);

#endif // MCU_STC89_KEY_H
