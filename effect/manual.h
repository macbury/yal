#ifndef ManualEffect_H
#define ManualEffect_H
#include "Effect.h"

class ManualEffect : public Effect {
  public:
    RGB currentColor;
    RGB targetColor;
    float alpha;

    virtual void init() {
      currentColor = { 255, 255, 255 };
      alpha = 1;
    }

    virtual void update() {
      if (targetColor.r != currentState.red || targetColor.g != currentState.green || targetColor.b != currentState.blue) {
        alpha = 0.0;
        targetColor.r = currentState.red;
        targetColor.g = currentState.green;
        targetColor.b = currentState.blue;
      }
      alpha += 0.05;

      if (alpha >= 1.0) {
        currentColor.r = targetColor.r;
        currentColor.g = targetColor.g;
        currentColor.b = targetColor.b;
        alpha = 1.0;
      }

      RGB color = lerpColor(alpha, currentColor, targetColor);
      for (int i = 0; i < size(); i++) {
        matrix[i] = color;
      }
    }

    virtual String name() {
      return "Manual";
    }
};

#endif
