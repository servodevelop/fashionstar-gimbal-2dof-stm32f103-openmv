'''
色块识别
1. 识别画面中的红色小球
2. 将识别的到的色块位置,通过串口发送
'''
import sensor
import image
import time
import ustruct as struct
from pyb import UART

# 红色小球的LAB色彩空间阈值 (L Min, L Max, A Min, A Max, B Min, B Max)
RED_BALL_THRESHOLD = (30, 74, 38, 85, -21, 62)
# ROI搜索半径
ROI_R = 10

# 串口初始化
uart = UART(3, 115200)

# OpenMV感光芯片初始化
sensor.reset() # 重置感芯片
sensor.set_pixformat(sensor.RGB565) # 设置像素格式为RGB565
sensor.set_framesize(sensor.QVGA) # 设置分辨率为QVGA (340 * 240)
sensor.set_vflip(True) # 纵向翻转
sensor.skip_frames(time = 2000) # 跳过2s内的帧, 等待画质稳定
sensor.set_auto_gain(False) # 关闭自动增益
sensor.set_auto_whitebal(False) # 关闭自动白平衡

# 初始化时钟
clock = time.clock()

last_roi = None
while(True):
    clock.tick() # 开始计时
    img = sensor.snapshot() # 拍摄一张照片
    blobs = []

    if last_roi is not None:
        x, y, w, h  = last_roi
        x -= ROI_R
        y -= ROI_R
        x = 0 if x < 0 else x
        y = 0 if y < 0 else y
        w = w + 2*ROI_R
        h = h + 2*ROI_R
        new_roi = (x, y, w, h)
        blobs = img.find_blobs([RED_BALL_THRESHOLD], pixels_threshold=100, \
              area_threshold=100, merge=True, roi=new_roi)

    if len(blobs) == 0:
        # 获取画面中的色块
        blobs = img.find_blobs([RED_BALL_THRESHOLD], pixels_threshold=100, area_threshold=100, merge=True)

    if len(blobs) != 0:
        # 获得画面中的最大的色块
        blob = max(blobs, key=lambda  b: b.area())
        # 更新ROI
        last_roi = blob.rect()
        # 可视化
        img.draw_rectangle(blob.rect()) # 绘制色块的矩形区域
        img.draw_cross(blob.cx(), blob.cy()) # 绘制色块的中心位置
        print("色块中心坐标: {} {}".format(blob.cx(), blob.cy()))
        # 通过串口发送数据(二进制　低字节序)
        uart.write(struct.pack('<BBBHH', 0xFF, 0xF1, True, blob.cx(), blob.cy()))
    else:
        # 目标丢失
        print("色块丢失!!!")
        last_roi = None
        # 通过串口发送数据(二进制　低字节序)
        uart.write(struct.pack('<BBBHH', 0xFF, 0xF1, False, 0, 0))
    # 打印当前的帧率
    print(clock.fps())
