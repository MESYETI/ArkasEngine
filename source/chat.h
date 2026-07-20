#ifndef AE_CHAT_H
#define AE_CHAT_H

#include "common.h"

#define CHAT_MESSAGES 128

typedef struct {
	char*    message;
	uint64_t time;
} ChatMsg;

typedef struct {
	ChatMsg messages[CHAT_MESSAGES];
} Chat;

extern Chat chat;

void Chat_Init(void);
void Chat_Free(void);
void Chat_Add(const char* str);

#endif
