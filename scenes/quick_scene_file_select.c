#include "../quick.h"
#include "../quick_favourites.h"

#include <storage/storage.h>
#include <string.h>

#define MAX_FILES  64
#define LABEL_LEN  (QUICK_NAME_LEN + 5)  // "[+] " prefix + name

// Persistent storage for submenu string pointers (submenu doesn't copy labels)
static char   fs_names[MAX_FILES][QUICK_NAME_LEN];
static char   fs_paths[MAX_FILES][QUICK_PATH_LEN];
static bool   fs_selected[MAX_FILES];
static char   fs_labels[MAX_FILES][LABEL_LEN];
static size_t fs_count = 0;

static void update_label(size_t i) {
    snprintf(fs_labels[i], LABEL_LEN, "%s %s",
             fs_selected[i] ? "[+]" : "[ ]",
             fs_names[i]);
}

static void mode_paths(QuickMode mode, const char** dir, const char** ext, const char** fav_path) {
    switch(mode) {
    case QuickModeNfc:
        *dir = EXT_PATH("nfc");
        *ext = ".nfc";
        *fav_path = QUICK_NFC_FAV_PATH;
        break;
    case QuickModeRfid:
        *dir = EXT_PATH("lfrfid");
        *ext = ".rfid";
        *fav_path = QUICK_RFID_FAV_PATH;
        break;
    case QuickModeIr:
    default:
        *dir = EXT_PATH("infrared");
        *ext = ".ir";
        *fav_path = QUICK_IR_FAV_PATH;
        break;
    }
}

static const char* mode_fav_path(QuickMode mode) {
    const char* dir;
    const char* ext;
    const char* fav_path;
    mode_paths(mode, &dir, &ext, &fav_path);
    return fav_path;
}

static const char* mode_title(QuickMode mode) {
    switch(mode) {
    case QuickModeNfc:
        return "NFC files";
    case QuickModeRfid:
        return "RFID files";
    case QuickModeIr:
    default:
        return "IR files";
    }
}

static void scan_directory(QuickApp* app) {
    const char* dir;
    const char* ext;
    const char* fav_path;
    mode_paths(app->mode, &dir, &ext, &fav_path);

    fs_count = 0;

    Storage* storage  = furi_record_open(RECORD_STORAGE);
    File*    dir_file = storage_file_alloc(storage);

    if(storage_dir_open(dir_file, dir)) {
        FileInfo info;
        char     name[QUICK_PATH_LEN];

        while(fs_count < MAX_FILES &&
              storage_dir_read(dir_file, &info, name, sizeof(name))) {
            if(info.flags & FSF_DIRECTORY) continue;

            size_t nlen = strlen(name);
            size_t elen = strlen(ext);
            if(nlen <= elen || strcmp(name + nlen - elen, ext) != 0) continue;

            strlcpy(fs_paths[fs_count], dir, QUICK_PATH_LEN);
            strlcat(fs_paths[fs_count], "/", QUICK_PATH_LEN);
            strlcat(fs_paths[fs_count], name, QUICK_PATH_LEN);

            strlcpy(fs_names[fs_count], name, QUICK_NAME_LEN);
            char* dot = strrchr(fs_names[fs_count], '.');
            if(dot) *dot = '\0';

            fs_selected[fs_count] =
                quick_favourites_contains(fav_path, fs_paths[fs_count]);

            update_label(fs_count);
            fs_count++;
        }

        storage_dir_close(dir_file);
    }

    storage_file_free(dir_file);
    furi_record_close(RECORD_STORAGE);

    // Sort alphabetically by name (insertion sort on parallel arrays)
    for(size_t i = 1; i < fs_count; i++) {
        char tmp_name[QUICK_NAME_LEN];
        char tmp_path[QUICK_PATH_LEN];
        bool tmp_sel;
        strlcpy(tmp_name, fs_names[i], QUICK_NAME_LEN);
        strlcpy(tmp_path, fs_paths[i], QUICK_PATH_LEN);
        tmp_sel = fs_selected[i];
        size_t j = i;
        while(j > 0 && strcasecmp(fs_names[j - 1], tmp_name) > 0) {
            strlcpy(fs_names[j], fs_names[j - 1], QUICK_NAME_LEN);
            strlcpy(fs_paths[j], fs_paths[j - 1], QUICK_PATH_LEN);
            fs_selected[j] = fs_selected[j - 1];
            j--;
        }
        strlcpy(fs_names[j], tmp_name, QUICK_NAME_LEN);
        strlcpy(fs_paths[j], tmp_path, QUICK_PATH_LEN);
        fs_selected[j] = tmp_sel;
    }

    // Rebuild labels after sort
    for(size_t i = 0; i < fs_count; i++) update_label(i);
}

static void quick_scene_file_select_callback(void* context, uint32_t index) {
    QuickApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void quick_scene_file_select_on_enter(void* context) {
    QuickApp* app = context;
    scan_directory(app);

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, mode_title(app->mode));

    for(size_t i = 0; i < fs_count; i++) {
        submenu_add_item(
            app->submenu,
            fs_labels[i],
            (uint32_t)i,
            quick_scene_file_select_callback,
            app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewSubmenu);
}

bool quick_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    QuickApp* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t index = event.event;
        if(index >= fs_count) return false;

        const char* fav_path = mode_fav_path(app->mode);

        Storage* storage = furi_record_open(RECORD_STORAGE);
        storage_common_mkdir(storage, QUICK_DATA_DIR);
        furi_record_close(RECORD_STORAGE);

        if(fs_selected[index]) {
            quick_favourites_remove(fav_path, fs_paths[index]);
            fs_selected[index] = false;
        } else {
            quick_favourites_append(fav_path, fs_paths[index]);
            fs_selected[index] = true;
        }

        update_label(index);

        // Rebuild submenu in-place so the tick mark updates immediately
        submenu_reset(app->submenu);
        submenu_set_header(app->submenu, mode_title(app->mode));
        for(size_t i = 0; i < fs_count; i++) {
            submenu_add_item(
                app->submenu,
                fs_labels[i],
                (uint32_t)i,
                quick_scene_file_select_callback,
                app);
        }

        return true;
    }

    return false;
}

void quick_scene_file_select_on_exit(void* context) {
    QuickApp* app = context;
    submenu_reset(app->submenu);
    fs_count = 0;
}
