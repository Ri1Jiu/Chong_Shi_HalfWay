#ifndef __EX_OLED_H
#define __EX_OLED_H

#include <stdio.h>
#include <stdint.h>

extern int po;
extern int i;         //光标位置
extern int s_DownPage;         //页面下翻指示符
extern int Li;         //灯泡亮起旋转
extern uint32_t key_Chosetime;
extern uint32_t key_Selecttime;
extern uint32_t key_Exittime;
extern uint32_t YEYE_Show_Frame;
extern int s_page;    //展示菜单页   一级菜单
extern int s_contr;    //函数实现时控制用
extern int DMA_CH;   //DMA状态
int  EXIT       (void);
int  READPin    (void);
void READPin_1_1(void);
int  READPinPin (void);
extern int READPin_PWM(void);
int  READPinPin_PWM(void);

int  READPin_TIM_Count(void);
int  READPinPin_TIM_Count(void);
//DMA
extern char UL[10];
uint8_t tick_elapsed ( uint32_t * last ,uint32_t period);
uint8_t tick_elapsed_UnCompose( uint32_t* last ,uint32_t period);
void OLED_PreFrame  (void);
void OLED_Frame_1   (void);
void OLED_Frame_1_1 (void);
void OLED_Frame_1_2 (void);
void OLED_Frame_1_3 (void);
void OLED_Frame_1_4 (void);

//---------------------{一级菜单实现预备}----------------------------------------
void PWM_Light(void);
void Encoder_Light(void);
void UART_Light(void);
//---------------------{一级菜单实现预备}----------------------------------------

void UART_OLED(void);
void OLED_UART(void);
void UART_Review(void);
void UART_Slaver(void);
//---------------------{一级菜单实现预备}----------------------------------------
void Encoder_Slaver(void);
void ADC(void); 


//---------------------{一级菜单实现预备}----------------------------------------

void TIM_UP_Count (void); 
void TIM_DOWN_Count (void); 
void TIM_MPU(void); 


//TIM2 -> Encoder
//TIM3 -> PWM_Light
//TIM4 -> Slavor
//IIC1 -> OLED
//IIC2 -> ???

#endif


