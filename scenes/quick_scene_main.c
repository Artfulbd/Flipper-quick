#include "../quick.h"

typedef enum {
    QuickMainNfc,
    QuickMainIr,
    QuickMainRfid,
    QuickMainAbout,
} QuickMainIndex;

static void quick_scene_main_callback(void* context, uint32_t index) {
    QuickApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void quick_scene_main_on_enter(void* context) {
    QuickApp* app = context;
    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "NFC",   QuickMainNfc,   quick_scene_main_callback, app);
    submenu_add_item(app->submenu, "IR",    QuickMainIr,    quick_scene_main_callback, app);
    submenu_add_item(app->submenu, "RFID",  QuickMainRfid,  quick_scene_main_callback, app);
    submenu_add_item(app->submenu, "About", QuickMainAbout, quick_scene_main_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewSubmenu);
}

bool quick_scene_main_on_event(void* context, SceneManagerEvent event) {
    QuickApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == QuickMainNfc) {
            app->mode = QuickModeNfc;
            scene_manager_next_scene(app->scene_manager, QuickSceneNfcList);
            return true;
        }
        if(event.event == QuickMainIr) {
            app->mode = QuickModeIr;
            scene_manager_next_scene(app->scene_manager, QuickSceneIrList);
            return true;
        }
        if(event.event == QuickMainRfid) {
            app->mode = QuickModeRfid;
            scene_manager_next_scene(app->scene_manager, QuickSceneRfidList);
            return true;
        }
        if(event.event == QuickMainAbout) {
            scene_manager_next_scene(app->scene_manager, QuickSceneAbout);
            return true;
        }
    }
    return false;
}

void quick_scene_main_on_exit(void* context) {
    QuickApp* app = context;
    submenu_reset(app->submenu);
}
