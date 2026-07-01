#include "quick.h"
#include <furi.h>

static bool quick_navigation_callback(void* context) {
    QuickApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static bool quick_custom_event_callback(void* context, uint32_t event) {
    QuickApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

QuickApp* quick_app_alloc(void) {
    QuickApp* app = malloc(sizeof(QuickApp));

    app->scene_manager = scene_manager_alloc(&quick_scene_handlers, app);

    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, quick_navigation_callback);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, quick_custom_event_callback);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, QuickViewSubmenu, submenu_get_view(app->submenu));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, QuickViewWidget, widget_get_view(app->widget));

    app->button_menu = button_menu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, QuickViewButtonMenu, button_menu_get_view(app->button_menu));

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, QuickViewTextBox, text_box_get_view(app->text_box));

    app->selected_path = furi_string_alloc();
    app->ir_worker     = infrared_worker_alloc();
    app->ir_transmitting = false;

    app->nfc          = NULL;
    app->nfc_device   = NULL;
    app->nfc_listener = NULL;

    app->rfid_dict   = NULL;
    app->rfid_worker = NULL;

    app->mode = QuickModeNfc;

    return app;
}

void quick_app_free(QuickApp* app) {
    // Stop any ongoing IR transmission
    if(app->ir_transmitting) {
        infrared_worker_tx_stop(app->ir_worker);
        app->ir_transmitting = false;
    }
    infrared_worker_free(app->ir_worker);

    // Stop and free NFC if still running (should be cleaned up by scene exit)
    if(app->nfc_listener) {
        nfc_listener_stop(app->nfc_listener);
        nfc_listener_free(app->nfc_listener);
    }
    if(app->nfc_device) nfc_device_free(app->nfc_device);
    if(app->nfc)        nfc_free(app->nfc);

    // Stop and free RFID if still running (should be cleaned up by scene exit)
    if(app->rfid_worker) {
        lfrfid_worker_stop(app->rfid_worker);
        lfrfid_worker_stop_thread(app->rfid_worker);
        lfrfid_worker_free(app->rfid_worker);
    }
    if(app->rfid_dict) protocol_dict_free(app->rfid_dict);

    furi_string_free(app->selected_path);

    view_dispatcher_remove_view(app->view_dispatcher, QuickViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, QuickViewWidget);
    view_dispatcher_remove_view(app->view_dispatcher, QuickViewButtonMenu);
    view_dispatcher_remove_view(app->view_dispatcher, QuickViewTextBox);
    view_dispatcher_free(app->view_dispatcher);

    scene_manager_free(app->scene_manager);

    submenu_free(app->submenu);
    widget_free(app->widget);
    button_menu_free(app->button_menu);
    text_box_free(app->text_box);

    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t quick_app_main(void* arg) {
    UNUSED(arg);

    QuickApp* app = quick_app_alloc();

    scene_manager_next_scene(app->scene_manager, QuickSceneMain);
    view_dispatcher_run(app->view_dispatcher);

    quick_app_free(app);
    return 0;
}
