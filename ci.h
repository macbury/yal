/*
* Caution! Implemented by drunken monkey!
*/

const char* CI_FINGERPRINT = "29 79 1C 39 D3 23 55 AA 45 C3 AA 86 7B 43 18 35 0F 38 33 E5";

WiFiClientSecure httpClient;
const int REFRESH_EVERY = 5000;
int ciUpdateStatus = REFRESH_EVERY;
bool fetchingCiStatus = false;
String ciStatusBody = "";
DynamicJsonBuffer ciStatusBuffer;

void handleDowloadedStatus() {
  fetchingCiStatus = false;
  if (currentState.effect->name() != "Manual") {
    JsonArray &array = ciStatusBuffer.parseArray(ciStatusBody);

    if (!array.success()) {
      Serial.println(ciStatusBody);
      Serial.println("json is fucked up!");
      ciStatusBody = "";
      return;
    }

    JsonObject &build = array.get<JsonObject>(0);
    if (build["status"] == String("success") || build["status"] == String("fixed")) {
      setEffectByName("Success");
    } else if (build["status"] == String("running") || build["status"] == String("queued")) {
      setEffectByName("Running");
    } else {
      setEffectByName("Failed");
    }
    sendCurrentState();
  }

  ciStatusBody = "";
}

void beginFetch() {
  fetchingCiStatus = true;
  ciStatusBody = "";
  char* host = "circleci.com";

  if (!httpClient.connect(host, 443)) {
    Serial.println("connection failed");
    return;
  }

  if (httpClient.verify(CI_FINGERPRINT, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
    return;
  }

  String url = "/api/v1.1/recent-builds?limit=1&circle-token="+String(CIRCLE_TOKEN);
  httpClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Accept: application/json\r\n" +
               "User-Agent: JustPlainDeskLamp\r\n" +
               "Connection: close\r\n\r\n");

  while (httpClient.connected()) {
    String line = httpClient.readStringUntil('\n');
    //Serial.println(line);
    yield();
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
}

void updateStatus() {
  if (fetchingCiStatus) {
    int maxToRead = 100;
    while(httpClient.available() && maxToRead > 0) {
      maxToRead--;
      int c = httpClient.read();
      if (c >= 0) {
        ciStatusBody += (char)c;
      }
      yield();
    }

    if (!httpClient.available()) {
      handleDowloadedStatus();
    }
  } else if (ciUpdateStatus >= REFRESH_EVERY) {
    beginFetch();
    ciUpdateStatus = 0;
  } else {
    ciUpdateStatus += TICK_DELAY;
  }
}
