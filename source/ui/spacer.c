#include "spacer.h"

UI_Element UI_NewSpacer(int fixedWidth) {
	return (UI_Element) {
		.fixedWidth      = fixedWidth,
		.data            = NULL,
		.preferredHeight = 0,
		.canHide         = true,
		.free            = NULL,
		.render          = NULL,
		.onClick         = NULL,
		.onEvent         = NULL
	};
}
