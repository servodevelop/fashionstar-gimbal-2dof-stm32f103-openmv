#include "gimbal.h"

// 偏航角与舵机角度 线性映射
// servo = K_yaw2srv * yaw + b_yaw2srv
float K_yaw2srv; // 偏航角转换为舵机角度的比例系数
float b_yaw2srv; // 偏航角转换为舵机角度的偏移量
// yaw = K_srv2yaw * servo + b_srv2yaw 
float K_srv2yaw; // 舵机角度转换为偏航角的比例系数
float b_srv2yaw; // 舵机角度转换为偏航角的偏移量
// 俯仰角与舵机角度 (线性映射)
// servo = K_pitch2srv * pitch + b_yaw2srv
float K_pitch2srv; // 俯仰角转换为舵机角度的比例系数
float b_pitch2srv; // 俯仰角转换为舵机角度的偏移量
// pitch = K_srv2pitch * servo + b_srv2pitch
float K_srv2pitch; // 舵机角度转换为俯仰角的比例系数
float b_srv2pitch; // 舵机角度转换为俯仰角的偏移量 

// 云台位姿状态
float curSrvYaw = 0; 	// 偏航角的原始舵机当前角度 (单位 °)
float curSrvPitch = 0; 	// 俯仰角的原始舵机当前角度 (单位 °)
float nextSrvYaw = 0; 	// 偏航角的原始舵机目标角度 (单位 °)
float nextSrvPitch = 0; // 俯仰角的原始舵机目标角度 (单位 °)
float curYaw = 0; 		// 云台当前的偏航角 (单位 °)
float curPitch = 0; 	// 云台当前的俯仰角 (单位 °)
float nextYaw = 0; 		// 云台目标的俯仰角 (单位 °)
float nextPitch = 0; 	// 云台目标的偏航角 (单位 °)
float speedYaw = 0; 	// 偏航角转速 (单位 °/s)
float speedPitch = 0; 	// 俯仰角转速 (单位 °/s)

// 云台初始化
void Gimbal_Init(Usart_DataTypeDef* servoUsart){
	
	// 生成舵机角度映射相关的系数
	Gimbal_GenSrvMapParams();
	// 舵机云台重置
	Gimbal_Reset(servoUsart);
	// 更新位姿
	Gimbal_Update(servoUsart);
}

// 重置舵机云台
void Gimbal_Reset(Usart_DataTypeDef* servoUsart){
	uint16_t interval1, interval2;
	// 设置为初始角度
	interval1 = Gimbal_SetYaw(servoUsart, YAW_INIT, YAW_SPEED_INIT);
	interval2 = Gimbal_SetPitch(servoUsart, PITCH_INIT, PITCH_SPEED_INIT);
	// 延时等待云台复位
	interval1 = interval1 > interval2 ? interval1 : interval2;
	SysTick_DelayMs(interval1);
}

// 生成舵机角度映射相关的系数
void Gimbal_GenSrvMapParams(void){
	K_yaw2srv = (YAW1_SERVO_ANGLE - YAW2_SERVO_ANGLE) / (YAW1 - YAW2);
	b_yaw2srv = YAW1_SERVO_ANGLE - K_yaw2srv * YAW1;
	K_srv2yaw = (YAW1 - YAW2) / (YAW1_SERVO_ANGLE - YAW2_SERVO_ANGLE);
	b_srv2yaw =  YAW1 - K_srv2yaw *  YAW1_SERVO_ANGLE;
	
	K_pitch2srv = (PITCH1_SERVO_ANGLE - PITCH2_SERVO_ANGLE) / (PITCH1 - PITCH2);
	b_pitch2srv = PITCH1_SERVO_ANGLE - K_pitch2srv * PITCH1;
	K_srv2pitch = (PITCH1 - PITCH2) / (PITCH1_SERVO_ANGLE - PITCH2_SERVO_ANGLE);
	b_srv2pitch = PITCH1 - K_srv2pitch * PITCH1_SERVO_ANGLE;	
}

// 更新偏航角
void Gimbal_UpdateYaw(Usart_DataTypeDef* servoUsart){
	FSUS_QueryServoAngle(servoUsart, SERVO_ID_YAW, &curSrvYaw);
	curYaw = Gimbal_Servo2Yaw(curSrvYaw);
}

// 更新俯仰角
void Gimbal_UpdatePitch(Usart_DataTypeDef* servoUsart){
	FSUS_QueryServoAngle(servoUsart, SERVO_ID_PITCH, &curSrvPitch);
	curPitch = Gimbal_Servo2Pitch(curSrvPitch);
}

// 更新舵机云台位姿
void Gimbal_Update(Usart_DataTypeDef* servoUsart){
	Gimbal_UpdateYaw(servoUsart);
	Gimbal_UpdatePitch(servoUsart);
}

// 偏航角转换为舵机角度
float Gimbal_Yaw2Servo(float yaw){
	return K_yaw2srv * yaw + b_yaw2srv;
}

// 舵机角度转换为偏航角
float Gimbal_Servo2Yaw(float servo){
	return K_srv2yaw * servo + b_srv2yaw;
}
	
// 俯仰角转换为舵机角度
float Gimbal_Pitch2Servo(float pitch){
	return K_pitch2srv * pitch + b_pitch2srv;
}

// 舵机角度转换为俯仰角
float Gimbal_Servo2Pitch(float servo){
	return K_srv2pitch * servo + b_srv2pitch;
}

// 设置云台的偏航角
uint16_t Gimbal_SetYaw(Usart_DataTypeDef* servoUsart, float yaw, float speed){
	uint16_t interval;
	// 更新角度
	Gimbal_UpdateYaw(servoUsart);
	// 边界检测
	yaw = (yaw < YAW_MIN) ? YAW_MIN: yaw;
	yaw = (yaw > YAW_MAX) ? YAW_MAX: yaw;
	// 目标角度
	nextYaw = yaw;
	nextSrvYaw = Gimbal_Yaw2Servo(yaw);
	// 控制舵机旋转
	interval = (uint16_t)__fabs(1000 * (curYaw - nextYaw) / speed); // 时间间隔单位ms
	FSUS_SetServoAngle(servoUsart, SERVO_ID_YAW, nextSrvYaw, interval, 0, 0);
	return interval;
}

// 设置云台的俯仰角
uint16_t Gimbal_SetPitch(Usart_DataTypeDef* servoUsart, float pitch, float speed){
	uint16_t interval;
	// 更新角度
	Gimbal_UpdatePitch(servoUsart);
	// 边界检测
	pitch = (pitch < PITCH_MIN) ? PITCH_MIN : pitch;
	pitch = (pitch > PITCH_MAX) ? PITCH_MAX : pitch;
	// 目标角度
	nextPitch = pitch;
	nextSrvPitch = Gimbal_Pitch2Servo(pitch);
	// 控制舵机旋转
	interval = (uint16_t)__fabs(1000 *(curPitch - nextPitch) / speed);
	FSUS_SetServoAngle(servoUsart, SERVO_ID_PITCH, nextSrvPitch, interval, 0, 0);
	return interval;
}

// 等待舵机旋转到特定的位置
void Gimbal_Wait(Usart_DataTypeDef* servoUsart){
	SysTick_DelayMs(10);
	while(1){
		// 更新角度
		Gimbal_Update(servoUsart);
		if (__fabs(curPitch - nextPitch) < SERVO_DEAD_BLOCK && __fabs(curYaw - nextYaw) < SERVO_DEAD_BLOCK){
			break;
		}
	}
}
