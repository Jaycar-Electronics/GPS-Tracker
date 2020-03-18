#ifndef _CORRECT_TIME_H__
#define _CORRECT_TIME_H__

#define _EEPROM_DATE_ADDRESS 0xDB

#include <Arduino.h>
#include <EEPROM.h>
#include <rtc.h>

void correctTheTime(DS1307 *rtc)
{
  short check = EEPROM.read(_EEPROM_DATE_ADDRESS);
  if (check == 0xFF)
  {
    rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
    EEPROM.write(_EEPROM_DATE_ADDRESS, 1);
  }
}
#endif