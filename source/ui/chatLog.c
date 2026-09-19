#include "../mem.h"
#include "../util.h"
#include "../text.h"
#include "../engine.h"
#include "../platform.h"
#include "chatLog.h"

static void Free(UI_Element* e) {
	UI_ChatLog* data = (UI_ChatLog*) e->data;

	for (size_t i = 0; i < data->lineNum; ++ i) {
		if (data->lines[i].message) {
			free(data->lines[i].message);
		}
	}
}

static void Render(UI_Container* container, UI_Element* e, bool focus) {
	(void) focus;

	UI_ChatLog* data = (UI_ChatLog*) e->data;
	Rect        rect = UI_ContainerGetRect(container);

	Rect eRect = (Rect) {
		rect.x + e->x, rect.y + e->y, e->w, e->h
	};

	uint64_t time = Platform_GetTime();

	int lineY = (eRect.y + eRect.h) - 1 - engine.font.charHeight;

	for (size_t i = 0; i < data->lineNum; ++ i) {
		UI_ChatLogMsg* msg = &data->lines[i];

		// show message for 10 seconds
		if ((container->hidden && (time - msg->time > 10000000)) || !msg->message) {
			continue;
		}

		if (container->hidden) {
			Rect lineRect = (Rect) {
				eRect.x, lineY, eRect.w, engine.font.charHeight
			};
			Backend_EnableAlpha(true);
			Backend_RenderRect(lineRect, (Colour) {0x00, 0x00, 0x00, 0x88});
		}

		Text_Render(&engine.font, msg->message, eRect.x + 1, lineY + 1);

		lineY -= engine.font.charHeight - 2;
	}
}

UI_Element UI_NewChatLog(size_t lineNum) {
	UI_Element ret;
	ret.fixedWidth      = 0;
	ret.data            = SafeMalloc(sizeof(UI_ChatLog));
	ret.preferredHeight = ((engine.font.charHeight + 2) * (int) lineNum) + 2;
	ret.canHide         = false;
	ret.free            = &Free;
	ret.render          = &Render;
	ret.onClick         = NULL;
	ret.onEvent         = NULL;

	UI_ChatLog* data = (UI_ChatLog*) ret.data;
	data->lines      = SafeMalloc(lineNum * sizeof(UI_ChatLogMsg));
	data->lineNum    = lineNum;

	for (size_t i = 0; i < lineNum; ++ i) {
		data->lines[i].message = NULL;
	}

	return ret;
}

void UI_ChatLogAddMsg(UI_Element* elem, const char* message) {
	UI_ChatLog* data = (UI_ChatLog*) elem->data;

	memmove(&data->lines[1], &data->lines[0], (data->lineNum - 1) * sizeof(UI_ChatLogMsg));

	data->lines[0] = (UI_ChatLogMsg) {Platform_GetTime(), NewString(message)};
}
