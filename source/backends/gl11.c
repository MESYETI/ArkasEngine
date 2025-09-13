#include "../stb.h"
#include "../map.h"
#include "../util.h"
#include "../video.h"
#include "../camera.h"
#include "../common.h"
#include "../backend.h"

#define GL_GLEXT_PROTOTYPES
#ifdef PLATFORM_OSX
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif
#ifndef USE_KHR_DEBUG
    #if !defined(NDEBUG) && defined(GL_KHR_debug) && GL_KHR_debug
        #define USE_KHR_DEBUG 1
    #else
        #define USE_KHR_DEBUG 0
    #endif
#endif

static void GL_Error(GLenum error) {
	const char* errorStr;
	switch (error) {
		case GL_INVALID_ENUM:      errorStr = "Invalid enum"; break;
		case GL_INVALID_VALUE:     errorStr = "Invalid value"; break;
		case GL_INVALID_OPERATION: errorStr = "Invalid operation"; break;
		case GL_STACK_OVERFLOW:    errorStr = "Stack overflow"; break;
		case GL_STACK_UNDERFLOW:   errorStr = "Stack underflow"; break;
		case GL_OUT_OF_MEMORY:     errorStr = "Out of memory"; break;
		default:                   errorStr = "???";
	}

	Error("OpenGL error: %s", errorStr);
}

#define GL(CALL) do { \
	CALL; \
	GLenum error = glGetError(); \
 \
	if (error != GL_NO_ERROR) { \
		GL_Error(error); \
	} \
} while(0)

typedef struct {
	float nearPlane;
	float farPlane;
	float projMatrix[4][4];
	float viewMatrix[4][4];
	float fov;
	float aspect;
	Model model;

	SDL_GLContext ctx;
	GLuint tex;
} State;

static State state;

static void CalcProjMatrix(void) {
	float tmp1 = 1.0f / tanf(state.fov * (float) PI / 180.0f * 0.5f);
	float tmp2 = 1.0f / (state.nearPlane - state.farPlane);
	state.projMatrix[0][0] = -(tmp1 / state.aspect);
	state.projMatrix[1][1] = tmp1;
	state.projMatrix[2][2] = (state.nearPlane + state.farPlane) * tmp2;
	state.projMatrix[3][2] = 2.0f * state.nearPlane * state.farPlane * tmp2;
}

static void CalcViewMatrix(void) {
	static float up[3];
	static float front[3];
	static float rotRadX, rotRadY, rotRadZ;

	rotRadX = camera.pitch * (float) PI / 180.0f;
	rotRadY = camera.yaw * -(float) PI / 180.0f;
	rotRadZ = camera.roll * (float) PI / 180.0f;

	static float sinX, cosX;
	static float sinY, cosY;
	static float sinZ, cosZ;

	sinX     = sin(rotRadX);
	cosX     = cos(rotRadX);
	sinY     = sin(rotRadY);
	cosY     = cos(rotRadY);
	sinZ     = sin(rotRadZ);
	cosZ     = cos(rotRadZ);
	up[0]    = sinX * sinY * cosZ + cosY * sinZ;
	up[1]    = cosX * cosZ;
	up[2]    = -sinX * cosY * cosZ + sinY * sinZ;
	front[0] = cosX * -sinY;
	front[1] = sinX;
	front[2] = cosX * cosY;

	state.viewMatrix[0][0] = front[1] * up[2] - front[2] * up[1];
	state.viewMatrix[1][0] = front[2] * up[0] - front[0] * up[2];
	state.viewMatrix[2][0] = front[0] * up[1] - front[1] * up[0];
	state.viewMatrix[3][0] =
		-(state.viewMatrix[0][0] * camera.pos.x + state.viewMatrix[1][0] *
		camera.pos.y + state.viewMatrix[2][0] * camera.pos.z);

	state.viewMatrix[0][1] = up[0];
	state.viewMatrix[1][1] = up[1];
	state.viewMatrix[2][1] = up[2];
	state.viewMatrix[3][1] =
		-(up[0] * camera.pos.x + up[1] * camera.pos.y + up[2] * camera.pos.z);

	state.viewMatrix[0][2] = -front[0];
	state.viewMatrix[1][2] = -front[1];
	state.viewMatrix[2][2] = -front[2];
	state.viewMatrix[3][2] =
		front[0] * camera.pos.x + front[1] * camera.pos.y + front[2] * camera.pos.z;
}

void Backend_Init(void) {
	video.ctx = SDL_GL_CreateContext(video.window);
	assert(SDL_GL_MakeCurrent(video.window, video.ctx) == 0);

	if (SDL_GL_SetSwapInterval(-1) == -1) {
		SDL_GL_SetSwapInterval(1);
	}

	Log("Vendor:   %s", (const char*) glGetString(GL_VENDOR));
	Log("Renderer: %s", (const char*) glGetString(GL_RENDERER));
	Log("Version:  %s", (const char*) glGetString(GL_VERSION));

	/*Log("Extensions:");
	printf("    ");

	const char* ext = (const char*) glGetString(GL_EXTENSIONS);
	for (size_t i = 0; ext[i] != 0; ++ i) {
		if (ext[i] == ' ') {
			printf("\n    ");
		}
		else {
			putchar(ext[i]);
		}
	}

	printf("\r");*/

	// set values
	state.nearPlane        = 0.001;
	state.farPlane         = 1000;
	state.fov              = 70;
	state.aspect           = 640.0 / 480.0; // TODO: replace this
	state.viewMatrix[3][3] = 1.0f;
	state.projMatrix[2][3] = -1.0f;

	CalcProjMatrix();

	int maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	Log("Max texture size: %d", maxTextureSize);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	Model_Load(&state.model, "heavy.zkm");

	// load texture
	int width, height, ch;

	uint8_t* data = stbi_load("texture.png", &width, &height, &ch, 0);
	if (data == NULL) {
		Error("Failed to load %s: %s", "texture.png", stbi_failure_reason());
	}

    glGenTextures(1, &state.tex);
    glBindTexture(GL_TEXTURE_2D, state.tex);
	glTexImage2D(
		GL_TEXTURE_2D, 0, ch, width, height, 0, (ch == 3) ? GL_RGB : GL_RGBA,
		GL_UNSIGNED_BYTE, data
	);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

    free(data);
}

void Backend_Free(void) {
	SDL_GL_DeleteContext(state.ctx);
	Model_Free(&state.model);
}

void Backend_RenderScene(void) {
	glViewport(0, 0, video.width, video.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GL(glMatrixMode(GL_PROJECTION));
	GL(glLoadMatrixf((float*) state.projMatrix));
	GL(glMatrixMode(GL_MODELVIEW));
	CalcViewMatrix();
	GL(glLoadMatrixf((float*) state.viewMatrix));

	// glBegin(GL_TRIANGLE_FAN);
	// glVertex3f(-0.5f, -0.5f, 1.0f);
	// glVertex3f(0.5f, -0.5f, 1.0f);
	// glVertex3f(0.5f, 0.5f, 1.0f);
	// glVertex3f(-0.5f, 0.5f, 1.0f);
	// GL(glEnd());

// 	static const FVec2 walls[] = {
// 		{-2.0, -2.0}, {-2.0, 2.0}, {2.0, 2.0}, {2.0, -2.0}, {-2.0, -2.0}
// 	};
// 	static const int wallCount = 4;
// 
// 	for (int i = 0; i < wallCount; ++ i) {
// 		glBegin(GL_TRIANGLE_FAN);
// 
// 		switch (i) {
// 			case 0: glColor3ub(255, 0,   0);   break;
// 			case 1: glColor3ub(0,   255, 0);   break;
// 			case 2: glColor3ub(0,   0,   255); break;
// 			case 3: glColor3ub(255, 255, 0);   break;
// 		}
// 
// 		const FVec2* point1 = &walls[i];
// 		const FVec2* point2 = &walls[i + 1];
// 
// 		glVertex3f(point1->x, -0.5, point1->y);
// 		glVertex3f(point2->x, -0.5, point2->y);
// 		glVertex3f(point2->x,  0.5, point2->y);
// 		glVertex3f(point1->x,  0.5, point1->y);
// 		GL(glEnd());
// 	}

	glBindTexture(GL_TEXTURE_2D, state.tex);
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		glBegin(GL_TRIANGLE_FAN);

		switch (i % 4) {
			case 0: glColor3ub(255, 255, 255);   break;
			case 1: glColor3ub(239, 239, 239);   break;
			case 2: glColor3ub(223, 223, 223); break;
			case 3: glColor3ub(191, 191, 191);   break;
		}

		const MapPoint* point1 = &map.points[i];
		const MapPoint* point2 = (i == map.pointsLen - 1)?
			&map.points[0] : &map.points[i + 1];

		float maxTexCoord;
		maxTexCoord = Distance(point1->pos, point2->pos) / 0.5;

		glTexCoord2f(maxTexCoord, 1.0); // lower left
		glVertex3f(point1->pos.x, -0.5, point1->pos.y);

		glTexCoord2f(0.0, 1.0); // lower right
		glVertex3f(point2->pos.x, -0.5, point2->pos.y);

		glTexCoord2f(0.0, 0.0); // upper right
		glVertex3f(point2->pos.x,  0.5, point2->pos.y);

		glTexCoord2f(maxTexCoord, 0.0); // upper left
		glVertex3f(point1->pos.x,  0.5, point1->pos.y);

		GL(glEnd());
	}

	//glColor3ub(64, 64, 64);
	glBegin(GL_TRIANGLE_FAN);

	for (size_t i = 0; i < map.pointsLen; ++ i) {
		glTexCoord2f(map.points[i].pos.x, map.points[i].pos.y);
		glVertex3f(map.points[i].pos.x, -0.5, map.points[i].pos.y);
	}
	GL(glEnd());

	// render ceiling
	glBegin(GL_TRIANGLE_FAN);
	for (size_t i = 0; i < map.pointsLen; ++ i) {
		glTexCoord2f(map.points[i].pos.x, map.points[i].pos.y);
		glVertex3f(map.points[i].pos.x, 0.5, map.points[i].pos.y);
	}
	GL(glEnd());

	glBindTexture(GL_TEXTURE_2D, 0);

	ModelRenderOpt opt;
	opt.scale = 0.01;
	opt.pos   = (FVec3) {0.0, -0.5, 0.0};
	Backend_RenderModel(&state.model, &opt);

	GL(glFinish());
	SDL_GL_SwapWindow(video.window);
}

void Backend_OnWindowResize(void) {
	state.aspect = ((float) video.width) / ((float) video.height);
	CalcProjMatrix();
}

void Backend_RenderModel(Model* model, ModelRenderOpt* opt) {
	glBegin(GL_TRIANGLES);
	
	for (size_t i = 0; i < model->facesNum; ++ i) {
		ModelFace* face = &model->faces[i];

		//glColor3ub(face->colour.r, face->colour.g, face->colour.b);

		#if 1
			int ci = (face->indices[0] * 0x10492851) ^ face->indices[1];
			uint8_t c[3] = {ci >> 16, ci >> 8, ci};
			glColor3ub(c[0], c[1], c[2]);
		#endif

		// glTexCoord2f(0.0f, 0.0f);
		glVertex3f(
			(model->vertices[face->indices[0]].x * opt->scale) + opt->pos.x,
			(model->vertices[face->indices[0]].y * opt->scale) + opt->pos.y,
			(model->vertices[face->indices[0]].z * opt->scale) + opt->pos.z
		);
		// glTexCoord2f(1.0f, 0.0f);
		glVertex3f(
			(model->vertices[face->indices[1]].x * opt->scale) + opt->pos.x,
			(model->vertices[face->indices[1]].y * opt->scale) + opt->pos.y,
			(model->vertices[face->indices[1]].z * opt->scale) + opt->pos.z
		);
		// glTexCoord2f(1.0f, 1.0f);
		glVertex3f(
			(model->vertices[face->indices[2]].x * opt->scale) + opt->pos.x,
			(model->vertices[face->indices[2]].y * opt->scale) + opt->pos.y,
			(model->vertices[face->indices[2]].z * opt->scale) + opt->pos.z
		);
	}
	
	glEnd();
}
