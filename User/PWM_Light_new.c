/**
  ******************************************************************************
  * @file    PWM_Light_new.c
  * @brief   PWM Light 参考实现（独立编译单元，不修改 EX_oled.c 任何一行）
  *
  * 【怎么用】
  *   方式 A（推荐，先对比）：把本文件加入 Keil 工程 → 在 EX_oled.c 的
  *           OLED_Frame_1_1 里把 function[0][i]() 里 i==0 那条临时改成
  *           调 PWM_Light_v2()，两个版本可以来回切着看。
  *   方式 B（彻底替换）：把本文件里的 PWM_Light_v2 改名为 PWM_Light，
  *           并删掉 EX_oled.c 里旧的 PWM_Light() 整个函数（633~828 行）。
  *
  * 【本版修掉了旧版的哪些问题】
  *   1. __HAL_TIM_SET_COMPARE 第二个参数传的是函数指针数组 TIM[]，不是通道号；
  *      三元宏判定不匹配后静默写进 CCR4，CH1 永远不动。
  *      → 本版用 PWM_CH[c]，类型是 uint32_t，且只有 CH1~CH3。
  *   2. tick_elapsed(&t,10) && state==0 的短路顺序错误，上升段永远跑不到。
  *      → 本版先推进亮度、再统一写 CCR，不再有"两个分支抢同一个时间戳"。
  *   3. 内层 while break 出来到外层 while(1) 后没有任何按键轮询 → 死机。
  *      → 本版只有一层 while(1)，按键扫描和呼吸都挂在同一个循环里。
  *   4. 子函数里回调 Frame[s_page]() 造成递归，1KB 栈会溢出。
  *      → 本版按 K3 直接 return，回到菜单层由上一级重绘。
  *   5. for 循环里用 s1_place 代替 ii；s2_place 恒定导致索引恒为 0 且可能 -1 越界。
  *      → 本版选择逻辑只有 PWM_Select(dir)，取模先加通道数，永不越界。
  *   6. 长短按没有真正测时长（HAL_Delay + while 阻塞等松手）。
  *      → 本版是标准的"按下记时刻 / 松开算差值 / 按住超时触发长按"状态机。
  *   7. 呼吸 + 按键代码被抄了三份。
  *      → 本版只有一份 PWM_Task() 和一份 Key_Scan()。
  ******************************************************************************
  */

#include "EX_oled.h"
#include "oled.h"
#include "font.h"
#include "tim.h"
#include "main.h"
#include <stdio.h>

/* ============================ 可调参数 ============================ */

#define PWM_CH_NUM        3U      /* 用几路：TIM3_CH1/CH2/CH3 */
#define PWM_FULL          100U    /* 100% 占空比 = TIM3 的 ARR+1（PSC=71,ARR=99 → 10kHz）*/
#define BREATH_STEP       1U      /* 每步变化量，1 → 从 0 到 100 约 1 秒 */
#define BREATH_TICK_MS    10U     /* 亮度刷新周期 */

#define KEY_SCAN_MS       10U     /* 按键扫描周期 */
#define KEY_DEBOUNCE_MS   20U     /* 去抖时间：电平稳定这么久才认 */
#define KEY_LONG_MS       600U    /* 按住超过这个时间算长按 */

/* 按键有效电平：
 *   0 = 按下时读到低电平（按键一端接 GND，另一端靠上拉）
 *   1 = 按下时读到高电平（按键一端接 VCC，另一端靠下拉）
 *
 * ⚠️ 注意：本工程 Core/Src/gpio.c 里 KEY1/2/3 配的是 GPIO_PULLDOWN，
 *    但旧代码全部用 !HAL_GPIO_ReadPin() 判断"按下"。
 *    两者只在你按键模块自带外部上拉（或板上已有上拉电阻）时才自洽。
 *    上电后先在调试器里看一眼没按键时 ReadPin 到底是 0 还是 1，
 *    如果没按键读到 0，说明内部下拉把线拉死了，要在 CubeMX 里改成 Pull-up。
 *    这里默认保持和旧代码相同的语义（低电平有效），先保证行为不变。
 */
#define KEY_ACTIVE_LEVEL  0U

/* 通道号表：一定要是 TIM_CHANNEL_x（0x00/0x04/0x08），不能是函数指针！ */
static const uint32_t PWM_CH[PWM_CH_NUM] = {
    TIM_CHANNEL_1,
    TIM_CHANNEL_2,
    TIM_CHANNEL_3,
};

/* ============================ 运行状态 ============================ */

static uint8_t  s_led_on[PWM_CH_NUM];   /* 1 = 该路参与呼吸 */
static int      s_sel;                  /* 当前选中的通道 0..2 */
static uint32_t s_lig;                  /* 当前亮度 0..PWM_FULL */
static uint8_t  s_dir;                  /* 0 = 变亮，1 = 变暗 */
static uint32_t s_breath_tick;

/* ============================ 按键层 ============================ */

#define KEY_EV_NONE   0U
#define KEY_EV_SHORT  1U
#define KEY_EV_LONG   2U

#define EV_NONE       0U
#define EV_K1_SHORT   1U
#define EV_K1_LONG    2U
#define EV_K2_SHORT   3U
#define EV_K2_LONG    4U
#define EV_BOTH       5U
#define EV_EXIT       6U

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t       raw;         /* 上次原始采样（1=按下）*/
    uint8_t       stable;      /* 去抖后电平（1=按下）*/
    uint32_t      t_change;    /* 原始电平变化时刻 */
    uint32_t      t_press;     /* 稳定按下时刻；0 表示这次按下已被吞掉 */
    uint8_t       long_fired;  /* 本次按住是否已经报过长按 */
} Key_t;

static Key_t s_k1 = { KEY1_GPIO_Port, KEY1_Pin, 0, 0, 0, 0, 0 };
static Key_t s_k2 = { KEY2_GPIO_Port, KEY2_Pin, 0, 0, 0, 0, 0 };
static Key_t s_k3 = { KEY3_GPIO_Port, KEY3_Pin, 0, 0, 0, 0, 0 };
static uint8_t s_combo_latch = 0;

static uint8_t Key_Raw(Key_t *k)
{
    uint8_t v = (HAL_GPIO_ReadPin(k->port, k->pin) != GPIO_PIN_RESET) ? 1U : 0U;
    /* KEY_ACTIVE_LEVEL==1 时按下=1，直接用；否则按下=0，取反 */
    return KEY_ACTIVE_LEVEL ? v : (uint8_t)(!v);
}

/* 每 KEY_SCAN_MS 调一次。返回值：KEY_EV_NONE / SHORT / LONG（长按在按住期间只报一次）*/
static uint8_t Key_Poll(Key_t *k, uint32_t now)
{
    uint8_t r = Key_Raw(k);

    /* 1. 原始电平变了 → 重新开始去抖计时 */
    if (r != k->raw) {
        k->raw = r;
        k->t_change = now;
    }

    /* 2. 稳定够久了，才承认电平真的变了 */
    if ((now - k->t_change) >= KEY_DEBOUNCE_MS && k->stable != r) {
        k->stable = r;
        if (r) {                        /* 按下沿 */
            k->t_press    = now;
            k->long_fired = 0;
        } else {                        /* 松开沿：这才是"短按"的判定点 */
            if (k->t_press != 0 && !k->long_fired &&
                (now - k->t_press) < KEY_LONG_MS) {
                k->t_press = 0;
                return KEY_EV_SHORT;
            }
            k->t_press = 0;
        }
    }

    /* 3. 还按着，且超过了长按阈值 → 报一次长按 */
    if (k->stable && !k->long_fired && k->t_press != 0 &&
        (now - k->t_press) >= KEY_LONG_MS) {
        k->long_fired = 1;
        return KEY_EV_LONG;
    }

    return KEY_EV_NONE;
}

static uint8_t Key_Scan(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t  e1  = Key_Poll(&s_k1, now);
    uint8_t  e2  = Key_Poll(&s_k2, now);
    uint8_t  e3  = Key_Poll(&s_k3, now);

    /* 退出键最优先 */
    if (e3 == KEY_EV_SHORT) return EV_EXIT;

    /* K1+K2 同时按住 = 组合键，先于单键判定；并吞掉这次按下的长短按 */
    if (s_k1.stable && s_k2.stable) {
        if (!s_combo_latch) {
            s_combo_latch = 1;
            s_k1.long_fired = s_k2.long_fired = 1;
            s_k1.t_press    = s_k2.t_press    = 0;
            return EV_BOTH;
        }
        return EV_NONE;
    }
    if (!s_k1.stable && !s_k2.stable) {
        s_combo_latch = 0;
    }

    if (e1 == KEY_EV_LONG)  return EV_K1_LONG;
    if (e2 == KEY_EV_LONG)  return EV_K2_LONG;
    if (e1 == KEY_EV_SHORT) return EV_K1_SHORT;
    if (e2 == KEY_EV_SHORT) return EV_K2_SHORT;
    return EV_NONE;
}

/* ============================ 逻辑层 ============================ */

/* dir = +1 向右选，-1 向左选；选中即翻转该路开关 */
static void PWM_Select(int dir)
{
    s_sel = (s_sel + (int)PWM_CH_NUM + dir) % (int)PWM_CH_NUM;   /* 先加模数，避免负数 */
    s_led_on[s_sel] ^= 1U;
}

static void PWM_SetAll(uint8_t on)
{
    for (uint8_t c = 0; c < PWM_CH_NUM; c++) {
        s_led_on[c] = on;
    }
}

/* 统一呼吸：先推进亮度，再把所有通道的 CCR 一次写完 */
static void PWM_Task(void)
{
    if (!tick_elapsed(&s_breath_tick, BREATH_TICK_MS)) return;

    if (s_dir == 0) {                       /* 变亮 */
        s_lig += BREATH_STEP;
        if (s_lig >= PWM_FULL) { s_lig = PWM_FULL; s_dir = 1; }
    } else {                                /* 变暗 */
        if (s_lig >= BREATH_STEP) s_lig -= BREATH_STEP;
        else                      s_lig = 0;
        if (s_lig == 0) s_dir = 0;
    }

    for (uint8_t c = 0; c < PWM_CH_NUM; c++) {
        /* 关掉的那路写 0，不需要 Stop（Write CCR 比 Start/Stop 更安全，
         * 也不会碰到 CCER 使能位）*/
        __HAL_TIM_SET_COMPARE(&htim3, PWM_CH[c], s_led_on[c] ? s_lig : 0U);
    }
}

static void PWM_Enter(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    for (uint8_t c = 0; c < PWM_CH_NUM; c++) {
        __HAL_TIM_SET_COMPARE(&htim3, PWM_CH[c], 0U);
        s_led_on[c] = 0;
    }

    s_sel = 0;
    s_lig = 0;
    s_dir = 0;
    s_breath_tick = HAL_GetTick();      /* 别让第一次进来就立刻跳一大步 */

    /* 清一次按键状态，避免上一层的按键残留被当成新事件 */
    s_k1.t_press = s_k2.t_press = s_k3.t_press = 0;
    s_k1.long_fired = s_k2.long_fired = s_k3.long_fired = 0;
    s_combo_latch = 0;
}

static void PWM_Exit(void)
{
    for (uint8_t c = 0; c < PWM_CH_NUM; c++) {
        __HAL_TIM_SET_COMPARE(&htim3, PWM_CH[c], 0U);
        s_led_on[c] = 0;
    }
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}

/* ============================ 显示层 ============================ */

static void UI_Draw(void)
{
    char buf[20];

    OLED_NewFrame();

    sprintf(buf, "PWM LIGHT");
    OLED_PrintString(32, 0, buf, &font16x16, OLED_COLOR_NORMAL);

    /* > 表示当前选中，+ 表示这路亮着参与呼吸，- 表示灭 */
    sprintf(buf, "%c1%c %c2%c %c3%c",
            (s_sel == 0) ? '>' : ' ', s_led_on[0] ? '+' : '-',
            (s_sel == 1) ? '>' : ' ', s_led_on[1] ? '+' : '-',
            (s_sel == 2) ? '>' : ' ', s_led_on[2] ? '+' : '-');
    OLED_PrintString(12, 20, buf, &font16x16, OLED_COLOR_NORMAL);

    sprintf(buf, "K1<  K2>  K3:OUT");
    OLED_PrintString(0, 38, buf, &font16x16, OLED_COLOR_NORMAL);

    sprintf(buf, "HOLD:ALL");
    OLED_PrintString(28, 52, buf, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

/* ============================ 对外入口 ============================ */

/**
 * @brief PWM 呼吸灯 + 按键选路
 * @note  非阻塞：整个函数只有一层 while(1)，靠 tick 驱动，
 *        按 K3 短按即返回上一级菜单（由菜单函数重新绘制，不再回调父级）。
 */
void PWM_Light_v2(void)
{
    uint32_t t_scan = 0;
    uint8_t  ev;

    PWM_Enter();
    UI_Draw();

    while (1) {

        /* --- 1. 按键：每 10ms 扫一次 --- */
        if (tick_elapsed(&t_scan, KEY_SCAN_MS)) {

            ev = Key_Scan();
            switch (ev) {
                case EV_K1_SHORT: PWM_Select(+1);  break;   /* 向右选 + 翻转 */
                case EV_K2_SHORT: PWM_Select(-1);  break;   /* 向左选 + 翻转 */
                case EV_K1_LONG:  PWM_SetAll(1);   break;   /* 长按 K1：全开 */
                case EV_K2_LONG:  PWM_SetAll(0);   break;   /* 长按 K2：全关 */
                case EV_BOTH:     PWM_SetAll(0); s_sel = 0; break;  /* 双键：复位 */
                case EV_EXIT:                                   /* 退出 */
                    PWM_Exit();
                    return;                                     /* ← 关键：直接返回 */
                default: break;
            }
            if (ev != EV_NONE) UI_Draw();
        }

        /* --- 2. 呼吸：非阻塞，自己管自己的时间戳 --- */
        PWM_Task();
    }
}
