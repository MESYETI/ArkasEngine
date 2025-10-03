#include "../stb.h"
#include "../map.h"
#include "../mem.h"
#include "../util.h"
#include "../video.h"
#include "../camera.h"
#include "../backend.h"
#include "gl11.h"

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
	Texture       textures[64];

	// per frame state
	bool* sectorsRendered;
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

void Backend_Init(bool beforeWindow) {
	if (beforeWindow) {
		assert(SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY
		) == 0);
		assert(SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MAJOR_VERSION, 2
		) == 0);
		assert(SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MINOR_VERSION, 0
		) == 0);
		//#if USE_KHR_DEBUG
		//    assert(SDL_GL_SetAttribute(
		//	    SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG
		//    ) == 0);
		//#endif

		return;
	}

	state.ctx = SDL_GL_CreateContext(video.window);
	assert(SDL_GL_MakeCurrent(video.window, state.ctx) == 0);

	if (SDL_GL_SetSwapInterval(-1) == -1) {
		SDL_GL_SetSwapInterval(1);
	}
	// SDL_GL_SetSwapInterval(0);

	Log("Backend info: GL11");
	Log("==================");
	Log("Vendor:           %s", (const char*) glGetString(GL_VENDOR));
	Log("Renderer:         %s", (const char*) glGetString(GL_RENDERER));
	Log("Version:          %s", (const char*) glGetString(GL_VERSION));

	int intVal;
	glGetIntegerv(GL_MAX_LIGHTS, &intVal);
	Log("Max lights:       %d", intVal);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &intVal);
	Log("Max texture size: %d", intVal);

	if (
		strstr((const char*) glGetString(GL_EXTENSIONS), "GL_ARB_multitexture")
		!= NULL
	) {
		Log("Multitexture available");
	}
	else {
		Log("Multitexture not available");
	}
	Log("==================");

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
	state.nearPlane        = 0.1;
	state.farPlane         = 1000;
	state.fov              = 70;
	state.aspect           = 640.0 / 480.0; // TODO: replace this
	state.viewMatrix[3][3] = 1.0f;
	state.projMatrix[2][3] = -1.0f;
	state.sectorsRendered  = NULL; // set this later

	CalcProjMatrix();

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Model_Load(&state.model, "heavy.zkm");
}

void Backend_Free(void) {
	SDL_GL_DeleteContext(state.ctx);
	Model_Free(&state.model);
}

static Texture* LoadTexture(uint8_t* data, int width, int height, int ch) {
	GLuint tex;
    GL(glGenTextures(1, &tex));
    GL(glBindTexture(GL_TEXTURE_2D, tex));
	GL(glTexImage2D(
		GL_TEXTURE_2D, 0, ch, width, height, 0, (ch == 3) ? GL_RGB : GL_RGBA,
		GL_UNSIGNED_BYTE, data
	));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT));

    free(data);

    // now put this texture in the texture array
    for (size_t i = 0; i < sizeof(state.textures) / sizeof(Texture); ++ i) {
    	if (!state.textures[i].used) {
    		state.textures[i].used   = true;
    		state.textures[i].name   = tex;
    		state.textures[i].width  = width;
    		state.textures[i].height = height;
    		return &state.textures[i];
    	}
    }

    Error("No more room for textures");
    return NULL;
}

Texture* Backend_LoadMemTexture(uint8_t* img, size_t len) {
	int width, height, ch;

	uint8_t* data = stbi_load_from_memory(img, (int) len, &width, &height, &ch, 0);
	if (data == NULL) {
		Error("Failed to load texture from memory: %s", stbi_failure_reason());
	}

	return LoadTexture(data, width, height, ch);
}

Texture* Backend_LoadTexture(const char* path) {
	int width, height, ch;

	uint8_t* data = stbi_load(path, &width, &height, &ch, 0);
	if (data == NULL) {
		Error("Failed to load %s: %s", path, stbi_failure_reason());
	}

	return LoadTexture(data, width, height, ch);
}

void Backend_FreeTexture(Texture* texture) {
	GL(glDeleteTextures(1, &texture->name));
	texture->used = false;
}

Vec2 Backend_GetTextureSize(Texture* texture) {
	return (Vec2) {texture->width, texture->height};
}

static void RenderSector(Sector* sector) {
	// float height = sector->ceiling - sector->floor;

	if (state.sectorsRendered[sector - map.sectors]) {
		return;
	}

	state.sectorsRendered[sector - map.sectors] = true;

	for (size_t i = 0; i < sector->length; ++ i) {
		const MapPoint* point1 = &map.points[i + sector->start];
		const MapPoint* point2 = (i == sector->length - 1)?
			&map.points[sector->start] : &map.points[i + sector->start + 1];

		const Wall* wall = &map.walls[i + sector->start];
		glBindTexture(GL_TEXTURE_2D, wall->texture->v.texture->name);

		FVec2 camPos = (FVec2) {camera.pos.x, camera.pos.z};

		float shadeLeft  = 1.0 - (Distance(camPos, point1->pos) / 20.0);
		float shadeRight = 1.0 - (Distance(camPos, point2->pos) / 20.0);

		if (shadeLeft  < 0.0) shadeLeft  = 0.0;
		if (shadeRight < 0.0) shadeRight = 0.0;

		float maxTexCoord;
		maxTexCoord = Distance(point1->pos, point2->pos);

		if (wall->isPortal) {
			Sector* nextSector = &map.sectors[wall->portalSector];

			// lower wall
			if (nextSector->floor > sector->floor) {
				glBegin(GL_TRIANGLE_FAN);

				glTexCoord2f(-maxTexCoord, sector->floor); // lower left
				glColor3f(shadeLeft, shadeLeft, shadeLeft);
				glVertex3f(point1->pos.x, sector->floor, point1->pos.y);

				glTexCoord2f(-0.0, sector->floor); // lower right
				glColor3f(shadeRight, shadeRight, shadeRight);
				glVertex3f(point2->pos.x, sector->floor, point2->pos.y);

				glTexCoord2f(-0.0, nextSector->floor); // upper right
				glColor3f(shadeRight, shadeRight, shadeRight);
				glVertex3f(point2->pos.x, nextSector->floor, point2->pos.y);

				glTexCoord2f(-maxTexCoord, nextSector->floor); // upper left
				glColor3f(shadeLeft, shadeLeft, shadeLeft);
				glVertex3f(point1->pos.x, nextSector->floor, point1->pos.y);

				GL(glEnd());
			}

			// upper wall
			if (nextSector->ceiling < sector->ceiling) {
				glBegin(GL_TRIANGLE_FAN);

				glTexCoord2f(-maxTexCoord, nextSector->ceiling); // lower left
				glColor3f(shadeLeft, shadeLeft, shadeLeft);
				glVertex3f(point1->pos.x, nextSector->ceiling, point1->pos.y);

				glTexCoord2f(-0.0, nextSector->ceiling); // lower right
				glColor3f(shadeRight, shadeRight, shadeRight);
				glVertex3f(point2->pos.x, nextSector->ceiling, point2->pos.y);

				glTexCoord2f(-0.0, sector->ceiling); // upper right
				glColor3f(shadeLeft, shadeLeft, shadeLeft);
				glVertex3f(point2->pos.x, sector->ceiling, point2->pos.y);

				glTexCoord2f(-maxTexCoord, sector->ceiling); // upper left
				glColor3f(shadeRight, shadeRight, shadeRight);
				glVertex3f(point1->pos.x, sector->ceiling, point1->pos.y);

				GL(glEnd());
			}
		}
		else {
			glBegin(GL_TRIANGLE_FAN);
			glColor3ub(255, 255, 255);

			glTexCoord2f(-maxTexCoord, sector->floor); // lower left
			glColor3f(shadeLeft, shadeLeft, shadeLeft);
			glVertex3f(point1->pos.x, sector->floor, point1->pos.y);

			glTexCoord2f(-0.0, sector->floor); // lower right
			glColor3f(shadeRight, shadeRight, shadeRight);
			glVertex3f(point2->pos.x, sector->floor, point2->pos.y);

			glTexCoord2f(-0.0, sector->ceiling); // upper right
			glColor3f(shadeRight, shadeRight, shadeRight);
			glVertex3f(point2->pos.x, sector->ceiling, point2->pos.y);

			glTexCoord2f(-maxTexCoord, sector->ceiling); // upper left
			glColor3f(shadeLeft, shadeLeft, shadeLeft);
			glVertex3f(point1->pos.x, sector->ceiling, point1->pos.y);

			GL(glEnd());
		}
	}

	// render floor
	glBindTexture(GL_TEXTURE_2D, sector->floorTexture->v.texture->name);
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0, 1.0, 1.0);

	for (size_t i = sector->length - 1; true; -- i) {
		size_t idx = i + sector->start;
		glTexCoord2f(-map.points[idx].pos.x, map.points[idx].pos.y);
		glVertex3f(map.points[idx].pos.x, sector->floor, map.points[idx].pos.y);

		if (i == 0) break;
	}
	GL(glEnd());

	// render ceiling
	glBindTexture(GL_TEXTURE_2D, sector->ceilingTexture->v.texture->name);
	glBegin(GL_TRIANGLE_FAN);
	for (size_t i = 0; i < sector->length; ++ i) {
		size_t idx = i + sector->start;
		glTexCoord2f(map.points[idx].pos.x, map.points[idx].pos.y);
		glVertex3f(map.points[idx].pos.x, sector->ceiling, map.points[idx].pos.y);
	}
	GL(glEnd());

	for (size_t i = 0; i < sector->length; ++ i) {
		const Wall* wall = &map.walls[i + sector->start];

		if (wall->isPortal) {
			RenderSector(&map.sectors[wall->portalSector]);
		}
	}
}

void Backend_RenderScene(void) {
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	GL(glMatrixMode(GL_PROJECTION));
	GL(glLoadMatrixf((float*) state.projMatrix));
	GL(glMatrixMode(GL_MODELVIEW));
	CalcViewMatrix();
	GL(glLoadMatrixf((float*) state.viewMatrix));

	if (state.sectorsRendered == NULL) {
		state.sectorsRendered = SafeMalloc(map.sectorsLen * sizeof(bool));
	}
	memset(state.sectorsRendered, 0, map.sectorsLen * sizeof(bool));

	RenderSector(camera.sector);

	glBindTexture(GL_TEXTURE_2D, 0);

	ModelRenderOpt opt;
	opt.scale = 0.01;
	opt.pos   = (FVec3) {0.0, -0.5, 0.0};
	Backend_RenderModel(&state.model, &opt);

	// now do 2D stuff
	Backend_Begin2D();
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
	
	GL(glEnd());
}

void Backend_DrawTexture(
	Texture* texture, TextureRenderOpt* p_opt, Rect* p_src, Rect* p_dest
) {
	TextureRenderOpt opt;
	if (p_opt == NULL) {
		opt.doTint = false;
	}
	else {
		opt = *p_opt;
	}

	Rect src;
	if (p_src == NULL) {
		src = (Rect) {0, 0, texture->width, texture->height};
	}
	else {
		src = *p_src;
	}

	Rect dest;
	if (p_dest == NULL) {
		dest = (Rect) {0, 0, video.width, video.height};
	}
	else {
		dest = *p_dest;
	}

	GL(glBindTexture(GL_TEXTURE_2D, texture->name));

	glBegin(GL_TRIANGLE_FAN);

	if (opt.doTint) {
		glColor3ub(opt.tint.r, opt.tint.g, opt.tint.b);
	}
	else {
		glColor3ub(255, 255, 255);
	}

	glTexCoord2f(
		((float) src.x) / ((float) texture->width),
		((float) src.y) / ((float) texture->height)
	);
	glVertex2i(dest.x, dest.y);
	glTexCoord2f(
		((float) (src.x + src.w)) / ((float) texture->width),
		((float) src.y) / ((float) texture->height)
	);
	glVertex2i(dest.x + dest.w, dest.y);
	glTexCoord2f(
		((float) (src.x + src.w)) / ((float) texture->width),
		((float) (src.y + src.h)) / ((float) texture->height)
	);
	glVertex2i(dest.x + dest.w, dest.y + dest.h);
	glTexCoord2f(
		((float) src.x) / ((float) texture->width),
		((float) (src.y + src.h)) / ((float) texture->height)
	);
	glVertex2i(dest.x, dest.y + dest.h);
	GL(glEnd());
}

void Backend_Begin(void) {
	glViewport(0, 0, video.width, video.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Backend_Begin2D(void) {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	GL(glMatrixMode(GL_MODELVIEW));
	GL(glLoadIdentity());
	GL(glMatrixMode(GL_PROJECTION));
	GL(glLoadIdentity());
	GL(glOrtho(
		0.0, (float) video.width, (float) video.height,
		0.0, -1.0, 1.0
	));
}

void Backend_Clear(uint8_t r, uint8_t g, uint8_t b) {
	glClearColor(((float) r) / 256.0, ((float) g) / 256.0, ((float) b) / 256.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Backend_FinishRender(void) {
	GL(glFinish());
	SDL_GL_SwapWindow(video.window);
}
