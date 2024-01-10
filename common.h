#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>

#define MAX_PATH 256
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define ERRPROC(c, p)          \
    do                         \
    {                          \
        if (c)                 \
        {                      \
            p;                 \
            goto error_return; \
        }                      \
    } while (0)

#define ERRRET(c, s, ...)                                                                 \
    do                                                                                    \
    {                                                                                     \
        if (c)                                                                            \
        {                                                                                 \
            printf("%s(%d) %s " s "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
        }                                                                                 \
    } while (0)

#define d(s, ...)                                                                     \
    do                                                                                \
    {                                                                                 \
        printf("%s(%d) %s " s "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#endif