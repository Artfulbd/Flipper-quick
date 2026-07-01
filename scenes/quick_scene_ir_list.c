#include "../quick.h"
#include "../quick_favourites.h"

#define MAX_FAVS QUICK_MAX_FAVS

static char ir_names[MAX_FAVS][QUICK_NAME_LEN];
static char ir_paths[MAX_FAVS][QUICK_PATH_LEN];
static size_t ir_count = 0;

static void quick_scene_ir_list_callback(void* context, uint32_t index) {
    QuickApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void quick_scene_ir_list_on_enter(void* context) {
    QuickApp* app = context;
    submenu_reset(app->submenu);

    ir_count = quick_favourites_load(
        QUICK_IR_FAV_PATH, ir_names, ir_paths, MAX_FAVS);

    for(size_t i = 0; i < ir_count; i++) {
        submenu_add_item(
            app->submenu, ir_names[i], i, quick_scene_ir_list_callback, app);
    }

    submenu_add_item(
        app->submenu,
        "Select from saved",
        QUICK_SELECT_INDEX,
        quick_scene_ir_list_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewSubmenu);
}

bool quick_scene_ir_list_on_event(void* context, SceneManagerEvent event) {
    QuickApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t index = event.event;
        if(index == QUICK_SELECT_INDEX) {
            scene_manager_next_scene(app->scene_manager, QuickSceneFileSelect);
            return true;
        }
        if(index < ir_count) {
            furi_string_set_str(app->selected_path, ir_paths[index]);
            strlcpy(app->selected_name, ir_names[index], QUICK_NAME_LEN);
            scene_manager_next_scene(app->scene_manager, QuickSceneIrRemote);
            return true;
        }
    }
    return false;
}

void quick_scene_ir_list_on_exit(void* context) {
    QuickApp* app = context;
    submenu_reset(app->submenu);
}
