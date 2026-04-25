#include <string.h>
#include "keyboard.h"

// TODO: this thing has missing values
static const char* keyTable[] = {
	/*  0 */ /* AE_KEY_NONE */          "None",
	/*  1 */ /* AE_KEY_A */             "A",
	/*  2 */ /* AE_KEY_B */             "B",
	/*  3 */ /* AE_KEY_C */             "C",
	/*  4 */ /* AE_KEY_D */             "D",
	/*  5 */ /* AE_KEY_E */             "E",
	/*  6 */ /* AE_KEY_F */             "F",
	/*  7 */ /* AE_KEY_G */             "G",
	/*  8 */ /* AE_KEY_H */             "H",
	/*  9 */ /* AE_KEY_I */             "I",
	/* 10 */ /* AE_KEY_J */             "J",
	/* 11 */ /* AE_KEY_K */             "K",
	/* 12 */ /* AE_KEY_L */             "L",
	/* 13 */ /* AE_KEY_M */             "M",
	/* 14 */ /* AE_KEY_N */             "N",
	/* 15 */ /* AE_KEY_O */             "O",
	/* 16 */ /* AE_KEY_P */             "P",
	/* 17 */ /* AE_KEY_Q */             "Q",
	/* 18 */ /* AE_KEY_R */             "R",
	/* 19 */ /* AE_KEY_S */             "S",
	/* 20 */ /* AE_KEY_T */             "T",
	/* 21 */ /* AE_KEY_U */             "U",
	/* 22 */ /* AE_KEY_V */             "V",
	/* 23 */ /* AE_KEY_W */             "W",
	/* 24 */ /* AE_KEY_X */             "X",
	/* 25 */ /* AE_KEY_Y */             "Y",
	/* 26 */ /* AE_KEY_Z */             "Z",
	/* 27 */ /* AE_KEY_0 */             "0",
	/* 28 */ /* AE_KEY_1 */             "1",
	/* 29 */ /* AE_KEY_2 */             "2",
	/* 30 */ /* AE_KEY_3 */             "3",
	/* 31 */ /* AE_KEY_4 */             "4",
	/* 32 */ /* AE_KEY_5 */             "5",
	/* 33 */ /* AE_KEY_6 */             "6",
	/* 34 */ /* AE_KEY_7 */             "7",
	/* 35 */ /* AE_KEY_8 */             "8",
	/* 36 */ /* AE_KEY_9 */             "9",
	/* 37 */ /* AE_KEY_RETURN */        "Return",
	/* 38 */ /* AE_KEY_ESCAPE */        "Escape",
	/* 39 */ /* AE_KEY_BACKSPACE */     "Backspace",
	/* 40 */ /* AE_KEY_TAB */           "Tab",
	/* 41 */ /* AE_KEY_SPACE */         "Space",
	/* 42 */ /* AE_KEY_MINUS */         "Minus",
	/* 43 */ /* AE_KEY_EQUALS */        "Equals",
	/* 44 */ /* AE_KEY_LEFT_BRACKET */  "Left bracket",
	/* 45 */ /* AE_KEY_RIGHT_BRACKET */ "Right bracket",
	/* 46 */ /* AE_KEY_BACKSLASH */     "Backslash",
	/* 47 */ /* AE_KEY_SEMICOLON */     "Semicolon",
	/* 48 */ /* AE_KEY_APOSTROPHE */    "Apostrophe",
	/* 49 */ /* AE_KEY_GRAVE */         "Grave",
	/* 50 */ /* AE_KEY_COMMA */         "Comma",
	/* 51 */ /* AE_KEY_PERIOD */        "Period",
	/* 52 */ /* AE_KEY_SLASH */         "Slash",
	/* 53 */ /* AE_KEY_CAPS_LOCK */     "Caps lock",
	/* 54 */ /* AE_KEY_F1 */            "F1",
	/* 55 */ /* AE_KEY_F2 */            "F2",
	/* 56 */ /* AE_KEY_F3 */            "F3",
	/* 57 */ /* AE_KEY_F4 */            "F4",
	/* 58 */ /* AE_KEY_F5 */            "F5",
	/* 59 */ /* AE_KEY_F6 */            "F6",
	/* 60 */ /* AE_KEY_F7 */            "F7",
	/* 61 */ /* AE_KEY_F8 */            "F8",
	/* 62 */ /* AE_KEY_F9 */            "F9",
	/* 63 */ /* AE_KEY_F10 */           "F10",
	/* 64 */ /* AE_KEY_F11 */           "F11",
	/* 65 */ /* AE_KEY_F12 */           "F12",
	/* 66 */ /* AE_KEY_PRINT_SCREEN */  "Print screen",
	/* 67 */ /* AE_KEY_SCROLL_LOCK */   "Scroll lock",
	/* 68 */ /* AE_KEY_PAUSE */         "Pause",
	/* 69 */ /* AE_KEY_INSERT */        "Insert",
	/* 70 */ /* AE_KEY_HOME */          "Home",
	/* 71 */ /* AE_KEY_PAGE_UP */       "Page up",
	/* 72 */ /* AE_KEY_PAGE_DOWN */     "Page down",
	/* 73 */ /* AE_KEY_DELETE */        "Delete",
	/* 74 */ /* AE_KEY_END */           "End",
	/* 75 */ /* AE_KEY_RIGHT */         "Right arrow",
	/* 76 */ /* AE_KEY_LEFT */          "Left arrow",
	/* 77 */ /* AE_KEY_DOWN */          "Down arrow",
	/* 78 */ /* AE_KEY_UP */            "Up arrow",
	/* 79 */ /* AE_KEY_NUM_LOCK */      "Num lock",
	/* 80 */ /* AE_KEY_NP_DIVIDE */     "Numpad divide",
	/* 81 */ /* AE_KEY_NP_MULTIPLY */   "Numpad multiply",
	/* 82 */ /* AE_KEY_NP_MINUS */      "Numpad minus",
	/* 83 */ /* AE_KEY_NP_PLUS */       "Numpad plus",
	/* 84 */ /* AE_KEY_NP_ENTER */      "Numpad enter",
	/* 85 */ /* AE_KEY_NP_0 */          "Numpad 0",
	/* 86 */ /* AE_KEY_NP_1 */          "Numpad 1",
	/* 87 */ /* AE_KEY_NP_2 */          "Numpad 2",
	/* 88 */ /* AE_KEY_NP_3 */          "Numpad 3",
	/* 89 */ /* AE_KEY_NP_4 */          "Numpad 4",
	/* 90 */ /* AE_KEY_NP_5 */          "Numpad 5",
	/* 91 */ /* AE_KEY_NP_6 */          "Numpad 6",
	/* 92 */ /* AE_KEY_NP_7 */          "Numpad 7",
	/* 93 */ /* AE_KEY_NP_8 */          "Numpad 8",
	/* 94 */ /* AE_KEY_NP_9 */          "Numpad 9",
	/* 95 */ /* AE_KEY_NP_PERIOD */     "Numpad period"
};

const char* Key_ToString(Key key) {
	if ((key < 0) || (key >= AE_KEY_AMOUNT)) {
		return "???";
	}

	return keyTable[key];
}

Key Key_FromString(const char* str) {
	for (size_t i = 0; i < AE_KEY_AMOUNT; ++ i) {
		if (strcmp(keyTable[i], str) == 0) return (Key) i;
	}

	return 0;
}
