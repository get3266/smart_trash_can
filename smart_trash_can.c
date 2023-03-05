#include "reg52.h"
sbit D5 = P3^7;//根据原理图（电路图），设备变量led1指向P3组IO口的第7口
sbit D6 = P3^6;//设备变量led2指向P3组IO口的第6口
sbit SW1 = P2^1;
sbit Trig = P1^5;
sbit Echo = P1^6;
sbit sg90_con = P1^1;
sbit vibrate ?= P3^2;
sbit beep ??= P2^0;
char jd;
char jd_bak;
char cnt = 0;
char mark_vibrate = 0;
void Delay150ms() //@11.0592MHz
{
	unsigned char i, j, k;
	i = 2;
	j = 13;
	k = 237;
	do{
		do{
		while (--k);
		} while (--j);
	} while (--i);
}
void Delay2000ms() //@11.0592MHz
{
	unsigned char i, j, k;
	i = 15;
	j = 2;
	k = 235;
	do{
		do{
		while (--k);
		} while (--j);
	} while (--i);
}
void Delay10us() //@11.0592MHz
{
	unsigned char i;
	i = 2;
	while (--i);
}
void Time0Init()
{
//1. 配置定时器0工作模式位16位计时
	TMOD &= 0xF0; //设置定时器模式
	TMOD |= 0x01;
//2. 给初值，定一个0.5出来
	TL0=0x33;
	TH0=0xFE;
//3. 开始计时
	TR0 = 1;
	TF0 = 0;
//4. 打开定时器0中断
	ET0 = 1;
//5. 打开总中断EA
	EA = 1;
}
void Time1Init()
{
	TMOD &= 0x0F; //设置定时器模式
	TMOD |= 0x10;
	TH1 = 0;
	TL1 = 0;
//设置定时器0工作模式1，初始值设定0开始数数，不着急启动定时器
}
void startHC()
{
	Trig = 0;
	Trig = 1;
	Delay10us();
	Trig = 0;
}
double get_distance()
{
	double time;
//定时器数据清零，以便下一次测距
	TH1 = 0;
	TL1 = 0;
//1. Trig ，给Trig端口至少10us的高电平
	startHC();
//2. echo由低电平跳转到高电平，表示开始发送波
	while(Echo == 0);
//波发出去的那一下，开始启动定时器
	TR1 = 1;
//3. 由高电平跳转回低电平，表示波回来了
	while(Echo == 1);
//波回来的那一下，停止定时器
	TR1 = 0;
//4. 计算出中间经过多少时间
	time = (TH1 * 256 + TL1)*1.085;//us为单位
//5. 距离 = 速度 （340m/s）* 时间/2
	return (time * 0.017);
}
void openStatusLight()
{
	D5 = 0;
	D6 = 1;
}
void closeStatusLight()
{
	D5 = 1;
	D6 = 0;
}
void initSG90_0()
{
	jd = 1; ???//初始角度是0度，0.5ms,溢出1就是0.5，高电平
	cnt = 0;
	sg90_con = 1;//一开始从高电平开始
}
void openDusbin()
{
	char n;
	jd = 3; //90度 1.5ms高电平
	 //舵机开盖
	if(jd_bak != jd){
		cnt = 0;
		beep = 0;
		for(n=0;n<2;n++)
			Delay150ms();
		beep = 1;
		Delay2000ms();
		}
	jd_bak = jd;
}
void closeDusbin()
{
//关盖
	jd = 1; //0度
	jd_bak = jd;
	cnt = 0;
	Delay150ms();
}
void EX0_Init()
{
//打开外部中断
	EX0 = 1;
//低电平触发
	IT0 = 0;
}
void main()
{
	double dis;
	Time0Init();
	Time1Init();
	EX0_Init();
//舵机的初始位置
	initSG90_0();
	while(1){
//超声波测距
	dis = get_distance();
	if(dis < 10 || SW1 == 0 || mark_vibrate == 1){//如果小于10厘米,或者sw1按键被按下
//开盖，灯状态，D5亮
		openStatusLight();
		openDusbin();
		mark_vibrate = 0;
		}else{
//关盖，灯状态，D5灭
		closeStatusLight();
		closeDusbin();
		}
	}
}
void Time0Handler() interrupt 1
{
	cnt++; //统计爆表的次数. cnt=1的时候，报表了1
	//重新给初值
	TL0=0x33;
	TH0=0xFE;
	//控制PWM波
	if(cnt < jd){
		sg90_con = 1;
		}else{
		sg90_con = 0;
	}
	if(cnt == 40){//爆表40次，经过了20ms
		cnt = 0; ?//当100次表示1s，重新让cnt从0开始，计算下一次的1s
		sg90_con = 1;
	}
}
void Ex0_Handler() interrupt 0
{
	mark_vibrate = 1;
}
