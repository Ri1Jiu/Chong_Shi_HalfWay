#ifndef __EX_OLED_H
#define __EX_OLED_H

extern int po = 1;
extern int i;
extern int j;
extern int state = 1;
void READPin(int * idx);
uint8_t tick_elapsed( int* last ,uint32_t period);
void OLED_PreFrame(void);
void OLED_Frame_1 (int * i);
void OLED_Frame_1_1 (int * o);
void OLED_Frame_1_2 (int * p);
void READPinPin(int * idx);

//---------------------{一级菜单实现预备}----------------------------------------
void PWM_Light(int * jdx);
void Encoder_Light(void);
void UART_Light(void);
void UART_OLED_Transmit(void);

//tim2 -> PWM_Light
//TIM3 -> Encoder
//TIM4 -> Slavor
#endif 
