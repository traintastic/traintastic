

#include <type_traits>
#include <byteswap.h>

constexpr bool is_big_endian = false;
constexpr bool is_little_endian = true;

template<typename T>
inline T byte_swap(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  if constexpr(sizeof(T) == 2)
    return bswap_16(value);
  else if constexpr(sizeof(T) == 4)
    return bswap_32(value);
  else if constexpr(sizeof(T) == 8)
    return bswap_64(value);
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T host_to_le(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T le_to_host(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T host_to_be(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<typename T>
constexpr T be_to_host(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr(is_little_endian/*_v<sizeof(T)>*/)
    return byte_swap(value);
  else if constexpr(is_big_endian/*_v<sizeof(T)>*/)
    return value;
  else
    static_assert(sizeof(T) != sizeof(T));
}
