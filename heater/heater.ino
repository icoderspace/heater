
#include "mcu.h"
#include "key.h"
#include "link.h"
/*
* 按键协议
* 1010 1010 1010 1011 都不按的时候接收到的信息
* 解析方式 反转：1101 0101 0101 0101
* 前8位固定
* 后八位 01(cool) 01(lock) 01(heat) 01(temp)
* 如果按了哪个键，括号里就为1 否则无值
* 例如按了heat键，反转后结果为 1101 0101 0101 01101
*
*
* LED灯协议：
* 都不亮灯：1101 0101 0101 0101
* 前8位固定
* 后八位 01(cool) 01(heat) 01(temp) 01(lock)
*
*
*/

//Ardiuno键盘的接收，连接键盘的发送端
int key_rec = 6;
//连接键盘的接收端
int key_send = 4;
//Ardiuno中控的接收，连接中控的发送端
int mcu_rec = 10;
//Ardiuno中控的发送，连接中控的接收端
int mcu_send = 9;

int manual = 3;

Key key(key_send, key_rec);
Mcu mcu(mcu_send, mcu_rec);
static const int TEMP_45 = 1;
static const int TEMP_65 = 2;
static const int TEMP_85 = 3;
static const int TEMP_100 = 4;

static const int LEVEL_1 = 0;
static const int LEVEL_2 = 1;
static const int LEVEL_3 = 2;
static const int LEVEL_4 = 3;
int cur_temp = TEMP_100;
int cur_level = LEVEL_1;
//不同温度，不同水位的出水时间
unsigned long time[5][4] = {
					{ 8000,14000,20000,20000 },
					{ 13000,22000,23000,23000 },
					{ 14000,23000,24000,24000 },
					{ 15000,25000,26000,26000 },
					{ 16000,27000,28000,28000 } };
unsigned long open_cool_timestramp = 0;
unsigned long open_heat_timestramp = 0;

void setup()
{
	Serial.begin(115200);
	Serial.print("Begin");

	pinMode(key_send, OUTPUT);
	pinMode(key_rec, INPUT_PULLUP);
	pinMode(mcu_send, OUTPUT);
	pinMode(mcu_rec, INPUT_PULLUP);
	pinMode(manual, INPUT_PULLUP);
}

void process_key_lock() {
	cur_level = (cur_level + 1) % 4;
}

int get_open_time()
{
	return time[cur_temp][cur_level];
}

void refresh_led() {
	switch (cur_level)
	{
	case LEVEL_1:
		key.show_led(Key::COMMAND_COOL);
		break;
	case LEVEL_2:
		key.show_led(Key::COMMAND_COOL | Key::COMMAND_TEMP);
		break;
	case LEVEL_3:
		key.show_led(Key::COMMAND_COOL | Key::COMMAND_TEMP | Key::COMMAND_HEAT);
		break;
	case LEVEL_4:
		key.show_led(Key::COMMAND_COOL | Key::COMMAND_TEMP | Key::COMMAND_HEAT | Key::COMMAND_LOCK);
		break;
	default:
		break;
	}
}


void process_key_temp() {
	cur_temp = (cur_temp + 1) % 4;
	mcu.send_command(Mcu::KEY_TEMP);
}

void process_key_cool() {
	open_cool_timestramp = millis();

	mcu.send_command(Mcu::KEY_COOL);
}

void process_key_heat() {
	while (mcu.is_locked()) {
		mcu.send_command(Mcu::KEY_LOCK);
		delay(500);
		mcu.listen();
		Serial.println("wait lock");
	}
	open_heat_timestramp = millis();
	mcu.send_command(Mcu::KEY_HEAT);
}

bool process_opening_water() {
	int key_command = key.get_command();

	if (mcu.has_status(Mcu::STATUS_COOL)) {
		//如果正在放水，按任意键停止
		if (key_command != Key::COMMAND_NONE) {
			mcu.send_command(Key::COMMAND_COOL);
			return true;
		}
		if (millis() - open_cool_timestramp > time[0][cur_level]) {
			//到达指定时间
			mcu.send_command(Key::COMMAND_COOL);
			return true;
		}
	}
	if (mcu.has_status(Mcu::STATUS_HEAT)) {
		//如果正在放水，按任意键停止
		if (key_command != Key::COMMAND_NONE) {
			mcu.send_command(Key::COMMAND_HEAT);
			return true;
		}
		if (millis() - open_heat_timestramp > get_open_time()) {
			//到达指定时间
			mcu.send_command(Key::COMMAND_HEAT);
			return true;
		}
	}
	return false;
}
void print() {
	Serial.print("cur_level=");
	Serial.print(cur_level);
	Serial.print(",cur_temp=");
	Serial.print(cur_temp);

	Serial.print(",open_cool_timestramp=");
	Serial.print(open_cool_timestramp);
	Serial.print(",open_heat_timestramp=");
	Serial.println(open_heat_timestramp);
	//处理
}
void refresh_mcu() {
	mcu.send_command(Mcu::KEY_NONE);
}

bool process_manual_key() {

	if (digitalRead(manual) == 0) {
		int key_command = key.get_command();
		switch (key_command)
		{
		case Key::COMMAND_LOCK:
			mcu.send_command(Mcu::KEY_LOCK);
			break;
		case Key::COMMAND_TEMP:
			mcu.send_command(Mcu::KEY_TEMP);
			break;
		case Key::COMMAND_COOL:
			mcu.send_command(Mcu::KEY_COOL);
			break;
		case Key::COMMAND_HEAT:
			mcu.send_command(Mcu::KEY_HEAT);
			break;
		default:
			break;
		}
		int mcu_status = mcu.get_status();
		if (mcu.has_status(Mcu::STATUS_COOL)) {
			key.show_led(Key::LED_COOL);
		}
		if (mcu.has_status(Mcu::STATUS_HEAT)) {
			key.show_led(Key::LED_HEAT);
		}
		if (mcu.has_status(Mcu::STATUS_TEMP)) {
			key.show_led(Key::LED_TEMP);
		}
		if (mcu.has_status(Mcu::STATUS_LOCK)) {
			key.show_led(Key::LED_LOCK);
		}
		return true;
	}
	return false;
}

void loop()
{
	//print();
	if (Serial.available()) {
		int c = Serial.read();
		Serial.println(c);
		switch (c)
		{
		case 'l':
			mcu.send_command(Mcu::KEY_LOCK);
			break;
		case 'c':
			mcu.send_command(Mcu::KEY_COOL);
			break;
		case 't':
			mcu.send_command(Mcu::KEY_TEMP);
			break;
		case 'h':
			mcu.send_command(Mcu::KEY_HEAT);
			break;
		default:
			break;
		}
	}
	if (key.listen() && mcu.listen()) {
		int key_command = key.get_command();
		int mcu_status = mcu.get_status();
		Serial.print("key command=");
		Serial.print(key_command);
		Serial.print(",mcu status=");
		Serial.println(mcu_status);
		if (process_manual_key()) {
			return;
		}
		//先处理放水
		if (!process_opening_water()) {
			switch (key_command)
			{
			case Key::COMMAND_LOCK:
				process_key_lock();
				break;
			case Key::COMMAND_TEMP:
				process_key_temp();
				break;
			case Key::COMMAND_COOL:
				process_key_cool();
				break;
			case Key::COMMAND_HEAT:
				process_key_heat();
				break;
			default:
				break;
			}
		}

	}
	refresh_led();
	refresh_mcu();
	//mcu.send_command(Mcu::KEY_TEMP);
	//delay(1000);
	///*for (int i = 0; i < 10; i++) {
	//	mcu.send_command(Mcu::KEY_NONE);
	//	delay(20);
	//}*/
	//Serial.println("loop");

}





