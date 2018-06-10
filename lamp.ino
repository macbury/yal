#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "credentials.h"

const byte TICK_DELAY = 33;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN_LED_STRIP, NEO_GRB + NEO_KHZ800);

#include "ha.h"
#include "effects.h"
#include "state.h"
#include "connection.h"

void on_mqtt_message(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);
  /**
   * Transform this to more normal stuff like array of char not a bytefuck
   */
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  if (processJson(message)) {
    sendCurrentState();
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  randomSeed(100);

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(on_mqtt_message);

  strip.begin();
  delay(100);
  Effect * effect = new ManualEffect();
  currentState = {
    255, 255, 255,
    0, false, effect
  };
}

void loop() {
  if (client.connected()) {
    client.loop();
    ArduinoOTA.handle();
    handleLight();
  } else {
    if (ensureMqttConnection()) {
      onConnect();
    }
  }

  delay(TICK_DELAY);
}
