#if !defined(AE_INPUT_SDL_H) && defined(AE_USE_SDL2)

#include <SDL2/SDL.h>
#include "../keyboard.h"

Key          Input_SDLScancodeToKey(SDL_Scancode key);
SDL_Scancode Input_KeyToSDLScancode(Key key);

#endif
