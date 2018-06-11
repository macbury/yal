const char* CMD_ON = "ON";
const char* CMD_OFF = "OFF";

typedef struct {
  byte red;
  byte green;
  byte blue;
  byte brightness;
  boolean enabled;
  Effect * effect;
} LampState;

LampState currentState;

#include "effect/manual.h"

int tempBrightness = 0;
void handleBrightness() {
  int target = currentState.enabled ? currentState.brightness : 0;
  int direction = tempBrightness - target;
  if (direction > 0) {
    tempBrightness--;
  } else if (direction < 0) {
    tempBrightness++;
  }

  strip.setBrightness(tempBrightness);
}

void transitToEffect(Effect * fromEffect, Effect * toEffect, float transitionTime) {
  float step = (TICK_DELAY/transitionTime);
  for (float alpha = 0; alpha <= 1.0; alpha+=step) {
    fromEffect->update();
    toEffect->update();
    handleBrightness();

    for(byte i=0; i<strip.numPixels(); i++) {
      RGB currentColor = fromEffect->get(i);
      RGB nextColor = toEffect->get(i);

      strip.setPixelColor(
        i,
        strip.Color(
          lerp(alpha, currentColor.r, nextColor.r),
          lerp(alpha, currentColor.g, nextColor.g),
          lerp(alpha, currentColor.b, nextColor.b)
        )
      );
    }

    strip.show();
    yield();
    delay(TICK_DELAY);
  }
}

void runEffect(Effect * toRunEffect, Effect * fromEffect, int duration) {
  transitToEffect(fromEffect, toRunEffect, 1000);
  while(duration >= 0) {
    toRunEffect->update();

    for(byte i=0; i<strip.numPixels(); i++) {
      RGB currentColor = toRunEffect->get(i);
      strip.setPixelColor(i, strip.Color(currentColor.r, currentColor.g, currentColor.b));
    }

    strip.show();
    duration -= TICK_DELAY;
    yield();
    delay(TICK_DELAY);
  }

  transitToEffect(toRunEffect, fromEffect, 1000);
}


void handleLight() {
  handleBrightness();

  currentState.effect->update();
  for(byte i=0; i<strip.numPixels(); i++) {
    RGB color = currentState.effect->get(i);
    strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
  }
  strip.show();
}

/**
 * Inform mqtt component in home assistant about light state
 */
void sendCurrentState() {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["state"] = currentState.enabled ? CMD_ON : CMD_OFF;

  root["brightness"] = currentState.brightness;
  root["effect"] = currentState.effect->name();

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  Serial.println("Sending current state:");
  Serial.println(buffer);
  client.publish(MQTT_TOPIC_STATE, buffer, true);
}

void setEffectByName(String name) {
  if (name != currentState.effect->name()) {
    Effect * targetEffect = NULL;
    Serial.print("Changing effect to: ");
    if (name == "Manual") {
      Serial.println("Manual");
      targetEffect = new ManualEffect();
    } else  if (name == "Failed") {
      Serial.println("Failed");
      targetEffect = new FailedEffect();
    } else if (name == "Running") {
      Serial.println("Running");
      targetEffect = new RunningEffect();
    } else if (name == "Success") {
      Serial.println("Success");
      targetEffect = new SuccessEffect();
    } else {
      Serial.println("Clear");
      targetEffect = new ClearEffect();
    }
    transitToEffect(currentState.effect, targetEffect, 1000);
    delete currentState.effect;
    currentState.effect = targetEffect;
  }
}
/**
 * Parse out everything about action
 */
boolean processJson(char * rawJson) {
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(rawJson);

  if (!root.success()) {
    Serial.println(rawJson);
    Serial.println("json is fucked up!");
    return false;
  }

  bool nextSwitchState = root["state"] == CMD_ON;
  if (!currentState.enabled && nextSwitchState && currentState.brightness == 0) {
    currentState.brightness = 150;
  }
  currentState.enabled = nextSwitchState;

  if (root.containsKey("brightness")) {
    currentState.brightness = root["brightness"];
  }

  if (root.containsKey("color")) {
    currentState.red = root["color"]["r"];
    currentState.green = root["color"]["g"];
    currentState.blue = root["color"]["b"];
  }

  if (root.containsKey("effect")) {
    setEffectByName(root["effect"]);
  }

  return true;
}
