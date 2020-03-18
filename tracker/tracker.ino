
#include "definitions.h"

#include <SPI.h>
#include <SD.h>

// non-const variables are declared here
bool connected = false;

//update tracking information every 20 seconds
const unsigned long sdcard_interval = SECONDS(20);
//we have also set up MINUTES and HOURS

static ledTimers timer;

void setup()
{
  Serial.begin(9600);
  gps.begin(9600);
  gps.setTimeout(1100); //if we don't receive anything in 1.1 seconds, timeout
  setupPins();
}

void loop()
{
  if (checkConnection())
  {
    String coords = "";
    String utc = "";
    String date = "";

    Serial.println("Running in Debug Mode");

    blinkConnected(); //charge mode

    //empty out buffer;
    if (!extractGPS(coords, utc, date))
    {
      blinkGPSError();
      return;
    }

    // if it's time to update
    if (checkTimeout(sdcard_interval))
    {
      //write information to SD card
      writeInformation(coords, utc, date);
    }
  }
  else
  {
    Serial.println("Running in Command Mode");
    allLightsOn();

    String command = Serial.readString();
    if (command.equals(""))
    {
      return;
    }
    processCommand(command);
  }
}

// =============================================================
//  Private Function definitions below
// =============================================================
void allLightsOn()
{
  digitalWrite(gps_led, HIGH);
  digitalWrite(sd_led, HIGH);
  digitalWrite(power_led, HIGH);
}

void writeInformation(String &coords, String &utc, String &date)
{
  digitalWrite(sd_led, HIGH);
  SPI.begin();
  if (!SD.begin(10))
  {
    Serial.println("SD card failed to init");
    blinkSDError();
    return;
  }

  File fptr = SD.open(date, FILE_WRITE);

  if (!fptr)
  {
    Serial.print("Failed to open file:");
    Serial.println(date);
    blinkSDError();
    return;
  }

  fptr.print(utc);
  fptr.print(" ");
  fptr.println(coords);

  fptr.close();

  digitalWrite(sd_led, LOW);
}
void blinkGPSError()
{
  //we don't need timers here.
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(gps_led, !digitalRead(gps_led));
    delay(50);
  }
  digitalWrite(gps_led, LOW);
}
void blinkSDError()
{
  //we don't need timers here.
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(sd_led, !digitalRead(sd_led));
    delay(50);
  }
  digitalWrite(sd_led, LOW);
}

bool extractGPS(String &coords, String &time, String &date)
{
  //this function will empty the buffer, which is what we want

  String tmp = "";
  tmp.reserve(5);

  Serial.println("waiting for information");
  while (!tmp.equals(F("GPRMC")))
  {
    gps.readStringUntil('$');       //read to start of sentence
    tmp = gps.readStringUntil(','); //read command of sentence
    if (tmp.equals(""))
    {
      return false;
    }
  }

  digitalWrite(gps_led, HIGH);
  // GPRMC data:
  // $GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62

  time = gps.readStringUntil(','); //time first;
  tmp = gps.readStringUntil(',');  //validity
  if (tmp.equals("V"))             // invalid
  {
    //log data; ?
    Serial.println("bad GPS information");
    Serial.println(gps.readStringUntil('\n'));
    return false;
  }

  coords = gps.readStringUntil(','); //latitude
  gps.readStringUntil(',');          // North or south
  coords.concat(',');
  coords.concat(gps.readStringUntil(',')); //longitude
  gps.readStringUntil(',');                // East or west
  gps.readStringUntil(',');                // dont care
  gps.readStringUntil(',');                // dont care
  date = gps.readStringUntil(',');         //date
  gps.readStringUntil('\n');               // we don't need anything else
  digitalWrite(gps_led, LOW);
  return true;
}

bool checkTimeout(unsigned long interval)
{
  long duration = millis() - timer.gps_lasttime;
  if (duration > interval)
  {
    timer.gps_lasttime = millis();
    return true;
  }
  return false;
}
void processCommand(String &command)
{
  command.toLowerCase();
  command.trim();
  String params = "";
  if (command.startsWith("list"))
  {
    listTrackingInformation();
  }
  else if (command.startsWith("read"))
  {
    params = command.substring(5);
    readTrackingInformation(params);
  }
  else if (command.startsWith("delete"))
  {
    params = command.substring(7);
    deleteTrackingInformation(params);
  }
  else if (command.startsWith("clear"))
  {
    clearTrackingInformation();
  }
  else
  {
    Serial.print("Unrecognised command '");
    Serial.print(command);
    Serial.println("'");
  }
}

void listTrackingInformation()
{
  //todo
  SPI.begin();
  if (!SD.begin(10))
  {
    Serial.println("failed to init SD card");
    blinkSDError();
    return;
  }
  Serial.print("LIST ");

  File dir = SD.open("/");
  while (true)
  {
    File next = dir.openNextFile();
    if (!next)
    { //nothing left
      break;
    }
    if (next.isDirectory())
    {
      continue;
    }
    Serial.print(next.name());
    Serial.print("|");
    next.close();
  }
  Serial.println(); //finish off
}
void readTrackingInformation(String &name)
{
  //todo
  SPI.begin();
  if (!SD.begin(10))
  {
    Serial.println("failed to init SD card");
    blinkSDError();
    return;
  }
  File file = SD.open(name, FILE_READ);
  if (!file || file.isDirectory())
  {
    Serial.print("incorrect data in ");
    Serial.println(name);
    return;
  }
  Serial.print("DATA ");
  String buffer = "";
  while (file.available())
  {
    buffer = file.readStringUntil('\n');
    buffer.trim();
    Serial.print(buffer);
    Serial.print("|");
  }
  Serial.println();
}
void clearTrackingInformation()
{
  //todo
}
void deleteTrackingInformation(String &name)
{
  //todo
}

void blinkConnected()
{
  long duration = millis() - timer.power_led_timer;
  if (duration > 1000)
  {
    digitalWrite(power_led, !digitalRead(power_led));
    timer.power_led_timer = millis();
  }
}
void blinkRunning()
{
  long duration = millis() - timer.power_led_timer;
  if (duration > 3000)
  {
    digitalWrite(power_led, !digitalRead(power_led));
    timer.power_led_timer = millis();
  }
}

void setupPins()
{
  pinMode(power_led, OUTPUT);
  pinMode(gps_led, OUTPUT);
  pinMode(sd_led, OUTPUT);
  pinMode(charge_pin, INPUT_PULLUP);
  pinMode(10, OUTPUT); //for sdcard; no point making variables
}

bool checkConnection()
{
  return digitalRead(charge_pin) == LOW;
}