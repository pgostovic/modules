#pragma once

#include "Param.hpp"

namespace phnq
{
  namespace engine
  {
    struct Button : Param
    {
      struct ButtonChangeListener
      {
        virtual void buttonValueDidChange(Button *port, bool value){};
      };

      Button()
      {
        this->setNumSteps(2);
      }

      Button *setListener(ButtonChangeListener *listener)
      {
        this->listener = listener;
        return this;
      }

      void setBoolValue(bool value)
      {
        setValue(value ? 1.f : 0.f);
      }

      void setValue(float value) override
      {
        if (value != getValue() && this->listener)
        {
          Param::setValue(value);
          this->listener->buttonValueDidChange(this, this->getStepValue() == 1);
        }
      }

      Type getType() override
      {
        return BUTTON;
      }

    private:
      ButtonChangeListener *listener = NULL;
    };
  }
}
