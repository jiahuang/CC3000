#ifndef __CC3000_DEBUG_H__
#define __CC3000_DEBUG_H__

#ifdef  __cplusplus
extern "C" {
#endif

void _CC_DEBUG (char *fmt, ... );

#define CC_DEBUG(format, ...) _CC_DEBUG((char *) F(format), ##__VA_ARGS__)

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // __COMMON_H__
