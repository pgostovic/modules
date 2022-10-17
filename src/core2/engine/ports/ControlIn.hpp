#pragma once

#include "Port.hpp"

namespace phnq
{
  namespace engine
  {
    struct ControlIn : Port<float>
    {
      struct ControlChangeListener
      {
        virtual void controlValueDidChange(ControlIn *port, float value){};
      };

      void setValue(float value) override
      {
        value = daisysp::fclamp(value, -1.f, 1.f);

        if (abs(this->getValue() - value) > CV_CHANGE_THRESHOLD)
        {
          Port::setValue(value);
          if (this->listener)
          {
            this->listener->controlValueDidChange(this, value);
          }
        }
      }

      virtual ControlIn *setListener(ControlChangeListener *listener)
      {
        this->listener = listener;
        return this;
      }

    private:
      ControlChangeListener *listener = NULL;
    };
  }
}
