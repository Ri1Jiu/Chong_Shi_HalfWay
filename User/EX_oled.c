#include "EX_oled.h"
#include "oled.h"
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

    int po = 1;
    int i = 0;
    int o = 0;
    int p = 0;
    int j = 0;
    int state = 1;
    int STaTe = 0;
    int STATE = 0;
    typedef void (* FUN)(int * idx);


//---------------------{ALL级菜单 按键}----------------------------------
//在不同标签页下保持功能
FUN Frame[5] = { OLED_Frame_1 , OLED_Frame_1_1 , OLED_Frame_1_2, OLED_Frame_1_3, OLED_Frame_1_4};


//-----------------{函数调用 二维数组}---------------------------


//---------------------{一级菜单(函数) }---------------------------------
FUN * function[4] = { Light, UART ,Encoder , TIM };
FUN Light[2] = { PWM_Light , Encoder_Light  };
FUN UART[3] = { UART_OLED_Transmit , UART_show , UART_Review };
FUN Encoder[2] = { Encoder_ };
FUN TIM[4] = { TIM_UP_Count , TIM_DOWN_Count };

















// 读取KEY按下情况,进行页面切换
void READPin(int * idx)
{
    if(!HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
    {
        HAL_Delay(30);
         if(!HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
         {
            (*idx )++; 
            Frame[state](idx);
            // OLED_Frame_1 (idx);
         }
         while(!HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){}
    }
}



//---------------------{二级菜单实现 (选中函数) }---------------------------------
void READPinPin(int * idx)
{
    if(!HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
    {
        HAL_Delay(20);
         if(!HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
         {
            if(STaTe == 0)
            {
                state = *idx;
                *idx = 0;
                Frame[state](*idx);
            }
            else if(STaTe == 1)
            {
                function[STaTe][*idx];
            }
            else if(STaTe == 2)
            {
                function[STaTe][*idx];
            }


         }
        //  state = 11;
         while(!HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){}
    }
}



//---------------------{ 非阻塞函数 }----------------------------------
uint8_t tick_elapsed( int* last ,uint32_t period)
{
    uint32_t now = HAL_GetTick();
    if( (uint32_t)( now - *last ) >= period)
    {
        return 1;
    }
    return 0;
}


//开机动画
void OLED_PreFrame(void)
{
    OLED_NewFrame();
    OLED_DrawImage(32 ,11 ,&bilibiliImg ,OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    HAL_Delay(400);
    
}


// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID
// For PID




//---------------------{一级菜单    数组}----------------------------------
//一级菜单设置的数组
char menu1[4][15] = 
{
    "Light",
    "UART",
    "Encoder",
    "TIM",
};
//---------------------{二级 一号 菜单    数组}----------------------------------
//二级 一号 菜单设置的数组
char menu1_1[3][20] = 
{
    "PWM Light",
    "Encoder Light",
    "UART_Light",
};
//---------------------{二级 二号 菜单    数组}----------------------------------
//二级 二号 菜单设置的数组
char menu1_2[3][20] = 
{
    "UART_OLED_Transmit",
    "UART_show",
    "UART_Review",
};
//---------------------{二级 三号 菜单    数组}----------------------------------
//二级 三号 菜单设置的数组
char menu1_3[2][20] = 
{
    "Encoder",
    " ",
};
//---------------------{二级 四号 菜单    数组}----------------------------------
//二级 四号 菜单设置的数组
char menu1_4[3][20] = 
{
    "TIM_UP_Count",
    "TIM_DOWN_Count",
    "",
};



//---------------------{一级菜单}----------------------------------
void OLED_Frame_1 (int * i)
{

    OLED_NewFrame();

    int height = *i * 16;
    *i %= 4;
    if(*i == 3)           //最后一行靠下，看不到边框
    {
        height -= 2;
    }
    OLED_PrintString(4 , 0 , menu1[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16, menu1[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 32, menu1[2] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 48, menu1[3] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, height, strlen(menu1[*i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    
}

//---------------------{二级 一号 菜单}----------------------------------
void OLED_Frame_1_1 (int * i)
{
    OLED_NewFrame();

    OLED_PrintString(4 , 0 , menu1_1[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16, menu1_1[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, *i * 16, strlen(menu1_1[*i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ---------------------{二级 二号 菜单}----------------------------------
void OLED_Frame_1_2 (int * i)
{
    OLED_NewFrame();

    OLED_PrintString(4 , 0 , menu1_2[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_2[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, *i * 16, strlen(menu1_2[*i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ---------------------{二级 三号 菜单}----------------------------------
void OLED_Frame_1_3 (int * i)
{
    OLED_NewFrame();

    OLED_PrintString(4 , 0 , menu1_3[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_3[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, *i * 16, strlen(menu1_3[*i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ---------------------{二级 四号 菜单}----------------------------------
void OLED_Frame_1_4 (int * i)
{
    OLED_NewFrame();

    OLED_PrintString(4 , 0 , menu1_4[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_4[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, *i * 16, strlen(menu1_4[*i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}



//---------------------{二级一号菜单实现预备}----------------------------------------
//预备啦!!!  一级菜单的实现！！！
void PWM_Light(int * jdx)
{
    char TIM[4] = { TIM_CHANNEL_1,TIM_CHANNEL_2,TIM_CHANNEL_3,TIM_CHANNEL_ALL };
    HAL_TIM_PWM_Start(&htim3,TIM[*jdx]);

//------------------------ !! ! 非阻塞改进版 ! !! -------------------------------------------
    // __HAL_TIM_SET_COMPARE(&htim3,TIM[j],0);
    while(1)
    {
        int p = 50;
        int count = 0;
        int last = 0;

        if( tick_elapsed(&last , 10) && count == 0)
        { p++;
          __HAL_TIM_SET_COMPARE(&htim3, TIM[*jdx], p);
          if(p == 100){ count = 1;}
        }
        // __HAL_TIM_SET_COMPARE(&htim3, TIM[j],0);

        else if( tick_elapsed(&last , 10) && count == 1)
        { p--;
          __HAL_TIM_SET_COMPARE(&htim3, TIM[*jdx], p);
          if(p == 50){ count = 0;}
        }
        
    }
}

void Encoder_Light()
{
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_ALL);
    
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_ALL,i);

}


void UART_Light()
{
  MX_USART1_UART_Init();




}


//---------------------{二级 二号菜单实现预备}----------------------------------------
void UART_OLED_Transmit()
{





}


void UART_show()
{





}


void UART_Review ()
{




}


//---------------------{二级 三号菜单实现预备}----------------------------------------

void Encoder_ ()
{








}






//---------------------{二级四号菜单实现预备}----------------------------------------

void TIM_UP_Count ()
{





}


void TIM_DOWN_Count ()
{







}







//---------------------{一级菜单   退出（EXIT））}--------------------------------
//----------------------{！！改为中断执行！！}--------------------------------

// void EXIT_Control(int * idx)
// {
//     if(HAL_GPIO_ReadPin(EXIT_GPIO_Port,EXIT_Pin))
//     {
//         HAL_Delay(30);
//          if(HAL_GPIO_ReadPin(EXIT_GPIO_Port,EXIT_Pin))
//          {

//          }
//          while(HAL_GPIO_ReadPin(EXIT_GPIO_Port,EXIT_Pin)){}
//     }
// }


//------------------------------{二级菜单}------------------------------------//


//-----------------------乐色囤积站----------------------
//----------------------------[PWM呼吸灯]-------------------------
    // while(1)
    // {
    //     __HAL_TIM_SET_COMPARE(&htim3, TIM[*jdx],50);
    //     for(int p = 50 ; p < 100 ; p++)
    //     {
    //       __HAL_TIM_SET_COMPARE(&htim3, TIM[*jdx], p);
    //       HAL_Delay(10);
    //     }

    //     // __HAL_TIM_SET_COMPARE(&htim3, TIM[j],0);


    //     for(int p = 100 ; p >= 50 ; p--)
    //     {
    //       __HAL_TIM_SET_COMPARE(&htim3, TIM[*jdx], p);
    //       HAL_Delay(10);
    //     }

    // }

