#ifndef AE_LOG_H
#define AE_LOG_H

void Log_Init(void);
void Log_End(void);
void Log(const char* format, ...);
void Error(const char* format, ...);
void Warn(const char* format, ...);

#endif
