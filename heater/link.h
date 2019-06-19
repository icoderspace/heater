#ifndef LINK_H
#define LINK_H

const int INTERVAL = 400;
bool __wait_start(int pin)
{
	/*
	 *0- ��ʼ�� �͵�ƽ
	 *1- ��ͷ �ߵ�ƽ ����3200us
	 *2-�͵�ƽ����400us��֮��ʼ������
	*/
	int status = 0;
	unsigned long timestramp = 0;
	int retry = 0;
	while (retry++ < 1600)
	{
		int value = digitalRead(pin);
		switch (status)
		{
		case 0:
			if (value == 1)
			{
				status = 1;
				timestramp = micros();
			}
			break;
		case 1:
			if (value == 0)
			{
				if (micros() - timestramp > 3100 && micros() - timestramp < 3300)
				{
					status = 2;
					return true;
				}
				else
				{
					//����ʱ���ɵ͵�ƽ
					status = 0;
				}
			}
			else if (micros() - timestramp > 3300)
			{
				status = 0;
			}
			break;
		default:
			break;
		}
		delayMicroseconds(INTERVAL / 4);
	}
	return false;
}
/*
 *����ָ���˿ڷ���20λ���ݣ����Ϊ0��ʾ�쳣���߳�ʱ
 */
unsigned long recevie(int pin)
{
	unsigned long result = 0;
	if (__wait_start(pin))
	{
		delayMicroseconds(50);
		for (int i = 0; i < 20; i++)
		{
			delayMicroseconds(INTERVAL);
			result = (result << 1) + digitalRead(pin);
		}
		return result;
	}
	Serial.println("start failed.");
	return 0;
}

/*
 * ���������λ��ǰ
 */
void send(int pin, unsigned long value)
{
	/*Serial.print("send pin=");
	Serial.print(pin);
	Serial.print(",value=");
	Serial.println(value, BIN);*/

	digitalWrite(pin, 1);
	delayMicroseconds(3200);
	digitalWrite(pin, 0);
	delayMicroseconds(400);
	int tmp = 0;
	while (value > 0)
	{
		digitalWrite(pin, value & 1);
		value >>= 1;
		delayMicroseconds(400);
	}
	digitalWrite(pin, 0);
}
#endif
