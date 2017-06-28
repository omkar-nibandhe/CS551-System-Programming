#ifdef DO_TRACE

#define TRACE(...) \
  do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)

#else

#define TRACE(...) do { } while (0)

#endif
