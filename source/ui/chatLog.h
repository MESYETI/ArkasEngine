#ifndef AE_UI_CHAT_LOG_H
#define AE_UI_CHAT_LOG_H

#include "../ui.h"

typedef struct {
	uint64_t time;
	char*    message;
} UI_ChatLogMsg;

typedef struct {
	UI_ChatLogMsg* lines;
	size_t         lineNum;
} UI_ChatLog;

UI_Element UI_NewChatLog(size_t lineNum);
void       UI_ChatLogAddMsg(UI_Element* elem, const char* message);

#endif
