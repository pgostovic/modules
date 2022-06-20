#pragma once

#include <math.h>

namespace phnq
{
  const float TRIGGER_DURATION_SECONDS = 0.001f;
  const float DEFAULT_SAMPLE_TIME = 1.f / 48000.f;

  struct Trigger
  {
  private:
    float sampleTime = DEFAULT_SAMPLE_TIME; // seconds
    unsigned int framesLeft = 0;

  public:
    void init(float sampleRate)
    {
      this->sampleTime = 1.f / sampleRate;
      this->framesLeft = 0;
    }

    /**
     * @brief Activate the trigger for some specified duration.
     *
     * @param duration time in seconds the trigger will remain active.
     */
    void activate(float duration = TRIGGER_DURATION_SECONDS)
    {
      this->framesLeft = (unsigned int)roundf(duration / sampleTime);
    }

    /**
     * @brief Advance by one sample frame.
     */
    bool process()
    {
      if (framesLeft > 0)
      {
        framesLeft--;
      }
      return isActive();
    }

    /**
     * @brief Return whether or not the trigger is currently active.
     *
     * @return true
     * @return false
     */
    bool isActive()
    {
      return framesLeft > 0;
    }
  };
}