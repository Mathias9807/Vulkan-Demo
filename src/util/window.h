#ifndef WINDOW_H
#define WINDOW_H


#include <stdbool.h>
#include "util/vulkan.h"

extern bool isClosing;

void openWindow();
void prepRender();
void tickWindow();
void quitWindow();


#endif // WINDOW_H
