/***************************************************
* 舵机云台激光打点
 ***************************************************/
#include "stm32f10x.h"
#include "math.h"
#include "usart.h"
#include "sys_tick.h"
#include "fashion_star_uart_servo.h"
#include "gimbal.h"

#define PI 3.1415926
#define RAD2DEG 180 / PI // 弧度转换为角度的比例系数
#define GIMBAL_SERVO_SPEED 200.0 // 云台旋转速度 (单位: °/s)

// 机械结构参数
#define OQ 25.0 // 云台(Gimbal)坐标系原点距离纸板(Board)坐标系原点的直线距离
#define OL 5.4 // 云台坐标系原点距离激光轴心(Laser)的距离

// 纸板网格参数
#define GRID_WIDTH 1.916 // 网格的宽度 (单位cm) 以实际测量为准
#define GRID_COLUMN  12 // 网格的列数
#define GRID_ROW 	 8 	// 网格的行数

// 偏移量
#define XB_OFFSET 0
#define YB_OFFSET 0

// 使用串口1作为舵机控制的端口
// <接线说明>
// STM32F103 PA9(Tx)  <----> 串口舵机转接板 Rx
// STM32F103 PA10(Rx) <----> 串口舵机转接板 Tx
// STM32F103 GND 	  <----> 串口舵机转接板 GND
// STM32F103 V5 	  <----> 串口舵机转接板 5V
// <注意事项>
// 使用前确保已设置usart.h里面的USART1_ENABLE为1
// 设置完成之后, 将下行取消注释
Usart_DataTypeDef* servoUsart = &usart1; 

/* 纸板(Board: [xb, yb])坐标系转化为全局坐标系(Global: [xg, yg, zg])*/
void TF_Board2Global(float xb, float yb, float *xg, float *yg, float *zg){
	*xg = OQ;
	*yg = -xb;
	*zg = yb;
}

/* 全局坐标系转换为纸板坐标系*/
void TF_Global2Board(float xg, float yg, float zg, float *xb, float *yb){
	*xb = -yg;
	*yb = zg;
}

/* 云台正向运动学*/
void FK_GimbalLaser(float alpha, float beta, float *xb, float *yb){
	float xg, yg, zg;
	xg = OQ;
	yg = xg * tan(alpha);
	zg = OL*cos(beta) - (xg - OL *sin(beta))*tan(beta);
	TF_Global2Board(xg, yg, zg, xb, yb);
}

/* 云台逆向运动学 */
void IK_GimbalLaser(float xg, float yg, float zg, float *alpha, float *beta){
	// 计算偏航角 Yaw(单位角度) = alpha
	*alpha = (atan2(yg, xg)) * RAD2DEG;
	// 计算俯仰角 Pitch(单位角度) = beta
	float theta = atan2(zg, xg);
	float OP3 = sqrt(pow(xg, 2) + pow(zg, 2));
	float lambda = acos(OL / OP3);
	*beta = (PI/2 - (theta + lambda))*RAD2DEG;
}

/* 移动云台让激光打在纸板的特定位置上 */
void MoveTo(float xb, float yb){
	float xg, yg, zg; // 纸板上的目标点在云台坐标系下的位置
	float alpha;	// 云台偏航角
	float beta; 	// 云台俯仰角
	
	// 添加偏移量
	xb += XB_OFFSET;
	yb += YB_OFFSET;
	
	// 纸板坐标系转换为云台坐标系
	TF_Board2Global(xb, yb, &xg, &yg, &zg);
	// 云台逆向运动学
	IK_GimbalLaser(xg, yg, zg, &alpha, &beta);
	
	// 设置云台选择到目标位姿
	Gimbal_SetYaw(servoUsart, alpha, GIMBAL_SERVO_SPEED);
	Gimbal_SetPitch(servoUsart, beta, GIMBAL_SERVO_SPEED);
	
	// 等待云台旋转到目标位置
	Gimbal_Wait(servoUsart);
}

/* 移动到特定的网格位置上 */
void MoveToGrid(int8_t xi, int8_t yi){
	MoveTo(xi*GRID_WIDTH, yi*GRID_WIDTH);
}

/* 测试垂直方向打点 */
void TestVertical(){
	for(int8_t yi=4; yi>=-4; yi--){
		// 运动到原点
		MoveToGrid(0, yi);
		SysTick_DelayMs(1000);
	}
}

/* 测试横向方向打点 */
void TestHorizontal(){
	for(int8_t xi=-6; xi<=6; xi++){
		// 运动到原点
		MoveToGrid(xi, 0);
		SysTick_DelayMs(1000);
	}
}

/* 测试角点 */
void TestCorner(){
	// 运动到A点
	MoveToGrid(-6, 4);
	SysTick_DelayMs(1000);
		
	// 运动到B点
	MoveToGrid(6, 4);
	SysTick_DelayMs(1000);
		
	// 运动到C点
	MoveToGrid(6, -4);
	SysTick_DelayMs(1000);
		
	// 运动到D点
	MoveToGrid(-6, -4);
	SysTick_DelayMs(1000);
}

/* 测试遍历所有的网格 */
void TestVisitAllGrid(){
	for(int8_t yi=2; yi>=-2; yi--){
		for(int8_t xi=-3; xi<=3; xi++){
			// 运动到原点
			MoveToGrid(xi, yi);
			SysTick_DelayMs(1000);
		}
	}
}
int main (void)
{
	// 嘀嗒定时器初始化
	SysTick_Init();
	Usart_Init(); // 串口初始化
	// 云台初始化
	Gimbal_Init(servoUsart);
	// 运动到原点
	MoveToGrid(0, 0);
	SysTick_DelayMs(1000);
		
	// 等待2s
	SysTick_DelayMs(2000);	
	
	while (1){
		// 测试角点
		// TestCorner();
		
		// 测试水平方向打点
		// TestHorizontal();
		
		// 测试垂直方向打点
		// TestVertical();
		
		// 测试遍历所有的网格
		TestVisitAllGrid();
		
	}
}
