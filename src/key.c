//
// Created by Litong Deng on 2022/10/13.
//

#include "key.h"

#include "common.h"

unsigned char KEY_STATUS = 1;
unsigned char PREV_KEY_STATUS = 1;
// 2bytes, 16bits, 16ms
static unsigned short KEY_BUFF = 0xFFFF;

unsigned char MATRIX_KEY_STATUS[4][4] = {
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1},
};
unsigned char MATRIX_PREV_KEY_STATUS[4][4] = {
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1},
    {1, 1, 1, 1},
};
unsigned short MATRIX_KEY_BUFFER[4][4] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
};

void get_single_input_key_with_delay() {
  turn_on_led_master_switch();
  turn_off_all_leds();

  while (1) {
    KEY_OUT_1 = 0;

    // KeyIn1 状态发生变化
    if (PREV_KEY_STATUS != KEY_IN_1) {
      // 延时 10ms 以消除物理按键抖动
      // 【在正式项目里使用这种延时逻辑是不被允许的】
      delay_ms(10);
      // 按键消除抖动后 KeyIn1 如果仍然是 turn on 状态
      if (KEY_IN_1 == 0) {
        // 改变 LED0 的状态：亮->灭->亮->灭
        switch_single_led(0);
      }
      PREV_KEY_STATUS = KEY_IN_1;
    }
  }
}

void get_single_input_key_with_interrupt() {
  turn_on_led_master_switch();
  turn_off_all_leds();

  EA = 1;  // enable global interrupt
  ET0 = 1; // enable Timer0 interrupt

  // setup T0_M1 = 0, T0_M0 = 1 (Timer0 mode TH0-TL0 16 bits timer)
  TMOD = 0x01;
  // setup TH0 TL0 initial value
  TH0 = 0xFC;
  TL0 = 0x67;
  TR0 = 1; // start/enable Timer0

  KEY_OUT_1 = 0;

  while (1) {

    if (PREV_KEY_STATUS != KEY_STATUS) {
      // 按键消除抖动后 KeyIn1 如果仍然是 turn on 状态
      if (KEY_STATUS == 0) {
        // 改变 LED0 的状态：亮->灭->亮->灭
        switch_single_led(0);
      }
      PREV_KEY_STATUS = KEY_STATUS;
    }
  }
}

void get_matrix_input_key_with_interrupt() {
  turn_on_led_master_switch();
  turn_off_all_leds();

  EA = 1;  // enable global interrupt
  ET1 = 1; // enable Timer1 interrupt

  // setup T1_M1 = 0, T1_M0 = 1 (Timer1 mode TH1-TL1 16 bits timer)
  TMOD = 0x10;
  // setup TH0 TL0 initial value
  TH1 = 0xFC;
  TL1 = 0x67;
  TR1 = 1; // start/enable Timer1

  unsigned char key_in = 0, key_out = 0;
  while (1) {
    for (key_out = 0; key_out < 4; ++key_out) {
      // enable KeyOut_i
      P2 &= (0xF0 | ~((1 << (3 - key_out))));
      for (key_in = 0; key_in < 4; ++key_in) {
        if (MATRIX_PREV_KEY_STATUS[key_out][key_in] !=
            MATRIX_KEY_STATUS[key_out][key_in]) {
          // 这里结合 key_out, key_in 就可以知道是按下了哪个按键
          if (MATRIX_KEY_STATUS[key_out][key_in] == 0) {
            switch_single_led(key_in);
          }
          MATRIX_PREV_KEY_STATUS[key_out][key_in] =
              MATRIX_KEY_STATUS[key_out][key_in];
        }
      }
    }
  }
}

void InterruptTime0_key() __interrupt(1) {
  // setup TH0 TL0 initial value, each interrupt will pass 1ms
  TH0 = 0xFC;
  TL0 = 0x67;

  // single key input
  KEY_BUFF = (KEY_BUFF << 1) | KEY_IN_1;
  if (KEY_BUFF == 0x0000) {
    // 单片机往往是检测状态，而不是监测过程；常常利用中断来判断按键处于什么状态
    KEY_STATUS = 0;
  } else if (KEY_BUFF == 0xFFFF) {
    KEY_STATUS = 1;
  }
}

void InterruptTime1_key() __interrupt(3) {
  // setup TH1 TL1 initial value, each interrupt will pass 1ms
  TH1 = 0xFC;
  TL1 = 0x67;

  // matrix key input
  unsigned char key_in = 0, key_out = 0, key_in_i;
  for (key_in = 0; key_in < 4; ++key_in) {
    for (key_out = 0; key_out < 4; ++key_out) {
      // get KeyIn_i
      key_in_i = (P2 >> (4 + key_in)) & 1;
      MATRIX_KEY_BUFFER[key_out][key_in] =
          (MATRIX_KEY_BUFFER[key_out][key_in] << 1) | key_in_i;
      if (MATRIX_KEY_BUFFER[key_out][key_in] == 0x0000) {
        MATRIX_KEY_STATUS[key_out][key_in] = 0;
      } else if (MATRIX_KEY_BUFFER[key_out][key_in] == 0xFFFF) {
        MATRIX_KEY_STATUS[key_out][key_in] = 1;
      }
    }
  }
}

// 教学板子 LED_SINGLE 总开关
void turn_on_led_master_switch() {
  enable_u3_74hc138();
  // 110 LEDS6 为低电平，三极管导通，LED 总开关打开
  ADDR_2 = 1;
  ADDR_1 = 1;
  ADDR_0 = 0;
}

void turn_off_all_leds() { LED_LINE = 0xff; }

void switch_single_led(unsigned char i) {
  // switch the i-th led from low-high
  LED_LINE ^= 0x01 << i;
}
