#ifndef AE_BACKENDS_GL_LEGACY_H
#define AE_BACKENDS_GL_LEGACY_H

#define GL_GLEXT_PROTOTYPES
#ifdef AE_BACKEND_GL11
	#ifdef PLATFORM_OSX
		#include <OpenGL/gl.h>
		#include <OpenGL/glext.h>
	#else
		#include <GL/gl.h>
		#include <GL/glext.h>
	#endif
#endif

#ifndef USE_KHR_DEBUG
    #if !defined(NDEBUG) && defined(GL_KHR_debug) && GL_KHR_debug
        #define USE_KHR_DEBUG 1
    #else
        #define USE_KHR_DEBUG 0
    #endif
#endif

typedef struct {
	bool   used;
	GLuint name;
	int    width;
	int    height;
	int    actualWidth;
	int    actualHeight;
} Texture;

#endif
