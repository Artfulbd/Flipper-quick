#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <infrared.h>

typedef struct QuickApp QuickApp;

// Populate ir_signal_names[]/ir_signal_count in app by scanning the .ir file.
bool quick_ir_list_signals(QuickApp* app, const char* path);

// Load signal at index from the .ir file into app->ir_* fields.
bool quick_ir_load_signal(QuickApp* app, const char* path, size_t index);
