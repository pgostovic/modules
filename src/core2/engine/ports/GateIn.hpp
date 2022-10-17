#pragma once

#include "Port.hpp"

namespace phnq
{
  namespace engine
  {
    struct GateIn : Port<bool>
    {
      enum Type
      {
        CV,
        SWITCH,
      };

      struct GateChangeListener
      {
        virtual void gateValueDidChange(GateIn *port, bool value){};
      };

      void setValue(bool value) override
      {
        if (value != getValue())
        {
          Port::setValue(value);
          if (this->listener)
          {
            this->listener->gateValueDidChange(this, value);
          }
        }
      }

      GateIn *setListener(GateChangeListener *listener)
      {
        this->listener = listener;
        return this;
      }

      GateIn *setType(Type type)
      {
        this->type = type;
        return this;
      }

      Type getType()
      {
        return type;
      }

    private:
      GateChangeListener *listener = NULL;
      Type type = CV;
    };
  }
}
