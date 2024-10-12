void panic(const char *message, const char *file, int line);

#define assert(condition)                                  \
    if (!(condition)) {                                    \
        panic("Assertion failed: " #condition, __FILE__, __LINE__); \
    }
    