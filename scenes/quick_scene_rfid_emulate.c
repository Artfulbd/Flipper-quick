#include "../quick.h"

static void quick_rfid_emulate_show_error(QuickApp* app, const char* msg) {
    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, msg);
    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewWidget);
}

void quick_scene_rfid_emulate_on_enter(void* context) {
    QuickApp* app = context;

    app->rfid_dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    app->rfid_protocol_id = lfrfid_dict_file_load(
        app->rfid_dict, furi_string_get_cstr(app->selected_path));

    if(app->rfid_protocol_id == PROTOCOL_NO) {
        protocol_dict_free(app->rfid_dict);
        app->rfid_dict = NULL;
        quick_rfid_emulate_show_error(app, "Failed to\nload RFID file");
        return;
    }

    app->rfid_worker = lfrfid_worker_alloc(app->rfid_dict);
    lfrfid_worker_start_thread(app->rfid_worker);
    lfrfid_worker_emulate_start(app->rfid_worker, (LFRFIDProtocol)app->rfid_protocol_id);

    // Blink LED like the native RFID app does while emulating
    notification_message(app->notifications, &sequence_blink_start_magenta);

    // Show emulate screen
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget, 64, 4, AlignCenter, AlignTop, FontPrimary, "Emulating");
    widget_add_string_element(
        app->widget, 64, 20, AlignCenter, AlignTop, FontSecondary, app->selected_name);
    widget_add_string_element(
        app->widget, 64, 54, AlignCenter, AlignBottom, FontSecondary, "Press Back to stop");
    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewWidget);
}

bool quick_scene_rfid_emulate_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void quick_scene_rfid_emulate_on_exit(void* context) {
    QuickApp* app = context;

    if(app->rfid_worker) {
        lfrfid_worker_stop(app->rfid_worker);
        lfrfid_worker_stop_thread(app->rfid_worker);
        lfrfid_worker_free(app->rfid_worker);
        app->rfid_worker = NULL;
        notification_message(app->notifications, &sequence_blink_stop);
    }
    if(app->rfid_dict) {
        protocol_dict_free(app->rfid_dict);
        app->rfid_dict = NULL;
    }

    widget_reset(app->widget);
}
