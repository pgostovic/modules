#pragma once

#include "Port.hpp"

namespace phnq
{
  namespace engine
  {
    struct CVIn : Port<float>
    {
      struct CVInChangeListener
      {
        virtual void cvInValueDidChange(CVIn *port, float value){};
      };

      void setValue(float value) override
      {
        value = daisysp::fclamp(value, -1.f, 1.f);

        if (abs(this->getValue() - value) > CV_CHANGE_THRESHOLD)
        {
          Port::setValue(value);
          if (this->listener)
          {
            this->listener->cvInValueDidChange(this, value);
          }
        }
      }

      virtual CVIn *setListener(CVInChangeListener *listener)
      {
        this->listener = listener;
        return this;
      }

    private:
      CVInChangeListener *listener = NULL;
    };
  }
}
