#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "fs.h"
#include "log.h"
#include "mem.h"
#include "window.h"
#include "console.h"

static FILE* logFile = NULL;

void Log_Init(void) {
	logFile = fopen(AE_LOCATION "log.txt", "w");

	if (!logFile) {
		Warn("Cannot open log file: " AE_LOCATION "log.txt");
	}
}

void Log_End(void) {
	if (!logFile) {
		fflush(logFile);
		fclose(logFile);
	}
}

// most of this is taken from vsprintf(3)
#define FMT(RET, FORMAT) do { \
	int     n    = 0; \
	size_t  size = 0; \
	va_list ap; \
\
	va_start(ap, FORMAT); \
	n = vsnprintf(RET, size, FORMAT, ap); \
	va_end(ap); \
\
	assert(n >= 0); \
\
	size = n + 1; \
	RET = (char*) SafeMalloc(size); \
	if (RET == NULL) { \
		return; \
	} \
\
	va_start(ap, FORMAT); \
	n = vsnprintf(RET, size, FORMAT, ap); \
	va_end(ap); \
\
	assert(n >= 0); \
} while (0);

void Log(const char* format, ...) {
	char* ret = NULL;

	FMT(ret, format);

	time_t     rawTime;
	struct tm* tm;
	
	time(&rawTime);
	tm = localtime(&rawTime);
	
	printf("[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, ret);
	if (logFile) {
		fprintf(
			logFile, "[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec,
			ret
		);
		fflush(logFile);
	}

	Console_WriteLine(ret);
	free(ret);
}

void Error(const char* format, ...) {
	char* ret = NULL;

	FMT(ret, format);

	time_t     rawTime;
	struct tm* tm;
	
	time(&rawTime);
	tm = localtime(&rawTime);
	
	printf("[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, ret);
	if (logFile) {
		fprintf(
			logFile, "[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec,
			ret
		);
		fflush(logFile);
	}

	Window_MessageBox(WINDOW_MSG_ERROR, "Fatal error", ret);
	free(ret);
	exit(1);
}

void Warn(const char* format, ...) {
	char* ret = NULL;

	FMT(ret, format);

	time_t     rawTime;
	struct tm* tm;
	
	time(&rawTime);
	tm = localtime(&rawTime);
	
	printf("[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, ret);
	if (logFile) {
		fprintf(
			logFile, "[%.2d:%.2d:%.2d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec,
			ret
		);
		fflush(logFile);
	}

	Window_MessageBox(WINDOW_MSG_WARNING, "Warning", ret);
	free(ret);
}
