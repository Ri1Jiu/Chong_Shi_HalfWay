#include "EX_oled.h"
#include "oled.h"
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

    int po = 1;
    int i = 0;
    int Li = 0;
    uint32_t key_Chosetime  = 0;
    uint32_t key_Selecttime = 0;
    uint32_t key_Exittime   = 0;
    uint32_t YEYE_Show_Frame= 0;
    int s_page = 0;
    int s_contr = 0;
    int DMA_CH = 0; 







    typedef void (* FUN)(void);
//---------------------{一级菜单(函数) }---------------------------------

FUN Light[3] = { PWM_Light , Encoder_Light ,UART_Light};
FUN UART[4] = { UART_OLED , OLED_UART , UART_Review , UART_Slaver};
FUN Encoder[2] = { Encoder_Slaver ,ADC };
FUN TIM[3] = { TIM_UP_Count , TIM_DOWN_Count , TIM_MPU };
FUN * function[4] = { Light, UART ,Encoder , TIM };

//---------------------{ALL级菜单 按键}----------------------------------
//在不同标签页下保持功能
FUN Frame[5] = { OLED_Frame_1 , OLED_Frame_1_1 , OLED_Frame_1_2, OLED_Frame_1_3, OLED_Frame_1_4};






int READPin()       // 读取KEY按下情况,进行页面切换
{
    
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
    {
        HAL_Delay(30);
         if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
         {
         }
         while(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){}
         return 1;   
    }
        return 0;
}


//---------------------{二级菜单实现 (选中函数) }---------------------------------
int  READPinPin()
{
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
    {
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
        {

        }
        while(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){}
        return 1 ;
    }
        return 0 ;
}


//-----------------{函数调用 二维数组}---------------------------
int EXIT ()
{ 
        if(HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin))
    {
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin))
        {
        
        while(HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)){}
        return 1 ;
        }
    }
        return 0 ;
}
//---------------------{二级菜单实现 ( 1_1 ) }---------------------------------
int s_PWM = 0;
// 1  左短
// 2  左长
// 3  两按
// 4  右短
uint32_t doen_count = 0;
int READPin_PWM()
{
    
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
    {
        HAL_Delay(30);
         if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) && HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
         {
            s_PWM = 3;
            
         }
         else if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
         { 
            doen_count = HAL_GetTick();
            while(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){}
            if(HAL_GetTick() - doen_count <= 200)
            {
                s_PWM = 1;
            }
            else 
            {
                s_PWM = 2;
            }
         }
         return 1;   
    }
        return 0;
}

int  READPinPin_PWM()
{
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
    {
        HAL_Delay(20);
         if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) && HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
         {
            s_PWM = 3;
            
         }
         else if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
         {
            s_PWM = 4;
         }
        while(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){}
        return 1 ;
    }
        return 0 ;
}

//---------------------{二级菜单实现 ( 2_1 ) }---------------------------------
static int message_num = 0;
static char RX_buff[18] = "";
static char message[10][18] = {
    "",
    "",
};

//---------------------{二级菜单实现 ( 4_1 ) }---------------------------------

    // static uint32_t Chose = 0;
    // static uint32_t time_UP_count = 0;
    static uint32_t U_second = 0;
    static uint32_t U_min = 0;
    static uint32_t U_hour = 0;
    static uint32_t D_second = 0;
    static uint32_t D_min = 0;
    static uint32_t D_hour = 0;

    static uint8_t TIM_Count_Finish = 0;
    static uint8_t TIM_DOWN_Count_Start = 0;
    static uint8_t TIM_UP_Count_Start = 0;
    uint32_t TIM_Count_Arr_Place = 0;
    static uint32_t * TIM_UP_Count_Arr[3] = {&U_hour , &U_min , &U_second};
    static uint32_t * TIM_DOWN_Count_Arr[3] = {&D_hour , &D_min , &D_second};

int READPin_TIM_Count()             //非选择，是定时用，不改了
{
    
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
    {
        HAL_Delay(30);
         if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin))
         {
         }
         while(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){}
         return 1 ;
    }
    return 0 ;

}


int READPinPin_TIM_Count()
{
    
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
    {
        HAL_Delay(30);
         if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin))
         {

         }
         while(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){}
         return 1 ;
    }
    return 0 ;
}


//---------------------{ ///DMA///DMA/// }----------------------------------
    char UL[10] = "";
    volatile int OLED_ready = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int compare = 0 ;
    
    if( DMA_CH == 13)//UL --》UART_Light
    {
        if(UL[1] == '1')
           {
               compare = 50;
        }
        if(UL[0] == 'R')
        {
            __HAL_TIM_SET_COMPARE(&htim3 ,TIM_CHANNEL_1 ,compare);
        }          
        if(UL[0] == 'B')
        {
            __HAL_TIM_SET_COMPARE(&htim3 ,TIM_CHANNEL_2 ,compare);
        }            
        if(UL[0] == 'Y')
        {
            __HAL_TIM_SET_COMPARE(&htim3 ,TIM_CHANNEL_3 ,compare);
        }
        //统一毕手势
        HAL_UART_Receive_DMA(&huart1 , (uint8_t* )UL ,strlen(UL));
    }
 

    else if (DMA_CH == 21)
    {

        message_num %= 10; 
        
        strncpy(message[message_num], RX_buff, 17);
        message[message_num][17] = '\0';                 /* 手动补结束符 */
        message_num++;
        OLED_ready = 1;            /* 只做标记，让主循环去刷屏 */
        HAL_UART_Receive_DMA(&huart1, (uint8_t *)RX_buff, 17);
    }
      
        // else if()
        // {

        // }
}

//---------------------{ 非阻塞函数 }----------------------------------
uint8_t tick_elapsed( uint32_t* last ,uint32_t period)
{
    uint32_t now = HAL_GetTick();
    if(( now - *last ) >= period)
    {
        *last = now;
        return 1;
    }
    return 0;
}

uint8_t tick_elapsed_UnCompose( uint32_t* last ,uint32_t period)
{
    uint32_t now = HAL_GetTick();
    if(( now - *last ) >= period)
    {
        *last = *last + period; 
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
char menu1_1[4][20] = 
{
    "PWM Light",
    "Encoder Light",
    "UART_Light",
};
//---------------------{二级 二号 菜单    数组}----------------------------------
//二级 二号 菜单设置的数组
char menu1_2[4][20] = 
{
    "UART_OLED",
    "OLED_UART",
    "UART_Review",
    "UART_Slaver",
};
//---------------------{二级 三号 菜单    数组}----------------------------------
//二级 三号 菜单设置的数组
char menu1_3[4][20] = 
{
    "Encoder_Slaver",
    "ADC",
};
//---------------------{二级 四号 菜单    数组}----------------------------------
//二级 四号 菜单设置的数组
char menu1_4[4][20] = 
{
    "TIM_UP_Count",
    "TIM_DOWN_Count",
    "TIM_MPU",
};



//---------------------{一级菜单}----------------------------------
    int to_exit = 0;
void OLED_Frame_1 ()
{
    s_page = 0;



        key_Chosetime  = 0;
        key_Selecttime = 0;
        key_Exittime   = 0;
    while(1)                 //选择
    {

        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
            i++;       //
            continue;
            }
        }

if( tick_elapsed(&YEYE_Show_Frame , (uint32_t) 20) )
{
    OLED_NewFrame();

    i %= 4;
    int height = i * 16;
    if(i == 3)           //最后一行靠下，看不到边框
    {
        height -= 2;
    }
    OLED_PrintString(4 , 0 , menu1[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16, menu1[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 32, menu1[2] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 48, menu1[3] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, height, strlen(menu1[i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}


        // if(to_exit == 1)
        // {
        //     i++;       //
        //     to_exit =  0;
        //     Frame[s_page]();
        // }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {
                if (!to_exit)
                {
                s_page = i + 1;
                i = 0;
                continue;
                }
                // if(to_exit)
                // {
                //     Frame[s_page]();
                // }
            }

        }

        if(s_page != 0)
        {
           Frame[s_page](); 
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {
                char miss1[30] = "NEVER   SAY";
                char miss2[30] = "GOODBOY";
                
                OLED_NewFrame();
                OLED_PrintString(18 ,10,miss1 ,&font16x16 ,OLED_COLOR_NORMAL );
                OLED_PrintString(28 ,28,miss2 ,&font16x16 ,OLED_COLOR_NORMAL );
                OLED_ShowFrame();
            }

        }
    }
}

//---------------------{二级 一号 菜单}----------------------------------
void OLED_Frame_1_1 ()
{
    s_page = 1;



    key_Chosetime  = 0;
    key_Selecttime = 0;
    key_Exittime   = 0;
    while(1)                 //选择
    {

        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
                i++;       //
                continue;
            }
        }

    OLED_NewFrame();

if( tick_elapsed(&YEYE_Show_Frame , (uint32_t) 20) )
{
    i %= 3;
    OLED_PrintString(4 , 0 , menu1_1[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16, menu1_1[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 32, menu1_1[2] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, i * 16 , strlen(menu1_1[i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {
                function[0][i]();
                continue;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {
                i = 0;
                s_page = 0;
                return;
            }

        }
    }

}

// ---------------------{二级 二号 菜单}----------------------------------
void OLED_Frame_1_2 ()
{
    s_page = 2;
    


    key_Chosetime  = 0;
    key_Selecttime = 0;
    key_Exittime   = 0;
    while(1)                 //选择
    {

        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
                i++;       //
                continue;
            }
        }
if( tick_elapsed(&YEYE_Show_Frame , (uint32_t) 20) )
{
    OLED_NewFrame();

    i %= 4;
    int height = i * 16;
    if(i == 3)           //最后一行靠下，看不到边框
    {
        height -= 2;
    }
    OLED_PrintString(4 , 0 , menu1_2[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_2[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 32,  menu1_2[2] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 48,  menu1_2[3] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, i * 16 , strlen(menu1_2[i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}
        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {             
                function[1][i]();
                //i = 0;     已有
                continue;
            }
        }

                if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {
                i = 1;
                s_page = 0;
                return;
            }

        }
    }

}

// ---------------------{二级 三号 菜单}----------------------------------
void OLED_Frame_1_3 ()
{
    s_page = 3;



    key_Chosetime  = 0;
    key_Selecttime = 0;
    key_Exittime   = 0;
    while(1)                 //选择
    {

        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
                i++;       //
            }
        }

if( tick_elapsed(&YEYE_Show_Frame , (uint32_t) 20) )
{
    OLED_NewFrame();
    i %= 2;
    OLED_PrintString(4 , 0 , menu1_3[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_3[1] , &font16x16, OLED_COLOR_NORMAL);

    OLED_DrawRoundRectangle(4 - 1, i * 16, strlen(menu1_3[i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {              
                function[2][i]();

                //i = 0;     已有
            }
        }

                if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())         //离开
            {
                i = 2;
                s_page = 0;
                return ;
            }

        }
    }

}

// ---------------------{二级 四号 菜单}----------------------------------
void OLED_Frame_1_4 ()
{
    s_page = 4;



    key_Chosetime  = 0;
    key_Selecttime = 0;
    key_Exittime   = 0;
    while(1)                 //选择
    {

        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
                i++;       //
                continue;
            }
        }

if( tick_elapsed(&YEYE_Show_Frame , (uint32_t) 20) )
{
    OLED_NewFrame();
    i %= 2;
    OLED_PrintString(4 , 0 , menu1_4[0] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 16,  menu1_4[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(4 , 32,  menu1_4[1] , &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawRoundRectangle(4 - 1, i * 16, strlen(menu1_4[i]) * 8 + 1 , 18 ,3, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {
                function[3][i]();
                //i = 0;     已有
                continue;
            }
        }

                if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())         //离开
            {
                i = 3;
                s_page = 0;
                return;
            }

        }
    }

}



//---------------------{二级一号菜单实现预备}----------------------------------------
//预备啦!!!  一级菜单的实现！！！
int s_LED_0 = 0;
int s_LED_1 = 0;
int s_LED_2 = 0;
int s1_place = 0;
int s2_place = 100;
int s2_idx_place = 0;

int  *s_LED[3] = { &s_LED_0,&s_LED_1, &s_LED_2};
void PWM_Light()
{
        uint32_t PWM_Light_Ligth = 50;
        uint32_t PWM_Light_Time = 0;
        uint8_t PWM_Light_Choose = 0;
        int PWM_Light_state = 0;
    char PWM_Light_CH[4] = { TIM_CHANNEL_1,TIM_CHANNEL_2,TIM_CHANNEL_3,TIM_CHANNEL_ALL };
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);

    OLED_NewFrame();
    OLED_DrawImage( 30 , 0 ,&loookImg ,OLED_COLOR_NORMAL);
    OLED_ShowFrame();

//------------------------ !! ! 非阻塞改进版 ! !! -------------------------------------------
    // __HAL_TIM_SET_COMPARE(&htim3,TIM[j],0);
    while(1)
    {
        if(PWM_Light_state == 0  && tick_elapsed(&PWM_Light_Time , 10))
        { PWM_Light_Ligth --;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 0){ PWM_Light_state = 1;}
        }
        // __HAL_TIM_SET_COMPARE(&htim3, TIM[j],0);

        else if( PWM_Light_state == 1 && tick_elapsed(&PWM_Light_Time , 10))
        { PWM_Light_Ligth++ ;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 50){ PWM_Light_state = 0;}
        }
        


        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin_PWM())
            {
                break;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_PWM())
            {
                break;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {   
                return ;
            }
        }
    }


    while(1)
    {

if(s_PWM == 1)
        {
            s1_place = (s1_place + 1) % 3;
            *s_LED[s1_place]  = (*s_LED[s1_place] + 1) % 2;

            for(int ii = 0; ii < 3 ; ii++)
            {
                if(*s_LED[ii])
                {
                HAL_TIM_PWM_Stop(&htim3 ,PWM_Light_CH[ii]);
                }
                else
                {
                HAL_TIM_PWM_Start(&htim3 ,PWM_Light_CH[ii]);
                }

            }

            s_PWM = 0;
            while(1)
            {
        if( tick_elapsed(&PWM_Light_Time , 10) && PWM_Light_state == 0)
        { PWM_Light_Ligth --;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 0){ PWM_Light_state = 1;}
        }
        // __HAL_TIM_SET_COMPARE(&htim3, TIM[j],0);

        else if( tick_elapsed(&PWM_Light_Time , 10) && PWM_Light_state == 1)
        { PWM_Light_Ligth++ ;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 50){ PWM_Light_state = 0;}
        }


        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin_PWM())
            {
                break;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_PWM())
            {
                break;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {   
                Frame[s_page]();
            }
        }

            }
        }

if(s_PWM == 4)
        {
            s2_place -= 1;
            s2_idx_place = (s2_place ) % 3;
            *s_LED[s2_idx_place]  = (*s_LED[s2_idx_place] + 1) % 2;

            for(int ii = 0; ii < 3 ; ii++)
            {
                if(*s_LED[ii])
                {
                HAL_TIM_PWM_Stop(&htim3 ,PWM_Light_CH[s1_place]);
                }
                else
                {
                HAL_TIM_PWM_Start(&htim3 ,PWM_Light_CH[s1_place]);
                }

            }
            s_PWM = 0;
            while(1)
            {
        if( tick_elapsed(&PWM_Light_Time , 10) && PWM_Light_state == 0)
        { PWM_Light_Ligth --;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 0){ PWM_Light_state = 1;}
        }
        // __HAL_TIM_SET_COMPARE(&htim3, TIM[j],0);

        else if( tick_elapsed(&PWM_Light_Time , 10) && PWM_Light_state == 1)
        { PWM_Light_Ligth++ ;
          __HAL_TIM_SET_COMPARE(&htim3, PWM_Light_CH[PWM_Light_Choose], PWM_Light_Ligth);
          if(PWM_Light_Ligth == 50){ PWM_Light_state = 0;}
        }

        
        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin_PWM())
            {
                break;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_PWM())
            {
                break;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {   
                return;
            }
        }
            }
        }

if(s_PWM == 2)
        {
            // int  PWM_Light_count = 0;
            s_PWM = 0;
            int lalssladl = 0;
            uint32_t left_long_Time = 0;
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);

            while(1)
            {
            if( tick_elapsed(&left_long_Time , (uint32_t) 700) )
            {
                lalssladl = (lalssladl + 1) % 3;
                if(lalssladl == 0)
                {
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 35);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 0);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , 35);
                }
                else if(lalssladl == 1)
                {
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 0);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 35);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , 35);
                }
                
                else if(lalssladl == 2)
                {
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 35);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 35);
            __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , 0);
                }
            }
        
            if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin_PWM())
            {
                break;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_PWM())
            {
                break;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {   
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
                return;
            }
        }
            }

        }

if(s_PWM == 3)
        {

            // int  PWM_Light_count = 0;
            s_PWM = 0;
            while(1)
            {
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 90);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 90);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , 90);

        HAL_Delay(21);

        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 0);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 0);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , 0);    
        
        HAL_Delay(21);
        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin_PWM())
            {
                break;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_PWM())
            {
                break;
                //i = 0;     已有
            }
        }

        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {   
                return;
            }
        }
            }
        }
        
        if( tick_elapsed(& key_Exittime, (uint32_t) 24) )
        {
            if(EXIT())
            {
            return;
            }
        }

    }
}

void Encoder_Light()
{
    uint32_t Encoder_Light_count = 0;
    uint32_t Encoder_Light_count_aaa = 0;
    uint32_t Encoder_Light_count_bbb = 0;
    uint32_t Encoder_Light_Time = 0;
    // uint32_t Encoder_Light_Refresh_idx = 0;
    uint32_t Encoder_Light_Refresh_Time = 0;
    key_Exittime = 0;
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);


    OLED_NewFrame();
    OLED_DrawImage( 30 , 0 ,&loookImg ,OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    HAL_Delay(300);

    
    // __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_ALL,i);

    while(1)
    {
        if( tick_elapsed(& Encoder_Light_Time, (uint32_t) 20) )
        {
        Encoder_Light_count = __HAL_TIM_GET_COUNTER(&htim2);

        if( Encoder_Light_count > 60000) 
        {
            Encoder_Light_count = 0;
            __HAL_TIM_SET_COUNTER(&htim2 , 0);
        }
            if( Encoder_Light_count > 50) 
        {
            Encoder_Light_count = 50;
            __HAL_TIM_SET_COUNTER(&htim2 , 50);
        }
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , Encoder_Light_count);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_2 , 50 - Encoder_Light_count);
        __HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_3 , Encoder_Light_count);
        }
        if(Encoder_Light_count > 25)
        {
            Encoder_Light_count_aaa = 25;
            Encoder_Light_count_bbb = Encoder_Light_count - 25;
        } 
        else
        {
            Encoder_Light_count_aaa = Encoder_Light_count;
            Encoder_Light_count_bbb = 0;
        }

    if( tick_elapsed(&Encoder_Light_Refresh_Time, (uint32_t) 24) )
    {


        OLED_NewFrame();
        OLED_DrawRectangle      (  30 - 24 , 40 , 25 , 10,OLED_COLOR_NORMAL);
        OLED_DrawRectangle      (  30 + 63 , 40 , 25 , 10,OLED_COLOR_NORMAL);
        OLED_DrawFilledRectangle(  30 - 24 , 40 , Encoder_Light_count_aaa ,10 , OLED_COLOR_NORMAL);
        OLED_DrawFilledRectangle(  30 + 63 , 40 , Encoder_Light_count_bbb ,10 , OLED_COLOR_NORMAL);
        OLED_DrawImage( 30 , 0 ,&loookImg ,OLED_COLOR_NORMAL);
        
        OLED_ShowFrame();
    }
        if( tick_elapsed(& key_Exittime, (uint32_t) 24) )
        {
            if(EXIT())
            {
            return;
            }
        }   
    }
}


void UART_Light()
{
    DMA_CH = 13;
    MX_USART1_UART_Init();
    HAL_TIM_PWM_Start(&htim3 , TIM_CHANNEL_ALL);
    char mess[20] = "Enter"; 

    HAL_TIM_PWM_Start(&htim3 ,TIM_CHANNEL_ALL);

    HAL_UART_Transmit_DMA(&huart1 , (uint8_t*)mess ,strlen(mess));

    HAL_UART_Receive_DMA(&huart1, (uint8_t*)UL ,strlen(UL));
}


//---------------------{二级 二号菜单实现预备}----------------------------------------
void UART_OLED( )
{
    MX_USART1_UART_Init();
    DMA_CH = 21;
    int UO_time = 0;
    char mess[10] = "输入";

    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)mess,strlen(mess) );

    HAL_UART_Receive_DMA(&huart1 , (uint8_t *)RX_buff , 17);
    while(1)
    {
    if (tick_elapsed((uint32_t *) UO_time , (uint32_t) 20) && OLED_ready)
        {OLED_ready = 0;
        OLED_NewFrame();
        OLED_PrintString(3, 18, message[message_num - 1], &font16x16, OLED_COLOR_NORMAL);
        OLED_ShowFrame(); 
        }  
    }
}


void OLED_UART()
{
    MX_USART1_UART_Init();
    OLED_NewFrame();
    OLED_PrintString(20 , 40 , "To Be Continue..." ,&font16x16 ,OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}
//To Be Continue...

void UART_Review ()
{
    // // static coco = 0;
    // int h = 0;
    // int coco = h + 4;
    // while(1)
    // {
    //     OLED_NewFrame();
    //     for(h ; h < coco ; h ++)
    //     {
    //         DMA_CH = 3;//先不管衔接问题
    //         OLED_PrintString(4 , 0 , message[h] , &font16x16, OLED_COLOR_NORMAL);
    //     }
    //     OLED_ShowFrame();

    //     if(tick_elapsed())
    //     {

    //     }
    // }

}
        //     if( DMA_CH == 3)
        //     {
        //         UART_OLED();
        //     }
        //  }

void UART_Slaver()
{






}
//上位机设定一路 PWM 的频率、占空比、占空比变化速率，并实现舵机角度的变化

//---------------------{二级 三号菜单实现预备}----------------------------------------

void Encoder_Slaver ()
{
    int Encoder_Slaver_Count = 0;
    int Slaver_Place = 0;
    int Encoder_Slaver_Refresh_idx = 0;
    uint32_t Encoder_Slaver_Refresh_Time = 0;
    uint32_t Encoder_Slaver_Time =  0;
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
    OLED_NewFrame();
    OLED_DrawImage( 30 , 0 ,&loookImg ,OLED_COLOR_NORMAL);
    OLED_ShowFrame();

    while(1)
    {        
        if( tick_elapsed(& Encoder_Slaver_Time, (uint32_t) 20) )
        {
        Encoder_Slaver_Count = __HAL_TIM_GET_COUNTER(&htim2);
        }
        if(Encoder_Slaver_Count != __HAL_TIM_GET_COUNTER(&htim2))
        {
            Encoder_Slaver_Refresh_idx = 1;
        }
        Encoder_Slaver_Count  = __HAL_TIM_GET_COUNTER(&htim2);

        if( Encoder_Slaver_Count > 60000) 
        {
            Encoder_Slaver_Count = 0;
            __HAL_TIM_SET_COUNTER(&htim2 , 0);
        }
            if( Encoder_Slaver_Count > 20) 
        {
            Encoder_Slaver_Count = 20;
            __HAL_TIM_SET_COUNTER(&htim2 , 20);
        }

        Slaver_Place = ((10 * Encoder_Slaver_Count) / (float)20 + 2.5 ) / 100.0 * 2000 ;
        __HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, Slaver_Place);


         if(Encoder_Slaver_Refresh_idx && tick_elapsed(&Encoder_Slaver_Refresh_Time, (uint32_t) 24) )
        {
        Encoder_Slaver_Refresh_idx = 0;

        OLED_NewFrame();
        OLED_DrawRectangle      (  30 - 24 , 40 , 25 , 10,OLED_COLOR_NORMAL);
        OLED_DrawFilledRectangle(  30 - 24 , 40 , Encoder_Slaver_Count * 25.0 / 25 ,10 , OLED_COLOR_NORMAL);
        OLED_DrawImage( 30 , 0 ,&loookImg ,OLED_COLOR_NORMAL);
        
        OLED_ShowFrame();
        }
    }

}


void ADC ()
{

}



//---------------------{二级四号菜单实现预备}----------------------------------------



void TIM_UP_Count ()
{
    uint32_t TIM_UP_Count_Time = 0;
    // static uint32_t day = 0;
    char message[20] = ""; 
    // uint32_t TIM_UP_Count_GO = 0;
    // static uint8_t TIM_Count_reverse = 1;

    key_Chosetime = 0;
    key_Exittime = 0;
    key_Selecttime = 0;
    static int TIM_UP_Count_State = 0;
    if(!TIM_UP_Count_State)
    {
            OLED_NewFrame();
            OLED_PrintString(0 , 32  , "按下按键开始计数" ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_ShowFrame();
    }
    else{
            OLED_NewFrame();
            sprintf(message , "%02u时%02u分%02u秒" ,*TIM_UP_Count_Arr[0],*TIM_UP_Count_Arr[1],*TIM_UP_Count_Arr[2] );
            OLED_PrintString(3 , 16  , message ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_PrintString(3 , 32  , "按下按键继续计时" ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_ShowFrame();
    }
    while(1)
    {
        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {
                TIM_UP_Count_Start = (TIM_UP_Count_Start + 1) % 2;
            }
        }

        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin())
            {
                TIM_UP_Count_Start = (TIM_UP_Count_Start + 1) % 2;
            }
        }
        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {
                break;
            }
        }
        if(!TIM_UP_Count_Start)
        {
            TIM_UP_Count_Time = HAL_GetTick();
        }

        if(TIM_UP_Count_Start && tick_elapsed_UnCompose(&TIM_UP_Count_Time , (uint32_t)1000))
        {//要可保存
    // static uint32_t * TIM_UP_Count_Arr[3] = {&U_hour , &U_min , &U_second};

           ( *TIM_UP_Count_Arr[2])++;
            // TIM_Count_reverse = (TIM_Count_reverse + 1) % 2;

        
        sprintf(message , "%02u时%02u分%02u秒" ,*TIM_UP_Count_Arr[0],*TIM_UP_Count_Arr[1],*TIM_UP_Count_Arr[2] );
        OLED_NewFrame();

        OLED_PrintString(3 , 32  , message ,&font16x16 , OLED_COLOR_NORMAL);
        // if(TIM_Count_reverse)
        // {
        // // OLED_DrawLine(3 + (32) * TIM_Count_Arr_Place, 32 + 17 , 3 + 16 + 32 * (TIM_Count_Arr_Place) , 32 + 17 ,OLED_COLOR_NORMAL);
        // }
        // OLED_DrawLine(3 + 16, 32 , 3 + 32 - 1, 32 ,OLED_COLOR_NORMAL);
        // OLED_DrawLine(3 + 32, 32 , 3 + 48 - 1, 32 ,OLED_COLOR_NORMAL);

        OLED_ShowFrame();
        if( *TIM_UP_Count_Arr[2] == 60)  {*TIM_UP_Count_Arr[1] += 1 ;*TIM_UP_Count_Arr[2] = 0;}
        if( *TIM_UP_Count_Arr[1] == 60)  {*TIM_UP_Count_Arr[0] += 1 ;*TIM_UP_Count_Arr[1] = 0;}
        TIM_UP_Count_State = 1;
        // if( second == 60)  {min += 1;second = 0;}
        // if( min == 60)     {hour += 1; min = 0;}
        // if( hour == 24)    {day += 1; hour = 0;}
        }
    
    }
}

void TIM_DOWN_Count ()
{
    // static uint32_t Chose = 0;
    static uint32_t time_DOWN_count = 0;
    static uint8_t TIM_Count_reverse = 1;
     TIM_Count_Finish = 0;
     int TIM_Count_OVER = 0;
    TIM_Count_Arr_Place = 0;
    char message[20] = ""; 

    static int TIM_DOWN_Count_State;
    if(!TIM_DOWN_Count_State)
    {
            OLED_NewFrame();
            sprintf(message , "%02u时%02u分%02u秒" ,*TIM_DOWN_Count_Arr[0],*TIM_DOWN_Count_Arr[1],*TIM_DOWN_Count_Arr[2] );
            OLED_PrintString(3 , 16  , message ,&font16x16 , OLED_COLOR_NORMAL);            
            OLED_PrintString(0 , 32  , "按下按键设置倒数" ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_ShowFrame();
    }
    else
    {
            OLED_NewFrame();
            sprintf(message , "%02u时%02u分%02u秒" ,*TIM_DOWN_Count_Arr[0],*TIM_DOWN_Count_Arr[1],*TIM_DOWN_Count_Arr[2] );
            OLED_PrintString(3 , 16  , message ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_PrintString(3 , 32  , "按下按键继续计时" ,&font16x16 , OLED_COLOR_NORMAL);
            OLED_ShowFrame();
    }


    while(1)
    {
        if( tick_elapsed(&key_Chosetime , (uint32_t) 20) )
        {
            if(READPin())
            {if(!TIM_Count_Finish)
                {
            (*TIM_DOWN_Count_Arr[ TIM_Count_Arr_Place] )++;
                }
             if(TIM_Count_Finish)
             {
                TIM_DOWN_Count_Start = (TIM_DOWN_Count_Start + 1) % 2;

             }
            }
        }
        if( tick_elapsed(&key_Selecttime , (uint32_t) 20) )
        {
            if(READPinPin_TIM_Count())
            {
            if(TIM_Count_Arr_Place < 3)
            {
                TIM_Count_Arr_Place++;
            }
            else if(TIM_Count_Arr_Place == 3)
            {
                TIM_Count_Arr_Place = (TIM_Count_Arr_Place + 1) % 3;
                if (TIM_Count_Arr_Place == 0)
                {
                TIM_Count_Finish = 1;
                TIM_DOWN_Count_Start = (TIM_DOWN_Count_Start + 1) % 2;
                }
            }
            }
        }
        if( tick_elapsed(&key_Exittime , (uint32_t) 20) )
        {
            if(EXIT())
            {
                break;
            }
        }


        // if(tick_elapsed(&Chose , 20))
        // {
        //     // ReadPin_4_1();
        // }
    if(!TIM_DOWN_Count_Start)
    {
        time_DOWN_count = HAL_GetTick();
        OLED_NewFrame();
        TIM_Count_reverse = (TIM_Count_reverse + 1) % 2;
        if(!TIM_DOWN_Count_Start)//编写时闪灭
        {
        sprintf(message , "%02u时%02u分%02u秒" ,*TIM_DOWN_Count_Arr[0], *TIM_DOWN_Count_Arr[1] ,*TIM_DOWN_Count_Arr[2] );
        OLED_PrintString(3 , 16, message ,&font16x16 , OLED_COLOR_NORMAL);
            if(TIM_Count_reverse)
            {

            if(TIM_Count_Arr_Place == 0)
            {OLED_DrawLine(3     , 32 + 1 , 3 + 16 - 1, 32 + 1 ,OLED_COLOR_NORMAL);}
            else if(TIM_Count_Arr_Place == 1)
            {OLED_DrawLine(3 + 16, 32 + 1 , 3 + 32 - 1, 32 + 1 ,OLED_COLOR_NORMAL);}
            else if(TIM_Count_Arr_Place == 2)
            {OLED_DrawLine(3 + 32, 32 + 1 , 3 + 48 - 1, 32 + 1 ,OLED_COLOR_NORMAL);}
            }
        }
        OLED_ShowFrame();
    }

    if(!TIM_Count_OVER && tick_elapsed_UnCompose(&time_DOWN_count , 1000))
    {
        TIM_DOWN_Count_State = 1;
        sprintf(message , "%02u时%02u分%02u秒" ,*TIM_DOWN_Count_Arr[0], *TIM_DOWN_Count_Arr[1] ,*TIM_DOWN_Count_Arr[2] );
        OLED_NewFrame();

        OLED_PrintString(3 , 16, message ,&font16x16 , OLED_COLOR_NORMAL);
        OLED_ShowFrame();


        if (*TIM_DOWN_Count_Arr[2] > 0) {*(TIM_DOWN_Count_Arr[2] )--;}
        if(*TIM_DOWN_Count_Arr[2] == 0 || *TIM_DOWN_Count_Arr[2] > 100)//计数的计算
        {
            if(*TIM_DOWN_Count_Arr[1] != 0)
            {
                *TIM_DOWN_Count_Arr[1] --;
                *TIM_DOWN_Count_Arr[2] = 60;
            }
            else if(*TIM_DOWN_Count_Arr[1] == 0)
            {
                if(*TIM_DOWN_Count_Arr[0] != 0)
                {
                    *TIM_DOWN_Count_Arr[0] --;
                    *TIM_DOWN_Count_Arr[1] = 60;
                }
                else if(*TIM_DOWN_Count_Arr[0] == 0)
                {
                    TIM_Count_OVER = 1;
                    
                    
                    //可以加个闪烁
                }
            }
        }
    }
        if(TIM_Count_OVER)
        {
            OLED_NewFrame();

            char mess[30] = "//TIMES OUT //";
            OLED_PrintString(4 ,48 ,mess ,&font16x16 ,OLED_COLOR_NORMAL);
            OLED_ShowFrame();
        }
    

    }
}


void TIM_MPU ()
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

