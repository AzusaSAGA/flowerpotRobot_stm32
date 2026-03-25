# stm32运动控制

芯片使用STM32F407VGt6；

**模块：**编码电机、步进电机、OLED、按键、旋转编码器、NRF24L01

**两种工作模式：**

1，遥控模式：

stm32接收来自遥控手柄的数据，控制电机运动和步进电机动作；

2，自动模式：

stm32接收上位机jetson orin nano的数据，控制电机运动和步进电机动作；





### 问题解决

```c
typedef struct {
	uint8_t header;
	uint32_t data1;
	uint32_t data2;
	uint8_t checksum;
}USART3_Frame;
```

定义的数据结构如上图，发给jetson，接收端收到16个字节，因为结构体内部数据会存在**内存对齐**；

解决方法：

```c
#pragma pack(push,1)   //保存当前对齐方式，并设置为1字节对齐
typedef struct {
	uint8_t header;
	uint32_t data1;
	uint32_t data2;
	uint8_t checksum;
}USART3_Frame;
#pragma pack(pop)      // 恢复之前的对齐方式
```

