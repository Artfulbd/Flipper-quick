#include "quick_favourites.h"
#include "quick.h"

#include <furi.h>
#include <storage/storage.h>
#include <string.h>

#define TAG "QuickFavs"

void quick_favourites_basename(const char* path, char* out, size_t out_size) {
    // Find last '/'
    const char* slash = strrchr(path, '/');
    const char* base  = slash ? slash + 1 : path;

    // Copy to out, removing extension
    strlcpy(out, base, out_size);
    char* dot = strrchr(out, '.');
    if(dot) *dot = '\0';
}

size_t quick_favourites_load(
    const char* fav_path,
    char        names[][48],
    char        paths[][256],
    size_t      max_count) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);
    size_t   count   = 0;

    if(storage_file_open(file, fav_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FuriString* line = furi_string_alloc();

        while(count < max_count) {
            furi_string_reset(line);
            // Read line character by character
            char c;
            bool got_char = false;
            while(storage_file_read(file, &c, 1) == 1) {
                if(c == '\n' || c == '\r') {
                    if(got_char) break;
                    continue;
                }
                furi_string_push_back(line, c);
                got_char = true;
            }

            if(!got_char) break;

            const char* p = furi_string_get_cstr(line);
            if(strlen(p) == 0) continue;

            strlcpy(paths[count], p, 256);
            quick_favourites_basename(p, names[count], 48);
            count++;
        }

        furi_string_free(line);
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    // Sort alphabetically by name (insertion sort on parallel arrays)
    for(size_t i = 1; i < count; i++) {
        char tmp_name[48];
        char tmp_path[256];
        strlcpy(tmp_name, names[i], 48);
        strlcpy(tmp_path, paths[i], 256);
        size_t j = i;
        while(j > 0 && strcasecmp(names[j - 1], tmp_name) > 0) {
            strlcpy(names[j], names[j - 1], 48);
            strlcpy(paths[j], paths[j - 1], 256);
            j--;
        }
        strlcpy(names[j], tmp_name, 48);
        strlcpy(paths[j], tmp_path, 256);
    }

    return count;
}

// Read all lines from fav_path into a FuriString array. Returns count.
static size_t favs_read_all_lines(
    Storage*     storage,
    const char*  fav_path,
    FuriString** lines,
    size_t       max) {
    File*  file  = storage_file_alloc(storage);
    size_t count = 0;

    if(storage_file_open(file, fav_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char c;
        while(count < max) {
            FuriString* line = furi_string_alloc();
            bool        got  = false;
            while(storage_file_read(file, &c, 1) == 1) {
                if(c == '\n' || c == '\r') {
                    if(got) break;
                    continue;
                }
                furi_string_push_back(line, c);
                got = true;
            }
            if(!got) {
                furi_string_free(line);
                break;
            }
            lines[count++] = line;
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    return count;
}

bool quick_favourites_contains(const char* fav_path, const char* file_path) {
    Storage*    storage = furi_record_open(RECORD_STORAGE);
    FuriString* lines[QUICK_MAX_FAVS];
    size_t      count   = favs_read_all_lines(storage, fav_path, lines, QUICK_MAX_FAVS);
    bool        found   = false;

    for(size_t i = 0; i < count; i++) {
        if(furi_string_equal_str(lines[i], file_path)) found = true;
        furi_string_free(lines[i]);
    }

    furi_record_close(RECORD_STORAGE);
    return found;
}

bool quick_favourites_append(const char* fav_path, const char* file_path) {
    if(quick_favourites_contains(fav_path, file_path)) return true;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, EXT_PATH("apps_data/quick"));

    File* file = storage_file_alloc(storage);
    bool  ok   = false;

    if(storage_file_open(file, fav_path, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        size_t len = strlen(file_path);
        ok = (storage_file_write(file, file_path, len) == len) &&
             (storage_file_write(file, "\n", 1) == 1);
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

bool quick_favourites_remove(const char* fav_path, const char* file_path) {
    Storage*    storage = furi_record_open(RECORD_STORAGE);
    FuriString* lines[QUICK_MAX_FAVS];
    size_t      count   = favs_read_all_lines(storage, fav_path, lines, QUICK_MAX_FAVS);

    // Write back all lines except the one to remove
    File* file = storage_file_alloc(storage);
    bool  ok   = storage_file_open(file, fav_path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(ok) {
        for(size_t i = 0; i < count; i++) {
            if(!furi_string_equal_str(lines[i], file_path)) {
                const char* p   = furi_string_get_cstr(lines[i]);
                size_t      len = strlen(p);
                storage_file_write(file, p, len);
                storage_file_write(file, "\n", 1);
            }
        }
        storage_file_close(file);
    }

    for(size_t i = 0; i < count; i++) furi_string_free(lines[i]);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}
