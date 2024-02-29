#include <Arduino.h>

#include "secret.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPIFFS.h>

#define LED_PIN 2

#define TL2C_SLEEP 35
#define TL2C_RELAY_INT 32
#define TL2C_ZONE1_BUTTON 32
#define TL2C_ZONE2_BUTTON 25
#define TL2C_ZONE3_BUTTON 26
#define TL2C_ZONE1_ACTIVE 14
#define TL2C_ZONE2_ACTIVE 13
#define TL2C_ZONE3_ACTIVE 15

#define TL2C_I2C_ADDR 0x40
#define TL2C_STATUS_REG 0
#define TL2C_CONFIG_REG 1
#define TL2C_ZONE1_ON_DELAY 2
#define TL2C_ZONE2_ON_DELAY 3
#define TL2C_ZONE3_ON_DELAY 4
#define TL2C_FIRMWARE_VERSION 5

#define I2C_PIN_SDA 21
#define I2C_PIN_SCL 22

#define I2C_RESPONSE_SUCCESS 0
#define I2C_RESPONSE_BUFFER_OVERFLOW 1
#define I2C_RESPONSE_NOT_FOUND 2
#define I2C_RESPONSE_BAD_DATA 3
#define I2C_RESPONSE_ERROR 4

enum
{
  ZONE1 = 0,
  ZONE2,
  ZONE3,
} Zone;

typedef struct
{
  bool enabled[3];
  bool active[3];
  bool relay_int;
  int delay[3];
} TL2C_State_type;

typedef struct {
  uint8_t state;
  uint8_t config;
  uint8_t zone_delay[3];
} TL2C_registers;

volatile TL2C_State_type tl2c_state;
volatile TL2C_registers tl2c_registers;



AsyncWebServer serverWiFi(80);

String txtModul, txtEin, txtAus;

int tl2c_address;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Seite nicht gefunden");
}


void print_binary(int v, int num_places)
{
  int mask = 0, n;

  for (n = 1; n <= num_places; n++)
  {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask; // truncate v to specified number of places

  while (num_places)
  {

    if (v & (0x0001 << num_places - 1))
    {
      Serial.print("1");
    }
    else
    {
      Serial.print("0");
    }

    --num_places;
    if (((num_places % 4) == 0) && (num_places != 0))
    {
      Serial.print("_");
    }
  }
}

// Render the place holder variables.
String processor(const String &var)
{
  if (var == "ZONE3-COLOUR")
  {
    if (tl2c_state.enabled[ZONE3])
    {
      if(tl2c_state.active[ZONE3]){
        return F("redButton");
      } else {
        return F("greenButton");
      }
    }
    else
    {
      return F("greyButton");
    }
  }

  if (var == "ZONE2-COLOUR")
  {
    if (tl2c_state.enabled[ZONE2])
    {
      if(tl2c_state.active[ZONE2]){
        return F("redButton");
      } else {
        return F("greenButton");
      }
    }
    else
    {
      return F("greyButton");
    }
  }

  if (var == "ZONE1-COLOUR")
  {
    if (tl2c_state.enabled[ZONE1])
    {
      if(tl2c_state.active[ZONE1]){
        return F("redButton");
      } else {
        return F("greenButton");
      }
    }
    else
    {
      return F("greyButton");
    }
  }

  if (var == "ZONE3-ENABLE")
  {
    if (tl2c_state.enabled[ZONE3])
    {
      return F("TRUE");
    }
    else
    {
      return F("FALSE");
    }
  }

  if (var == "ZONE2-ENABLE")
  {
    if (tl2c_state.enabled[ZONE2])
    {
      return F("TRUE");
    }
    else
    {
      return F("FALSE");
    }
  }
  if (var == "ZONE1-ENABLE")
  {
    if (tl2c_state.enabled[ZONE1])
    {
      return F("TRUE");
    }
    else
    {
      return F("FALSE");
    }
  }


  if (var == "ZONE3-DELAY")
  {
    return String(tl2c_state.delay[ZONE3]);
  }

  if (var == "ZONE2-DELAY")
  {
    return String(tl2c_state.delay[ZONE2]);
  }

  if (var == "ZONE1-DELAY")
  {
    return String(tl2c_state.delay[ZONE1]);
  }
  return String(); // Fehler => leerer String
}



void handle_config_rest_post(AsyncWebServerRequest *request) {

}


int locate_tl2c()
{
  Serial.println("Locate TL2C Begin \n");
  int nDevices = 0;
  int tl2c_addr = 0;
  byte error;
  for (int i = 1; i < 128; i++)
  {
    Wire.beginTransmission(i);
    error = Wire.endTransmission(true);
    if (error == I2C_RESPONSE_SUCCESS)
    {
      tl2c_addr = i;
      Serial.printf("TL2C found at Address %02x \n", i);
      nDevices++;
      break;
    }
  } // end for

  if (nDevices == 0)
  {
    Serial.println("No TL2C found on the I2C bus\n");
  }
  else
  {
    Serial.println("Locate TL2C End \n");
  }
  return tl2c_addr;
}

/**
 * @brief Reads the registers from the TL2C
 *
 * @return int
 */

volatile bool tl2c_state_change = false;

/**
 * @brief The interrupt hander for a relay state change
 *
 */
void IRAM_ATTR tl2c_state_change_isr()
{
  tl2c_state_change = true;
}

int read_tl2c_register(int reg)
{
  Serial.printf("Reading the %02x register \n", reg);

  Wire.beginTransmission(tl2c_address);
  Wire.write(reg);
  int response = Wire.endTransmission();
  int result = -1;
  if (response == I2C_RESPONSE_SUCCESS)
  {
    Wire.requestFrom(tl2c_address, 1);
    while (Wire.available())
    {
      result = Wire.read();
    }
    Serial.print("Response: 0b");
    Serial.println(result, BIN);
  }
  else
  {
    Serial.printf("Failed to ready the I2C bus addr: %02x, response: %02x \n", tl2c_address, response);
  }
  return result;
}

int read_tl2c_state()
{
  return read_tl2c_register(TL2C_STATUS_REG);
}

int read_tl2c_config()
{
  return read_tl2c_register(TL2C_CONFIG_REG) >> 4;
}

int read_tl2c_zone_delay(int zone)
{
  int result = -1;
  switch (zone)
  {
  case ZONE1:
    result = read_tl2c_register(TL2C_ZONE1_ON_DELAY);
    break;
  case ZONE2:
    result = read_tl2c_register(TL2C_ZONE2_ON_DELAY);
    break;
  case ZONE3:
    result = read_tl2c_register(TL2C_ZONE3_ON_DELAY);
    break;
  default:
    result = -1;
    break;
  }
  return result;
}

void read_tl2c()
{
    tl2c_state_change = false;
    // Get the state of the TL2C
    Serial.println("State change detected");
    tl2c_registers.state = read_tl2c_state();
    if (tl2c_registers.state > 0)
    {
      Serial.printf("The state was read OK: 0b");
      print_binary(tl2c_registers.state, 8);
      Serial.println();
      int v = tl2c_registers.state;
      for( int bit = 0; bit < 3; bit++){
        if( v & (1 << bit) ) {
          Serial.printf("Setting bit %d \n", bit);
          tl2c_state.active[bit] = true;
        } else
          tl2c_state.active[bit] = false;
      }
      if ( v & (1 << 3)) {
        Serial.println("The relay bit is set");
        tl2c_state.relay_int = true;
      } else {
        tl2c_state.relay_int = false;
      }
    }
    else
    {
      Serial.println("Reading the state failed");
    }

    tl2c_registers.config = read_tl2c_config();
    if(tl2c_registers.config > 0 )
    {
      Serial.printf("The config was read OK: 0b");
      print_binary(tl2c_registers.config, 8);
      Serial.println();
      int v = tl2c_registers.config;
      for( int bit = 0; bit < 3; bit++){
        if( v & (1 << bit) ) {
          Serial.printf("Setting bit %d \n", bit);
          tl2c_state.enabled[bit] = true;
        } else
          tl2c_state.enabled[bit] = false;
      }
    }
    else
    {
      Serial.println("Reading the config failed");
    }
    for (int z = 0; z < 3; z++){
      tl2c_registers.zone_delay[z] = read_tl2c_zone_delay( z );
      if(tl2c_registers.zone_delay[z] > 0 )
      {
        Serial.printf("The zone %d delay was read OK: %d \n", z, tl2c_registers.zone_delay[z]);
        Serial.println();
        tl2c_state.delay[z] = tl2c_registers.zone_delay[z];
      }
      else
      {
        Serial.printf("Reading the zone %d delay failed \n", z);
      }

    }
}

void write_register(uint8_t reg, uint8_t value) {
  Serial.printf("Writing to TL2C Reg: %d, value %d \n", reg, value);
    Wire.beginTransmission(tl2c_address);  
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();    
  Serial.println("Writing end");
}


void write_config()
{
    write_register(TL2C_CONFIG_REG,  tl2c_registers.config<<4);
}

void write_zone_delay(int zone, uint8_t delay){
  switch (zone)
  {
  case ZONE1:
    write_register(TL2C_ZONE1_ON_DELAY, tl2c_registers.zone_delay[ZONE1]);
    break;
  case ZONE2:
    write_register(TL2C_ZONE2_ON_DELAY, tl2c_registers.zone_delay[ZONE2]);
    break;
  case ZONE3:
    write_register(TL2C_ZONE3_ON_DELAY, tl2c_registers.zone_delay[ZONE3]);
    break;
  default:
    break;
  }

}



void write_tl2c() {
  // tl2c_state -> tl2c_registers
  // Then use Wire to write to tl2c_address
  Serial.println("write_tl2c begin");
  uint8_t config = 0;
  for (int zone = 0; zone < 3; zone++){ 
    if( tl2c_state.enabled[zone] ){
      config = config | (1<<zone);
    }
    tl2c_registers.zone_delay[zone] = tl2c_state.delay[zone];
    write_zone_delay(zone, tl2c_state.delay[zone]);
  }
  tl2c_registers.config = config;
  write_config();
  Serial.println("write_tl2c end");
}

String asString(bool enabled, bool active) {
  String result = "OFF"; //Grey
  if ( enabled ){
    if( active )
      result = "ACTIVE"; // Red
    else
      result = "ENABLED"; // Green
  }
  return result;
}

void handle_get_zone_state(AsyncWebServerRequest *request){
  String json = "[";
  json += "{\"zone\": {\"id\": \"1\", \"state\": \""+asString(tl2c_state.enabled[0], tl2c_state.active[0])+"\"}},";
  json += "{\"zone\": {\"id\": \"2\", \"state\": \""+asString(tl2c_state.enabled[1], tl2c_state.active[1])+"\"}},";
  json += "{\"zone\": {\"id\": \"3\", \"state\": \""+asString(tl2c_state.enabled[2], tl2c_state.active[2])+"\"}}";
  json += "]";

  request->send(200, "application/json", json);
}


void handle_zone_form_post(AsyncWebServerRequest *request){
  Serial.println("handle_zone_form_post: Begin");

  int params = request->params();
  int zoneId = -1;
  bool enabled = false;
  bool updateEnable = false;
  int zoneDelay = 0;
  for(int i=0; i<params; i++){
      AsyncWebParameter* p = request->getParam(i);
      Serial.printf("name: %s, value: %s \n", p->name(), p->value());
      if (  p->name() == "zone-id" ){
        zoneId = p->value().toInt();
        zoneId--;
        Serial.printf("Setting the zoneID: %s to: %d, ", p->name(), zoneId);
      }
      if ( p->name() == "zone-enable" ){
        enabled = !(p->value().equals("TRUE"));
        if( enabled )
          Serial.printf("setting the enable flag %s to: TRUE \n", p->name());
        else
          Serial.printf("setting the enable flag %s to: FALSE \n", p->name());
      }
      if( p->name() == "update-enable"){
        Serial.printf("Setting the update-enable to %s \n", p->value());
        updateEnable = p->value().equals("true");
      }
      if(p->name() == "zone-delay"){
        Serial.printf("Setting the zone delay to %s \n", p->value());
        zoneDelay = p->value().toInt();
      }
  }
  Serial.printf("The zoneId: %d \n", zoneId);
  if (zoneId > -1 && updateEnable ){
    Serial.println("Updating the tl2c register for the zone enable");
    tl2c_state.enabled[zoneId] = enabled;
    // Write the config
  }
  if( zoneId > -1 && zoneDelay > 0 ){
    Serial.println("Updating the tl2c register for the delay");
    tl2c_state.delay[zoneId] = zoneDelay;
  }
  write_tl2c();
  request->send(SPIFFS,  "/index.html", String(), false, processor);  
  Serial.println("handle_zone_form_post: End");
}


void setup_gpio()
{
  Serial.println("Starting GPIO setup");

  pinMode(TL2C_SLEEP, OUTPUT);
  pinMode(TL2C_RELAY_INT, INPUT_PULLUP);
  pinMode(TL2C_ZONE1_BUTTON, INPUT_PULLUP);
  pinMode(TL2C_ZONE2_BUTTON, INPUT_PULLUP);
  pinMode(TL2C_ZONE3_BUTTON, INPUT_PULLUP);
  pinMode(TL2C_ZONE1_ACTIVE, OUTPUT);
  pinMode(TL2C_ZONE2_ACTIVE, OUTPUT);
  pinMode(TL2C_ZONE3_ACTIVE, OUTPUT);

  digitalWrite(TL2C_SLEEP, HIGH);
  digitalWrite(TL2C_ZONE1_ACTIVE, LOW);
  digitalWrite(TL2C_ZONE1_ACTIVE, LOW);
  digitalWrite(TL2C_ZONE1_ACTIVE, LOW);
  attachInterrupt(digitalPinToInterrupt(TL2C_RELAY_INT), tl2c_state_change_isr, RISING);
  Serial.println("Completed GPIO setup");
}

void setup_server()
{
  Serial.println("Setting up the server");

  Serial.print("Connecting ... ");
  Serial.println(TL2C_WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(TL2C_WLAN_HOST);
  WiFi.begin(TL2C_WLAN_SSID, TL2C_WLAN_PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(""); // Verbindung aufgebaut
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Start WiFi Server!");
  serverWiFi.begin();

  serverWiFi.onNotFound(notFound);

  // Methode für root / web page
  serverWiFi.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { Serial.println("GET /");
                  request->send(SPIFFS, "/index.html", String(), false, processor); 
                  });

  // Methode für Laden der style.css-Datei
  serverWiFi.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });
  // Methode für Laden der jquery-3.7.1.min.js-Datei
  serverWiFi.on("/jquery-3.7.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/jquery-3.7.1.min.js", "text/javascript"); });
  // Callback für Laden der java.js-Datei
  serverWiFi.on("/java.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/java.js", "text/javascript"); });
  // Callback für Laden der java.js-Datei
  serverWiFi.on("/WhiteOnBlak_logo.png", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/WhiteOnBlak_logo.png", "image/png"); });

  // serverWiFi.on("/api/v1/config", HTTP_POST, handle_config_rest_post);

  serverWiFi.on("/state", HTTP_GET, handle_get_zone_state);

  serverWiFi.on("/", HTTP_POST, handle_zone_form_post);

  Serial.println("Server setup complete");
}

/**
 * @brief
 *
 */
void setup()
{
  Serial.println("Starting setup");

  setup_gpio();

  tl2c_state.active[ZONE3] = false;
  tl2c_state.active[ZONE2] = true;
  tl2c_state.active[ZONE1] = true;

  tl2c_state.delay[ZONE3] = 5;
  tl2c_state.delay[ZONE2] = 15;
  tl2c_state.delay[ZONE1] = 20;

  Serial.begin(9600);

  setup_server();

  if (!SPIFFS.begin(true))
  {
    Serial.println("ERROR: The SPIFFS could not be mounted");
    return;
  }

  Wire.begin(I2C_PIN_SDA, I2C_PIN_SCL, 100000UL); // Channel 1 I2C
  Serial.println("Locating the TL2C");
  tl2c_address = locate_tl2c();
  Serial.printf("TL2C Address: %02x \n", tl2c_address);
  Serial.println("Setup complete");

  delay(200);

  read_tl2c();

}

void loop()
{
  if (tl2c_state_change)
  {
    // TODO - Get the state first and if there is a change in the 
    // relay interrupt state then do the rest.
    read_tl2c();
  }
  delay(1);
}
