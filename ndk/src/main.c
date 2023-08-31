#include "core_api.h"

#define GPIO_10 10
#define SNUM 8


void SendOnePix(unsigned char *ptr);						 // 发送一个LED的24bit数据
void ResetDataFlow(void);									 // 芯片复位脉冲，为发送下一帧做准备
int SendOneFrame(void *L);									 // 发送一帧数据
void SendSameColor(unsigned char *ptr, unsigned char cnt);	 // 相同颜色发送若干次
void SendOneFrameFrom(unsigned char i, unsigned char *ptr);	 // 从第i个像素开始发送
															 // 一帧数据
void SendOneFrameSince(unsigned char i, unsigned char *ptr); // 从第i个像素点的数据
															 // 开始发送

E_AMOPENAT_GPIO_PORT port = 0;

int init(void *L)
{
	// int str_size = 0;
	// char *data = luaL_checklstring(L,1, &str_size);
	// if(data[0] - 48 >= 0 && data[0] - 48 <= 50){
	// 	//port = data[0];
	// 	OPENAT_lua_print("WS2812 initialize on pin %d", data[0] - 48);
	// }
	int m = luaL_optinteger(L, 1, 0);
	OPENAT_lua_print("WS2812 initialize on pin %d", m);

	// OPENAT_lua_print("WS2812 init");
	// T_AMOPENAT_GPIO_CFG outpin;
	// T_OPENAT_GPIO_PARAM param;
	// T_OPENAT_GPIO_INT_CFG intCfg; //中断配置
	// intCfg.intType = OPENAT_GPIO_NO_INT; //不设置中断
	// param.intCfg = intCfg;//导入中断配置
	// param.pullState = OPENAT_GPIO_PULLUP;//下拉
	// param.defaultState = 1;//默认输出低电平
	// outpin.mode = OPENAT_GPIO_OUTPUT; //设置为输出模式
	// outpin.param = param; //导入参数
	// OPENAT_config_gpio(GPIO_10, &outpin); //配置GPIO_10
	return 0;
}

//-------------------------------------------------------------------------------
// 子程序名称:SendOnePix(unsigned char *ptr)
// 功能：发送一个像素点的24bit数据
// 参数：接收的参数是一个指针，此函数会将此指针指向的地址的连续的三个Byte的数据发送
// 说明：在主函数中直接调用此函数时，在整个帧发送开始前需要先执行 ResetDataFlow()
//		数据是按归零码的方式发送，速率为800KBPS
//-------------------------------------------------------------------------------
void SendOnePix(unsigned char *ptr)
{
	unsigned char i, j;
	unsigned char temp;

	for (j = 0; j < 3; j++)
	{
		temp = ptr[j];
		for (i = 0; i < 8; i++)
		{
			if (temp & 0x80) // 从高位开始发送
			{
				OPENAT_set_gpio(port, 1);
				OPENAT_set_gpio(port, 1);
				OPENAT_set_gpio(port, 1);
				OPENAT_set_gpio(port, 0);
			}
			else // 发送“0”码
			{
				OPENAT_set_gpio(port, 1);
				OPENAT_set_gpio(port, 0);
				OPENAT_set_gpio(port, 0);
				OPENAT_set_gpio(port, 0);
			}
			temp = (temp << 1); // 左移位
		}
	}
}

//-------------------------------------------------------------------------------
// 子程序名称:ResetDateFlow(void)
// 功能：复位，为下一次发送做准备，
// 说明：将DI置位为0后，延时约65us
//-------------------------------------------------------------------------------
void ResetDataFlow(void)
{
	unsigned char i;		  // DI置为0后，延时50us以上，实现帧复位
	for (i = 0; i < 173; i++) // 此处33Mhz时延时65us
	{
		OPENAT_set_gpio(10, 0);
	}
}

//-------------------------------------------------------------------------------
// 子程序名称:SendOneFrame(void *L)
// 功能：发送一帧数据（即发送整个数组的数据）
// 参数：接收的参数是一个指针，此函数会将此指针指向的地址的整个数组的数据发送
//-------------------------------------------------------------------------------
int SendOneFrame(void *L)
{
	unsigned char k;
	int str_size = 0;
	port = luaL_optinteger(L, 1, 0);

	// if (port < 0 || port > 50)
	// {
	// 	OPENAT_lua_print("WS2812 pin sould be 0~50,your input is %d", port);
	// 	port = 0;
	// 	// 返回0代表失败
	// 	lua_pushinteger (L,0);
	// 	return 1;
	// }

	char *ptr = luaL_checklstring(L, 2, &str_size);

	// if (str_size % 3 != 0)
	// {
	// 	OPENAT_lua_print("WS2812 data size error,your input is %d", str_size);
	// 	// 返回0代表失败
	// 	lua_pushinteger (L,0);
	// 	return 1;
	// }0 3 9 12 15 18 21 24 
	//

	OPENAT_lua_print("WS2812 Show One Frame on pin %d", port);

	ResetDataFlow(); // 发送帧复位信号

	for (k = 0; k < SNUM; k++) // 发送一帧数据，SNUM是板子LED的个数
	{
		SendOnePix(&ptr[(3 * k)]);
	}

	ResetDataFlow(); // 发送帧复位信号

	// 返回传入数组大小代表成功
	lua_pushinteger (L,str_size);
	return 1;
}

//-------------------------------------------------------------------------------
// 子程序名称:SendSameColor(unsigned char *ptr,unsigned char cnt)
// 功能：相同颜色的点发送cnt次
// 参数：接收的参数是一个指针，指向像素点颜色数组，cnt传递发送个数
//-------------------------------------------------------------------------------
void SendSameColor(unsigned char *ptr, unsigned char cnt)
{
	unsigned char k;

	ResetDataFlow(); // 发送帧复位信号

	for (k = 0; k < cnt; k++) // 发送一帧数据，SNUM是板子LED的个数
	{
		SendOnePix(&ptr[0]);
	}

	ResetDataFlow(); // 发送帧复位信号
}
//-------------------------------------------------------------------------------
// 子程序名称:SendOneFrameFrom(unsigned char i,unsigned char *ptr)
// 功能：从指定的像素点开始发送一帧数据（即发送整个数组的数据）
// 参数：接收的参数是一个指针，此函数会将此指针指向的地址的整帧数据发送
//		i:把数组的第0个像素数据发送到第i个像素点（第0个像素是板上标号为01的像素）
// 说明：即原本对应第一个像素的数据会发送到第i个像素点（LED）上
//-------------------------------------------------------------------------------
void SendOneFrameFrom(unsigned char i, unsigned char *ptr)
{
	unsigned char k;

	ResetDataFlow(); // 发送帧复位信号

	for (k = (SNUM - i); k < SNUM; k++) // 发送一帧数据
	{
		SendOnePix(&ptr[(3 * k)]);
	}
	for (k = 0; k < (SNUM - i); k++)
	{
		SendOnePix(&ptr[(3 * k)]);
	}

	ResetDataFlow(); // 发送帧复位信号
}

//-------------------------------------------------------------------------------
// 子程序名称:SendOneFrameSince(unsigned char i,unsigned char *ptr)
// 功能：从第i个像素点的数据开始发送一帧数据（即发送整个数组的数据）
// 参数：接收的参数是一个指针，此函数会将此指针指向的地址的整帧数据发送
//		i:把数组的第i个像素数据发送到第1个像素点
// 说明：即原本对应第i像素的数据会发送到第1个像素点（LED）上，第i+1个像素点的数据
//		发送到第2个像素上
//-------------------------------------------------------------------------------
void SendOneFrameSince(unsigned char i, unsigned char *ptr)
{
	unsigned char k;

	ResetDataFlow(); // 发送帧复位信号

	for (k = i; k < SNUM; k++) // 发送一帧数据
	{
		SendOnePix(&ptr[(3 * k)]);
	}
	for (k = 0; k < i; k++)
	{
		SendOnePix(&ptr[(3 * k)]);
	}
	ResetDataFlow(); // 发送帧复位信号
}

// /*测试函数2, 由lua直接调用*/
// int test_fun4(void *L)
// {
//     int str_size = 0;
//     /*获取第一个参数,参数类型为string*/
//     char *data = luaL_checklstring(L,2, &str_size);
//     OPENAT_lua_print("fun4 exe string=%d", data[0] - 48);
//     /*第一个返回值为字符串*/
//     lua_pushstring(L,data);
//     return 1;
// }

luaL_Reg user_lib[] = {
	{"init", init},
	{"SendOneFrame", SendOneFrame},
	{NULL, NULL}};

/*入口函数*/
int Initialize(void *L)
{
	/*C函数注册*/
	luaI_openlib(L, "WS2812", user_lib, 0);
	return 1;
}
