#pragma once

#include "Port.hpp"

namespace phnq
{
  namespace engine
  {
    struct AudioOut : Port<float>
    {
      void setValue(float value) override
      {
        Port::setValue(daisysp::fclamp(value, -2.f, 2.f));
      }
    };
  }
}
