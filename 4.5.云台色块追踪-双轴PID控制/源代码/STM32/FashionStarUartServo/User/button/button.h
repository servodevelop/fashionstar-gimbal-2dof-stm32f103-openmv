/*
 * 按键驱动
 * Author: Kyle
 */
 
#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"
#include "gimbal.h"
#include "sys_tick.h"

// PB0: 按键-偏航角增加
#define BUTTON_YAW_ADD_PORT GPIOB 
#define BUTTON_YAW_ADD_PIN GPIO_Pin_4
// PB1: 按键-偏航角减少
#define BUTTON_YAW_MINUS_PORT GPIOB
#define BUTTON_YAW_MINUS_PIN GPIO_Pin_5
// PB5: 按键-俯仰角增加
#define BUTTON_PITCH_ADD_PORT GPIOB
#define BUTTON_PITCH_ADD_PIN GPIO_Pin_6
// PB4: 按键-俯仰角减少
#define BUTTON_PITCH_MINUS_PORT GPIOB
#define BUTTON_PITCH_MINUS_PIN GPIO_Pin_7

#define BUTTON_NUM 4 // 按键个数

// 按键序号
#define BUTTON_ID_YAW_ADD 0
#define BUTTON_ID_YAW_MINUS 1
#define BUTTON_ID_PITCH_ADD 2
#define BUTTON_ID_PITCH_MINUS 3

// 按键按下每次角度的增加幅度
#define BUTTON_ANGLE_STEP 5

 
// 按键中断初始化
void Button_NVIC_Config(void);
// 按键GPIO配置
void Button_GPIO(void);
// 按键中断配置
void Button_EXTI(void);
// 按键初始化
void Button_Init(void);

#endif
