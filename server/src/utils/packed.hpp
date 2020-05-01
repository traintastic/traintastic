
#ifndef TRAINTASTIC_SERVER_UTILS_PACKED_HPP
#define TRAINTASTIC_SERVER_UTILS_PACKED_HPP

#ifdef _MSC_VER
  #define ATTRIBUTE_PACKED
  #define PRAGMA_PACK_PUSH_1 __pragma(pack(push, 1))
  #define PRAGMA_PACK_POP __pragma(pack(pop))
#else
  #define ATTRIBUTE_PACKED __attribute__((packed))
  #define PRAGMA_PACK_PUSH_1
  #define PRAGMA_PACK_POP
#endif

#endif
