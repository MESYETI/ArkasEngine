#ifndef AE_CONFIG_H
#define AE_CONFIG_H

#include <stdio.h>

typedef void (*ConfigExt)(FILE*);
extern ConfigExt configExt;

void SaveConfig(void);
void SaveDefaultConfig(void);

#endif
