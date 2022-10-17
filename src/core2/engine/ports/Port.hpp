#pragma once

#include <queue>
#include <vector>
#include <daisysp.h>
#include "../Engine.hpp"

namespace phnq
{
  namespace engine
  {
    const float CV_CHANGE_THRESHOLD = 0.00001f;

    struct BasePort
    {
    };

    template <class T>
    struct Port : BasePort
    {
    private:
      std::string id;
      T value;
      uint16_t delay = 0;
      std::queue<T> delayBuffer;

    public:
      T getValue()
      {
        return this->value;
      }

      virtual void setValue(T value)
      {
        // Queue up the value change if there is a delay set.
        if (delay > 0)
        {
          delayBuffer.push(value);
          if (delayBuffer.size() < delay)
          {
            return;
          }
          value = delayBuffer.front();
          delayBuffer.pop();
        }

        this->value = value;
      }

      std::string getId()
      {
        return this->id;
      }

      auto setId(std::string id) -> Port<T> *
      {
        this->id = id;
        return this;
      }

      /**
       * @brief Set the number frames before a set value takes effect. This can
       * be useful when coordinating related input ports such as CV and Gate. The
       * CV value change may lag a bit so delaying the gate allows the CV to
       * settle before it's value is taken.
       *
       * @param delay number frames before a set value takes effect.
       * @return Port* for chainability.
       */
      auto setDelay(uint16_t delay) -> Port<T> *
      {
        this->delay = delay;
        return this;
      }
    };
  }
}
