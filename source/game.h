#ifndef AE_GAME_H
#define AE_GAME_H

#include "video.h"
#include "common.h"

void Game_Init(void);
void Game_Free(void);
void Game_Update(bool top);
void Game_HandleEvent(SDL_Event* e);
void Game_Render(void);

#endif
