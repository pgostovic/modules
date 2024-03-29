#pragma once

#include "CVIn.hpp"

namespace phnq
{
  namespace engine
  {
    struct Param : CVIn
    {
      enum Type
      {
        PARAM,
        BUTTON,
      };

      Param *setListener(CVInChangeListener *listener) override
      {
        return static_cast<Param *>(CVIn::setListener(listener));
      }

      Param *setNumSteps(uint16_t numSteps)
      {
        this->numSteps = numSteps;
        return this;
      }

      void setValue(float value) override
      {
        if (numSteps > 1)
        {
          float interval = 1.f / (float)numSteps;
          value = interval * truncf(value / interval);
        }

        CVIn::setValue(value);
      }

      uint16_t getStepValue()
      {
        if (numSteps < 2)
        {
          return 0;
        }
        else
        {
          float interval = 1.f / (float)numSteps;
          uint16_t stepValue = (uint16_t)truncf(getValue() / interval);
          return stepValue < numSteps ? stepValue : numSteps - 1;
        }
      }

      virtual Type getType()
      {
        return PARAM;
      }

    private:
      uint16_t numSteps = 0;
    };
  }
}
