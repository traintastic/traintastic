

#include <type_traits>
#ifndef  _MSC_VER
  #include <byteswap.h>
#endif

constexpr bool is_big_endian = false;
constexpr bool is_little_endian = true;

template<typename T>
inline T byte_swap(T value)
{
  static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
  if constexpr(sizeof(T) == 2)
#ifdef  _MSC_VER
    return _byteswap_ushort(value);
#else
    return bswap_16(value);
#endif
  else if constexpr(sizeof(T) == 4)
#ifdef  _MSC_VER
    return _byteswap_ulong(value);
#else
    return bswap_32(value);
#endif
  else if constexpr(sizeof(T) == 8)
#ifdef  _MSC_VER
    return _byteswap_uint64(value);
#else
    return bswap_64(value);
#endif
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
