#ifndef __CC3000_DEBUG_H__
#define __CC3000_DEBUG_H__

#ifdef  __cplusplus
extern "C" {
#endif

void _TM_DEBUG (char *fmt, ... );

#define TM_DEBUG(format, ...) _TM_DEBUG((char *) F(format), ##__VA_ARGS__)
// #define TM_DEBUG(...)

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // __COMMON_H__
