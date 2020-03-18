#ifndef _DEFINITIONS_H__
#define _DEFINITIONS_H__

//GPS is connected to Serial1, we can do a quick rename to remind us
#define gps Serial1

const short power_led = 6;
const short sd_led = 5;
const short gps_led = 3;
const short charge_pin = A0;

//simple macros
#define SECONDS(x) (x * 1000)
#define MINUTES(x) (SECONDS(60) * x)
#define HOURS(x) (MINUTES(60) * x)


typedef struct
{
  unsigned long power_led_timer = 0;
  unsigned long gps_lasttime = 0;
} ledTimers;

#endif
