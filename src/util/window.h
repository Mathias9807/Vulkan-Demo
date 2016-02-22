/*
 * Simple Vulkan application
 * 
 * Copyright (c) 2016 by Mathias Johansson
 * 
 * This code is licensed under the MIT license 
 * 		https://opensource.org/licenses/MIT
 */

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
