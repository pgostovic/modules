#pragma once

#include "Port.hpp"

namespace phnq
{
  namespace engine
  {
    struct CVOut : Port<float>
    {
      void setValue(float value) override
      {
        Port::setValue(daisysp::fclamp(value, -1.f, 1.f));
      }
    };
  }
}
