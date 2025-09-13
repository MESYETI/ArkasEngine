#ifndef AE_BACKEND_H
#define AE_BACKEND_H

#include "model.h"

void Backend_Init(void);
void Backend_Free(void);
void Backend_RenderScene(void);
void Backend_OnWindowResize(void);
void Backend_RenderModel(Model* model, ModelRenderOpt* opt);

#endif
