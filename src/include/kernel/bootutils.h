#pragma once
#include <limine.h>

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(6);

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

// Start and end markers for Limine requests, can be moved to any C file
__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;