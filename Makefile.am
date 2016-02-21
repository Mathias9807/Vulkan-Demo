AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = --pedantic -Wall -std=c99 -O2 -I./src/
AM_LDFLAGS = -L/usr/local/lib

bin_PROGRAMS = vulkan
vulkan_LDADD = -lvulkan -lxcb
vulkan_SOURCES = src/main.c src/util/window.c src/util/window.h src/util/vulkan.c src/util/vulkan.h
