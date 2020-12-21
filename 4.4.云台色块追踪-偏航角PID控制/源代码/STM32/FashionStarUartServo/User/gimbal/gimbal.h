/*
 * 2DoF舵机云台驱动
 * Author: Kyle
 * UpdateTime: 2019/12/13
 */
 
#ifndef __GIMBAL_H
#define __GIMBAL_H

#include "stm32f10x.h"
#include "usart.h"
#include "fashion_star_uart_servo.h"
#include "sys_tick.h"

// 云台驱动器相关设置
#define SERVO_ID_YAW 0 // 偏航角的舵机ID
#define SERVO_ID_PITCH 1 // 俯仰角的舵机ID
#define SERVO_DEAD_BLOCK 2.0 // 舵机死区

// 云台舵机的标定数据
#define YAW1 90.0
#define YAW1_SERVO_ANGLE -90.4
#define YAW2 -90.0
#define YAW2_SERVO_ANGLE 95.1

#define PITCH1 60.0
#define PITCH1_SERVO_ANGLE -66.2
#define PITCH2 -90.0
#define PITCH2_SERVO_ANGLE 86.3

// 云台的角度范围
#define YAW_MIN -90
#define YAW_MAX 90
#define PITCH_MIN -90
#define PITCH_MAX 60

// 云台的初始位姿
#define YAW_INIT 0.0
#define YAW_SPEED_INIT 100.0
#define PITCH_INIT -45.0
#define PITCH_SPEED_INIT 100.0

// 偏航角与舵机角度 线性映射
// servo = K_yaw2srv * yaw + b_yaw2srv
extern float K_yaw2srv; // 偏航角转换为舵机角度的比例系数
extern float b_yaw2srv; // 偏航角转换为舵机角度的偏移量
// yaw = K_srv2yaw * servo + b_srv2yaw 
extern float K_srv2yaw; // 舵机角度转换为偏航角的比例系数
extern float b_srv2yaw; // 舵机角度转换为偏航角的偏移量
// 俯仰角与舵机角度 (线性映射)
// servo = K_pitch2srv * pitch + b_yaw2srv
extern float K_pitch2srv; // 俯仰角转换为舵机角度的比例系数
extern float b_pitch2srv; // 俯仰角转换为舵机角度的偏移量
// pitch = K_srv2pitch * servo + b_srv2pitch
extern float K_srv2pitch; // 舵机角度转换为俯仰角的比例系数
extern float b_srv2pitch; // 舵机角度转换为俯仰角的偏移量 

// 云台位姿状态
extern float curSrvYaw; // 偏航角的原始舵机当前角度 (单位 °)
extern float curSrvPitch; // 俯仰角的原始舵机当前角度 (单位 °)
extern float nextSrvYaw; // 偏航角的原始舵机目标角度 (单位 °)
extern float nextSrvPitch; // 俯仰角的原始舵机目标角度 (单位 °)
extern float curYaw; // 云台当前的偏航角 (单位 °)
extern float curPitch; // 云台当前的俯仰角 (单位 °)
extern float nextYaw; // 云台目标的俯仰角 (单位 °)
extern float nextPitch; // 云台目标的偏航角 (单位 °)
extern float speedYaw; // 偏航角转速 (单位 °/s)
extern float speedPitch; // 俯仰角转速 (单位 °/s)


// 云台初始化
void Gimbal_Init(Usart_DataTypeDef* servoUsart);
// 生成舵机角度映射相关的系数
void Gimbal_GenSrvMapParams(void);
// 重置舵机云台
void Gimbal_Reset(Usart_DataTypeDef* servoUsart);
// 更新偏航角
void Gimbal_UpdateYaw(Usart_DataTypeDef* servoUsart);
// 更新俯仰角
void Gimbal_UpdatePitch(Usart_DataTypeDef* servoUsart);
// 更新舵机云台位姿
void Gimbal_Update(Usart_DataTypeDef* servoUsart);
// 偏航角转换为舵机角度
float Gimbal_Yaw2Servo(float yaw);
// 舵机角度转换为偏航角
float Gimbal_Servo2Yaw(float servo);
// 俯仰角转换为舵机角度
float Gimbal_Pitch2Servo(float pitch);
// 舵机角度转换为俯仰角
float Gimbal_Servo2Pitch(float servo);
// 设置云台的偏航角
uint16_t Gimbal_SetYaw(Usart_DataTypeDef* servoUsart, float yaw, float speed);
// 设置云台的俯仰角
uint16_t Gimbal_SetPitch(Usart_DataTypeDef* servoUsart, float pitch, float speed);
// 等待舵机旋转到特定的位置
void Gimbal_Wait(Usart_DataTypeDef* servoUsart);
#endif
