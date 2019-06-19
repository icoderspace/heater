#ifndef KEY_H
#define KEY_H
class Key
{
private:
	int receive_pin;
	int send_pin;
	int command = 0;
public:
	static const int COMMAND_NONE = 0;
	static const int COMMAND_LOCK = 1;
	static const int COMMAND_COOL = 4;
	static const int COMMAND_HEAT = 2;
	static const int COMMAND_TEMP = 8;

	static const int LED_NONE = 0;
	static const int LED_LOCK = 1;
	static const int LED_COOL = 4;
	static const int LED_HEAT = 2;
	static const int LED_TEMP = 8;

	Key(int send_pin, int receive_pin)
	{
		this->send_pin = send_pin;
		this->receive_pin = receive_pin;
	}

	int get_command() const
	{
		return command;
	}
	/*
	* 点亮指定灯
	*/
	bool show_led(int led)
	{
		unsigned long command = 0b11010101;
		int map[4]{ LED_COOL,LED_HEAT,LED_TEMP,LED_LOCK };
		for (int i = 0; i < 4; ++i)
		{
			command = (command << 2) + 1;
			if ((led&map[i]) == map[i])
			{
				command = (command << 1) + 1;
			}
		}
		/*unsigned long revert = 0;
		while (command > 0)
		{
			revert = (revert << 1) + (command & 1);
			command >>= 1;
		}
		Serial.println(revert, BIN);*/
		send(send_pin, command);
	}

	/*
	 * 监听键盘发送了什么命令
	 */
	bool listen()
	{
		unsigned long raw = recevie(receive_pin);
		//Serial.println(raw, BIN);
		//delay(1);
		command = COMMAND_NONE;
		if (raw <= 0)
		{
			Serial.println("no value for key");
			command = COMMAND_NONE;
			return false;
		}
		unsigned long temp = raw;
		unsigned long value = 0;
		while (temp > 0)
		{
			value = (value << 1) + (temp & 1);
			temp >>= 1;
		}
		int map[4]{ COMMAND_TEMP,COMMAND_HEAT,COMMAND_LOCK,COMMAND_COOL };

		while ((value & 1) == 0)
		{
			value >>= 1;
		}
		int mask = 3;
		for (int i = 0; i < 4; ++i)
		{
			if ((value&mask) == mask)
			{
				command |= map[i];
				value >>= 2;
			}
			else
			{
				value >>= 1;
			}
			if ((value & 1) == 0)
			{
				value >>= 1;
			}
			else
			{
				command = COMMAND_NONE;
				Serial.println("key:wrong protocol,e1");
				Serial.println(raw, BIN);
				return false;
			}
		}
		if (value != 0b11010101)
		{
			command = COMMAND_NONE;
			Serial.println("key:wrong protocol,e2");
			Serial.println(raw, BIN);
			return false;
		}
		return true;
	}
};
#endif