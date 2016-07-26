AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = --pedantic -Wall -std=c99 -O2 -I./src/
AM_LDFLAGS = 

bin_PROGRAMS = vulkan
vulkan_LDADD = -lvulkan
vulkan_SOURCES = src/main.c src/util/vulkan.c src/util/vulkan.h
