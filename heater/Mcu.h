#ifndef MCU_H
#define MCU_H
#include "link.h"


class Mcu
{
private:
	int receive_pin;
	int send_pin;
	int status = 0;
public:
	static const int STATUS_NONE = 0;
	static const int STATUS_LOCK = 1;
	static const int STATUS_COOL = 4;
	static const int STATUS_HEAT = 2;
	static const int STATUS_TEMP = 8;

	static const int KEY_NONE = 0;
	static const int KEY_LOCK = 1;
	static const int KEY_COOL = 4;
	static const int KEY_HEAT = 2;
	static const int KEY_TEMP = 8;

	Mcu(int send_pin, int receive_pin)
	{
		this->send_pin = send_pin;
		this->receive_pin = receive_pin;
	}

	bool is_locked() const
	{
		return (status&STATUS_LOCK) == STATUS_LOCK;
	}

	bool has_status(int target) const
	{
		return (status&target) == target;
	}

	int get_status() const
	{
		return status;
	}
	/*
	 * ¸ømcu·¢ËÍÃüÁî
	 */
	bool send_command(int key)
	{
		unsigned long command = 0b11010101;
		int map[4]{ KEY_COOL,KEY_LOCK,KEY_HEAT,KEY_TEMP };
		for (int i = 0; i < 4; ++i)
		{
			command = (command << 2) + 1;
			if ((key&map[i]) == map[i])
			{
				command = (command << 1) + 1;
			}
		}
		//		unsigned long revert = 0;
		//		while (command > 0)
		//		{
		//			revert = (revert << 1) + (command & 1);
		//			command >>= 1;
		//		}
		for (int i = 0; i < 5; ++i)
		{
			send(send_pin, command);
			delay(10);
		}
		if (key != KEY_NONE)
		{
			send_command(KEY_NONE);
		}
	}

	bool listen()
	{
		unsigned long raw = recevie(receive_pin);
		status = STATUS_NONE;
		if (raw <= 0)
		{
			Serial.println("no value for mcu");
			status = STATUS_NONE;
			return false;
		}
		unsigned long temp = raw;
		unsigned long value = 0;
		while (temp > 0)
		{
			value = (value << 1) + (temp & 1);
			temp >>= 1;
		}
		int map[4]{ STATUS_LOCK,STATUS_TEMP,STATUS_HEAT,STATUS_COOL };
		while ((value & 1) == 0)
		{
			value >>= 1;
		}
		int mask = 3;
		for (int i = 0; i < 4; ++i)
		{
			if ((value&mask) == mask)
			{
				status |= map[i];
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
				status = STATUS_NONE;
				Serial.println("mcu wrong protocol,e1");
				Serial.println(raw, BIN);
				return false;
			}
		}
		if (value != 0b11010101)
		{
			status = STATUS_NONE;
			Serial.println("mcu wrong protocol,e2");
			Serial.println(raw, BIN);
			return false;
		}
		return true;
	}
};
#endif