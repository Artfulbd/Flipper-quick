#pragma once

#include <stdbool.h>
#include <stddef.h>

// Load favourites from file into names[] (display) and paths[] (full paths).
// Returns number of entries loaded.
size_t quick_favourites_load(
    const char* fav_path,
    char        names[][48],
    char        paths[][256],
    size_t      max_count);

// Append a new path to the favourites file. Skips if already present.
// Returns true on success.
bool quick_favourites_append(const char* fav_path, const char* file_path);

// Remove a path from the favourites file. Returns true on success.
bool quick_favourites_remove(const char* fav_path, const char* file_path);

// Check if a path is already in the favourites file.
bool quick_favourites_contains(const char* fav_path, const char* file_path);

// Extract the display name from a full path: strip directory and extension.
void quick_favourites_basename(const char* path, char* out, size_t out_size);
