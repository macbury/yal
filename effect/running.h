#ifndef RunningEffect_H
#define RunningEffect_H
#include "Effect.h"

RGB runningMainColor = { 30, 161, 255 };
RGB runningSecondColor = { 255, 255, 255 };

class RunningEffect : public Effect {
  public:
    virtual void update() {
      _time += 0.1f;
      for (byte i = 0; i < size(); i++) {
        float alpha = 0.5f + sin(i + _time) * 0.5f;
        matrix[i] = lerpColor(alpha, runningMainColor, runningSecondColor);
      }
    }

    virtual String name() {
      return "Running";
    }
  private:
    float _time;
};

#endif
