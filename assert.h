void panic(const char *message, const char *file, int line);

#define assert(condition)                                  \
    if (!(condition)) {                                    \
        panic("Assertion failed: " #condition, __FILE__, __LINE__); \
    }

#define fatal(msg)     panic("FATAL Error:" #msg, __FILE__, __LINE__);
    