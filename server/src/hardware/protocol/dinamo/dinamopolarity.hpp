#ifndef ASD
#define ASD

#include <cstdint>

namespace Dinamo {

enum class Polarity : int8_t
{
  Negative = -1,
  Positive = 1,
};

constexpr std::string_view toString(const Dinamo::Polarity value)
{
  switch(value)
  {
    using enum Dinamo::Polarity;

    case Negative:
      return "negative";

    case Positive:
      return "positive";

    default: [[unlikely]]
      return {};
  }
}

}

constexpr Dinamo::Polarity operator~(const Dinamo::Polarity value) noexcept
{
  return static_cast<Dinamo::Polarity>(-static_cast<int8_t>(value));
}

#endif
