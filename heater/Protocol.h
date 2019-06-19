
#ifndef PROTOCOL_H
#define PROTOCOL_H
class Protocol
{
	

	static const int NONE = 0;
	static const int STARTING = 1;
	static const int TRANSFER = 2;
	static const int STOPING = 3;
	static const int FINISHED = 4;
	static const int INTERVAL = 400;
	//采样点 400us
	static const int STARTING_INTERVAL = 3200;

	static const int DELTA = 80;
private:
	int rec_pin;
	int send_pin;
	unsigned long result = 0;
	int bit = 0;
	int status = NONE;
	//上一个采样点
	unsigned long last_point = 0;


	bool readBit(int value)
	{
		unsigned long now = micros();
		//留出50us等待电平稳定
		if (now - last_point > INTERVAL + 50)
		{
			last_point += INTERVAL;
			result <<= 1;
			result += value;
			bit++;
			return true;
		}
		return false;
	}

	void processNone(int value)
	{
		if (value == 1)
		{
			status = STARTING;
			last_point = micros();
			//Serial.println("p0");
		}
	}

	void processStarting(int value)
	{
		long now = micros();
		if (value == 0)
		{
			if (now - last_point >= STARTING_INTERVAL - DELTA && now - last_point <= STARTING_INTERVAL + DELTA)
			{
				//在延时3150~3250us的时刻降为低电平
				status = TRANSFER;
				last_point = now;
				//Serial.println("p1");
			}
			else
			{
				//其他时刻进入低电平， 异常
				Serial.println("e1");
				reset();
			}
		}
		else
		{
			if (now - last_point >= STARTING_INTERVAL + DELTA)
			{
				//如果超过3250us还是高电平，异常
				Serial.println("e2");
				reset();
			}
			else
			{
				//正常等待恢复低电平的状态
			}
		}
	}

	void processTransfer(int value)
	{
		long now = micros();
		//如果过了450us，读取数据
		if (now - last_point > INTERVAL + DELTA)
		{
			last_point += INTERVAL;
			if (value == 0 && (result & 0x01) == 0)
			{
				//连续两个0，结束
				status = FINISHED;
			}
			result = (result << 1) + value;
			bit++;
			if (bit > 20)
			{
				//最多16位
				Serial.println("e4");
				reset();
			}
		}
	}

	//	void processStoping(int value)
	//	{
	//		long now = micros();
	//		if (now - last_point > INTERVAL + DELTA)
	//		{
	//			last_point += INTERVAL;
	//			if (value == 0)
	//			{
	//				status = FINISHED;
	//			}
	//			else
	//			{
	//				Serial.println(result, BIN);
	//				Serial.println("e3");
	//				//异常
	//				reset();
	//			}
	//		}
	//	}

public:
	void init(int rec_pin, int send_pin)
	{
		this->rec_pin = rec_pin;
		this->send_pin = send_pin;
		digitalWrite(send_pin, 0);
	}

	void print()
	{
		//    Serial.print("pin = ");
		//    Serial.print(pin);
		//    Serial.print(",status = ");
		//    Serial.print(status);
		//    Serial.print(",now = ");
		//    Serial.print(micros());
		//    Serial.print(",last_point = ");
		//    Serial.print(last_point);
		//    Serial.print(",stop = ");
		//    Serial.print(stop);
		//    Serial.print(",stop_bit = ");
		//    Serial.println(stop_bit);
	}

	void reset()
	{
		//Serial.println("reseting");
		print();
		bit = 0;
		last_point = micros();
		status = NONE;
		result = 0;

	}

	unsigned long read()
	{
		unsigned long tmp = result;
		reset();
		return tmp;
	}

	bool available()
	{
		int value = digitalRead(rec_pin);
		switch (status)
		{
		case NONE:
			processNone(value);
			break;
		case STARTING:
			processStarting(value);
			break;
		case TRANSFER:
			processTransfer(value);
			break;
		
		case FINISHED:
			return true;
		default:
			break;
		}
		return false;
	}
	void send(unsigned long value)
	{
		digitalWrite(send_pin, 1);
		delayMicroseconds(3200);
		digitalWrite(send_pin, 0);
		delayMicroseconds(400);
		int tmp = 0;
		while (value > 0)
		{
			digitalWrite(send_pin, value & 1);
			value >>= 1;
			delayMicroseconds(400);
		}
		digitalWrite(send_pin, 0);
	}
};


#endif // !PROTOCOL_H

