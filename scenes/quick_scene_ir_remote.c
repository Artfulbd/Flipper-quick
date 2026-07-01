#include "../quick.h"
#include "../quick_ir_parser.h"

static void quick_scene_ir_remote_button_callback(void* context, int32_t index, InputType type) {
    QuickApp* app = context;
    if(type == InputTypePress) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher,
            (uint32_t)(QuickCustomEventIrTxStart | (index & 0xFF)));
    } else if(type == InputTypeRelease) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, QuickCustomEventIrTxStop);
    }
}

static void quick_ir_tx_start(QuickApp* app) {
    if(app->ir_transmitting) return;

    if(app->ir_is_raw) {
        infrared_worker_set_raw_signal(
            app->ir_worker,
            app->ir_timings,
            app->ir_timings_count,
            app->ir_frequency,
            app->ir_duty_cycle);
    } else {
        infrared_worker_set_decoded_signal(app->ir_worker, &app->ir_message);
    }

    infrared_worker_tx_set_get_signal_callback(
        app->ir_worker,
        infrared_worker_tx_get_signal_steady_callback,
        app);
    infrared_worker_tx_start(app->ir_worker);
    app->ir_transmitting = true;
}

static void quick_ir_tx_stop(QuickApp* app) {
    if(!app->ir_transmitting) return;
    infrared_worker_tx_stop(app->ir_worker);
    infrared_worker_tx_set_get_signal_callback(app->ir_worker, NULL, NULL);
    app->ir_transmitting = false;
}

void quick_scene_ir_remote_on_enter(void* context) {
    QuickApp* app = context;
    button_menu_reset(app->button_menu);

    const char* path = furi_string_get_cstr(app->selected_path);

    if(!quick_ir_list_signals(app, path)) {
        // Show error via widget
        widget_reset(app->widget);
        widget_add_string_multiline_element(
            app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, "Failed to\nload IR file");
        view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewWidget);
        return;
    }

    button_menu_set_header(app->button_menu, app->selected_name);

    for(size_t i = 0; i < app->ir_signal_count; i++) {
        button_menu_add_item(
            app->button_menu,
            app->ir_signal_names[i],
            (int32_t)i,
            quick_scene_ir_remote_button_callback,
            ButtonMenuItemTypeCommon,
            app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewButtonMenu);
}

bool quick_scene_ir_remote_on_event(void* context, SceneManagerEvent event) {
    QuickApp* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        if(app->ir_transmitting) {
            quick_ir_tx_stop(app);
            return true; // consume Back while transmitting
        }
        return false;
    }

    if(event.type == SceneManagerEventTypeCustom) {
        uint32_t ev = event.event;

        if((ev & 0xFF00) == (uint32_t)QuickCustomEventIrTxStart) {
            size_t signal_idx = ev & 0xFF;
            const char* path = furi_string_get_cstr(app->selected_path);
            if(quick_ir_load_signal(app, path, signal_idx)) {
                quick_ir_tx_start(app);
            }
            return true;
        }

        if(ev == (uint32_t)QuickCustomEventIrTxStop) {
            quick_ir_tx_stop(app);
            return true;
        }
    }

    return false;
}

void quick_scene_ir_remote_on_exit(void* context) {
    QuickApp* app = context;
    quick_ir_tx_stop(app);
    button_menu_reset(app->button_menu);
    widget_reset(app->widget);
}
