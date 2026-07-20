#include <string.h>
#include "chat.h"
#include "util.h"
#include "platform.h"

Chat chat;

void Chat_Init(void) {
	for (size_t i = 0; i < sizeof(chat.messages) / sizeof(ChatMsg); ++ i) {
		chat.messages[i].message = NULL;
		chat.messages[i].time    = 0;
	}
}

void Chat_Free(void) {
	for (size_t i = 0; i < sizeof(chat.messages) / sizeof(ChatMsg); ++ i) {
		if (chat.messages[i].message) {
			free(chat.messages[i].message);
		}

		chat.messages[i].message = NULL;
		chat.messages[i].time    = 0;
	}
}

void Chat_Add(const char* str) {
	memmove(&chat.messages[1], &chat.messages[0], (CHAT_MESSAGES - 1) * sizeof(ChatMsg));
	chat.messages[0] = (ChatMsg) {NewString(str), Platform_GetTime()};
}
