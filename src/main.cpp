#include <Arduino.h>

#define WIFI_USE
#include <Hendi-Multi-IoT.h>
#include "Hendi-EEPROM.h"

int device = 3;

int urutX;
int urutY;

void ResetStep(DynamicJsonDocument source_doc);

unsigned long daq_check;
unsigned long daq_check_interval = 10 * 1000;
void switchRelay(bool _the_state);
int getTemperature();
void sendData();
void deviceRelaySwitch(DynamicJsonDocument source_doc);
void devicePing(DynamicJsonDocument source_doc);
void deviceRandomFunction(DynamicJsonDocument source_doc);
void autoRelay();
void deviceSwitchConnection(DynamicJsonDocument source_doc);

void uwbRead();

#define keyPin 25
#define mechPin 32
#define pbpin 0

#define epromSave 115

int Running;
int keyStatus;
int Xvalue;
int Yvalue;

int hexToDec(String hexString);

long mechStep;
long mechStepMinutes;
long lastMilis;
long lastMilis2;

#define RXD2 16
#define TXD2 17

int Nilai[500];
int nilaike = 0;

String NilaiString;
String hexString;

int load;

int statusRufer;
unsigned long the_current_time=0;
unsigned long time_update_check=0;
unsigned long time_update_interval= 60 * 1000;

void updateTheTime(){
  the_current_time = pTime();
  time_update_check = millis();
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  // // device1
  //  urutX = 10;
  //  urutY = 12;
  // device2
  //  urutX = 24;
  //  urutY = 26;
  // device3
  urutX = 38;
  urutY = 40;
  pinMode(keyPin, INPUT_PULLUP);
  pinMode(mechPin, INPUT_PULLUP);
  pinMode(pbpin, INPUT_PULLUP);
  if (true)
  {
    if (true)
    { // read EEPROM for node code, register mark, wifi ssid and wifi password
      setEEPROM();
      readEEPROMProfile();
    }

    if (true)
    {
      setButton();
      setWifiProfile();
      enableFreeWifi(); // connect free wifi if main wifi unavailable
      // setSecureFunction(autoRelay);                  // function runned while reconnect internet
    }

    if (register_mark != true)
    {
      apnMode();
    }

    if (true)
    {                              // add topic
      addInTopic("flux/command/"); // automatically converted to "topic/nodecode"
      addOutTopic("flux/general_json/");
      addFeedBackTopic("flux/feedback/"); // automatically converted to "topic/nodecode"
      setPingFunction(sendData);
      setOTA();
    }

    if (true)
    {
      setCommandCode(0, deviceRelaySwitch); // callback with EEPROM
      setCommandCode(1, devicePing);        // cmd 2-5 already preserved
      setCommandCode(4, deviceSwitchConnection);
      setCommandCode(6, deviceUnregisterNew);
      setCommandCode(7, deviceRandomFunction);
      setCommandCode(10, ResetStep);
    }

    while (net_mark == false)
    { // connect to wifi and mqtt
      reconnectAttempt();

      if (digitalRead(pbpin) == 0)
      {
        Serial.print("reset");
        eepromWriteInt32(epromSave, 0);
        mechStep = 0;
        mechStepMinutes = 0;
        while (digitalRead(pbpin) == 0)
        {
        }
        Serial.println(" ok");
      }
    }
  }
  mechStepMinutes = eepromGetInt32(epromSave);
  mechStep = eepromGetInt32(epromSave);
  updateTheTime();
}

void loop()
{
  ptr_MQTT->loop();

  connectivityEstablishment();

  if (isFirst)
  {
    if(millis() - time_update_check > time_update_interval){
      updateTheTime();
    }

    if (millis() - daq_check > 1000)
    {
      if (digitalRead(mechPin) == 0)
      {
        mechStep++;


        
      if (mechStep >= mechStepMinutes)
      {
        mechStepMinutes = mechStepMinutes + 30;
        Serial.print(mechStepMinutes);
        eepromWriteInt32(epromSave, mechStepMinutes);
      }
      }
      uwbRead();
      sendData();
      daq_check = millis();


    }

    keyStatus = !digitalRead(keyPin);

    // if (digitalRead(mechPin) == 0)
    // {
    //   if (millis() - lastMilis > 1000)
    //   {
    //     lastMilis = millis();
    //     mechStep++;
    //   }

    //   if (mechStep >= mechStepMinutes)
    //   {
    //     mechStepMinutes = mechStepMinutes + 30;
    //     Serial.print(mechStepMinutes);
    //     eepromWriteInt32(epromSave, mechStepMinutes);
    //   }
    // }

    // if (millis() - lastMilis2 > 1000)
    // {
    //   uwbRead();
      // lastMilis2 = millis();
      // Serial.print("key : ");
      // Serial.println(keyStatus);
      // Serial.print("step : ");
      // Serial.println(mechStep);
    // }

    if (digitalRead(pbpin) == 0)
    {
      Serial.print("reset");
      eepromWriteInt32(epromSave, 0);
      mechStep = 0;
      mechStepMinutes = 0;
      while (digitalRead(pbpin) == 0)
      {
      }
      Serial.println(" ok");
    }
  }

  // buttonPress();
}

void uwbRead()
{

  for (int o = 0; o < 3;)
  {

    o++;
    if (Serial2.available())
    {
      // Membaca sejumlah byte dari Serial
      byte buffer[64]; // Buffer untuk menyimpan data
      int bytesRead = Serial2.readBytes(buffer, sizeof(buffer));

      // Mengonversi setiap byte dalam buffer menjadi hexadecimal dan mencetaknya
      for (int i = 0; i < bytesRead; i++)
      {
        String hexString = String(buffer[i], HEX);
        if (hexString.length() < 2)
        {
          hexString = "0" + hexString; // Menambahkan 0 di depan jika perlu
        }

        if (hexString == "01" || load >= 1)
        {
          if (hexString == "83" || load >= 2)
          {
            load = 2;
            // Serial.print(hexString);
            // Serial.print(" ");
            // Serial.print(hexToDec(hexString));
            // Serial.print(" ");
            nilaike++;
            Nilai[nilaike] = hexToDec(hexString);
          }
          else
          {
            load = 1;
          }
        }
      }
    }
    else
    {
      statusRufer++;
    }
    Xvalue = Nilai[urutX];
    Yvalue = Nilai[urutY];
    if (statusRufer >= 3)
    {
      Xvalue = 0;
      Yvalue = 0;
    }
    // if (Nilai[10] > 0 && Nilai[12] > 0)
    // {
    // Serial.println();
    // Serial.println("Forklift 1 :");
    // Serial.print("X Axis ");
    // Serial.println(Nilai[urutX]);
    // Serial.print("Y Axis ");
    // Serial.println(Nilai[urutY]);
    // Serial.println();
    // }
  }
  load = 0;
  nilaike = 0;
  statusRufer = 0;
}

int hexToDec(String hexString)
{
  int decimalValue = 0;

  // Loop melalui setiap karakter dalam string hexadecimal
  for (int i = 0; i < hexString.length(); i++)
  {
    char hexChar = hexString.charAt(i);
    int hexValue = 0;

    // Mengkonversi karakter hexadecimal ke nilai integer
    if (hexChar >= '0' && hexChar <= '9')
    {
      hexValue = hexChar - '0';
    }
    else if (hexChar >= 'A' && hexChar <= 'F')
    {
      hexValue = 10 + (hexChar - 'A');
    }
    else if (hexChar >= 'a' && hexChar <= 'f')
    {
      hexValue = 10 + (hexChar - 'a');
    }

    // Menghitung nilai decimal
    decimalValue = (decimalValue * 16) + hexValue;
  }

  return decimalValue;
}

///////mqtt
void autoRelay()
{
}

void sendData()
{
  unsigned int _data_length = 300;
  DynamicJsonDocument _doc(_data_length);
  String _txt;
  _doc["nodeCode"] = node_code;
  // _doc["time"] = pTime();
  _doc["time"] = the_current_time + ((millis() - time_update_check)/1000);
  _doc["0"] = getCurrentRSSI();
  _doc["1"] = keyStatus;
  _doc["2"] = mechStepMinutes;
  _doc["3"] = Xvalue;
  _doc["4"] = Yvalue;
  serializeJsonPretty(_doc, _txt);
  ptr_MQTT->publish(out_topic, _txt.c_str());
  // Serial.println(_txt);
  // delay(200);
  // Serial.println("Send Data");
}

void controlRelay(int _valOrder, int _condition)
{

  // delay(1000);
}

void deviceRelaySwitch(DynamicJsonDocument source_doc)
{
  Serial.println("Income command");
  int _cmd_code = source_doc["cmdCode"];
  int _the_order = source_doc["valOrder"];
  int _the_value = source_doc["value"];
  String _uid = source_doc["uid"];
  int _relay_start_parameter_order = 1; // lowest is 0
  // controlRelay(_the_order - _relay_start_parameter_order,_the_value);
  DynamicJsonDocument _doc(200);
  String _txt;
  _doc["cmdCode"] = _cmd_code;
  _doc["status"] = "true";
  _doc["uid"] = _uid;
  serializeJsonPretty(_doc, _txt);
  ptr_MQTT->publish(feed_back_topic, _txt.c_str());
  sendData();
}

void deviceSwitchConnection(DynamicJsonDocument source_doc)
{
  int _connection = source_doc["connection"];

  if (_connection == CONNECT2LAN)
  {
    if (connectivity_mark != CONNECT2LAN)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pConnectLAN();
    }
  }
  else if (_connection == CONNECT2WIFI)
  {
    if (connectivity_mark != CONNECT2WIFI)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pConnectWifi(local_wifi_ssid, local_wifi_pass);
    }
  }
  else if (_connection == CONNECT2GSM)
  {
    if (connectivity_mark != CONNECT2GSM)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pReconnectGPRS();
    }
  }
  else
  {
    if (connectivity_mark != CONNECT2WIFI)
    {
      wifi_mark = false;
      net_mark = false;
      free_mark = false;
      mqtt_mark = false;
      pFreeWifi();
    }
  }
  pSecureFunction();
  if (net_mark == true)
  {
    if (mqtt_mark == false)
    {
      connectMQTT();
    }
  }
}

void devicePing(DynamicJsonDocument source_doc)
{
  sendData();
}

void deviceRandomFunction(DynamicJsonDocument source_doc)
{
}
////mqtt

void ResetStep(DynamicJsonDocument source_doc)
{
  int _cmd_code = source_doc["cmdCode"];
  int _reset = source_doc["reset"];
  if (_reset = 1){
      Serial.print("reset");
      eepromWriteInt32(epromSave, 0);
      mechStep = 0;
      mechStepMinutes = 0;
  }
}