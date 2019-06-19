#ifndef CONSTANT_H
#define CONSTANT_H

static const unsigned long KEY_VALUE_LOCK = 46422;
static const unsigned long KEY_VALUE_HEAT = 54614;
static const unsigned long KEY_VALUE_COOL = 44374;
static const unsigned long KEY_VALUE_TEMP = 21846;
static const unsigned long KEY_VALUE_NORMAL = 43692;
//所有灯都不亮
static const unsigned long LED_VALUE_NORMAL = 0b1101010101010101;

//只亮冷水键
static const unsigned long LED_VALUE_COOL = 0b11010101011010101;
//只亮热温键
static const unsigned long LED_VALUE_HEAT = 0b11010101010110101;
//只亮调温键
static const unsigned long LED_VALUE_TEMP = 0b11010101010101101;
//只亮LOCK键
static const unsigned long LED_VALUE_LOCK = 0b11010101010101011;

static const unsigned long LED_VALUE_LOCK_COOL = 0b110101010110101011;
static const unsigned long LED_VALUE_LOCK_HEAT = 0b110101010101101011;
static const unsigned long LED_VALUE_LOCK_TEMP = 0b110101010101011011;
static const unsigned long LED_VALUE_COOL_TEMP = 0b110101010101011011;

static const unsigned long LED_VALUE_LOCK_COOL_HEAT = 0b110101010111101011;

#endif