#ifndef __ERRNO_H__
#define __ERRNO_H__

typedef enum { DEBUG = 0, INFO, WARNING, ERROR, CRITICAL } log_level_e;

#define ESUCCESS 0
#define EFAILURE 1
#define EINVALID 2
#define EBUSY 3
#define ENORESOURCE 4
#define ENOMEM 6

#endif