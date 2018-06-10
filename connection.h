void printWifiInfo() {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupOTA() {
  Serial.println("Configuring ArduinoOTA");
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(OTA_HOST);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("Starting");
    clearColor();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  bool sec = false;
  while (true) {
    Serial.println("Connecting to: ");
    if (sec) {
      WiFi.begin(SECOND_WIFI_NAME, SECOND_WIFI_PASSWORD);
      Serial.println(SECOND_WIFI_NAME);
    } else {
      WiFi.begin(FIRST_WIFI_NAME, FIRST_WIFI_PASSWORD);
      Serial.println(FIRST_WIFI_NAME);
    }

    sec = !sec;
    for (int i = 0; i < 2; i++) {
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        randomSeed(micros());
        printWifiInfo();
        setupOTA();
        return;
      }
      delay(100);
      yield();
    }
  }
}

void ensureWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print("WIFI Disconnected. Attempting reconnection.");
    setupWifi();
  }
}

bool ensureMqttConnection() {
  ensureWifiConnection();
  bool ledTick = false;
  while (!client.connected()) {
    yield();
    ledTick = !ledTick;
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "lampka-";
    clientId += String(random(0xffff), HEX);
    Serial.print("Client id: ");
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      printWifiInfo();
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 500 ms");
      delay(500);
    }
  }

  return false;
}

void onConnect() {
  Serial.println("Subscribing: ");

  Serial.println(MQTT_TOPIC_COMMAND);
  client.subscribe(MQTT_TOPIC_COMMAND);

  sendCurrentState();
}

int payloadToInt(byte* payload, unsigned int length) {
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  return String(message).toInt();
}
