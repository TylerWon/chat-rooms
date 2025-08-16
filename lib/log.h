    #define LOG_LEVEL_DEBUG 0
    #define LOG_LEVEL_INFO 1
    #define LOG_LEVEL_WARN 2
    #define LOG_LEVEL_ERROR 3

    #define CURRENT_LOG_LEVEL LOG_LEVEL_INFO // Set desired log level

    #if CURRENT_LOG_LEVEL <= LOG_LEVEL_DEBUG
    #define LOG_DEBUG(fmt, ...) fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #else
    #define LOG_DEBUG(fmt, ...) do {} while(0) // Empty macro
    #endif

    #if CURRENT_LOG_LEVEL <= LOG_LEVEL_INFO
    #define LOG_INFO(fmt, ...) fprintf(stderr, "[INFO] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #else
    #define LOG_DEBUG(fmt, ...) do {} while(0) // Empty macro
    #endif

    #if CURRENT_LOG_LEVEL <= LOG_LEVEL_WARN
    #define LOG_WARN(fmt, ...) fprintf(stderr, "[WARN] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #else
    #define LOG_DEBUG(fmt, ...) do {} while(0) // Empty macro
    #endif

    #if CURRENT_LOG_LEVEL <= LOG_LEVEL_ERROR
    #define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
    #else
    #define LOG_DEBUG(fmt, ...) do {} while(0) // Empty macro
    #endif