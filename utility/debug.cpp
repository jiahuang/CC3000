#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

#ifdef  __cplusplus
extern "C" {
#endif

void _DEBUG (char *fmt, ... )
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

#ifdef  __cplusplus
}
#endif // __cplusplus