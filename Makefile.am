AUTOMAKE_OPTIONS = foreign

CFLAGS = --pedantic -Wall -std=c99 -O2 -I./src/
LDFLAGS = -lvulkan -lglfw

bin_PROGRAMS = vulkan
vulkan_SOURCES = src/main.c src/util/window.c src/util/window.h src/util/vulkan.c src/util/vulkan.h
