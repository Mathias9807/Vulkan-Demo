AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = --pedantic -Wall -std=c99 -O2 -I./src/ `sdl2-config --cflags`
AM_LDFLAGS = `sdl2-config --libs` -I/usr/local/lib

bin_PROGRAMS = vulkan
vulkan_LDADD = -lvulkan -lSDL2 -lSDL_vulkan
vulkan_SOURCES = src/main.c src/util/vulkan.c src/util/vulkan.h
