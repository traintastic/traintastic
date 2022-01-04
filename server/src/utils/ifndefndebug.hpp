#ifndef IFN
#define IFN

#ifndef NDEBUG
  #define IFNDEF_NDEBUG(...) __VA_ARGS__
#else
  #define IFNDEF_NDEBUG(...)
#endif

#endif
