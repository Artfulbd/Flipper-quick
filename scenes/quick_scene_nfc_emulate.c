#include "../quick.h"

static bool quick_nfc_protocol_is_supported(NfcProtocol protocol) {
    switch(protocol) {
    case NfcProtocolIso14443_3a:
    case NfcProtocolIso14443_4a:
    case NfcProtocolIso15693_3:
    case NfcProtocolFelica:
    case NfcProtocolMfUltralight:
    case NfcProtocolMfClassic:
    case NfcProtocolSlix:
        return true;
    default:
        return false;
    }
}

static void quick_nfc_emulate_show_error(QuickApp* app, const char* msg) {
    widget_reset(app->widget);
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, msg);
    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewWidget);
}

void quick_scene_nfc_emulate_on_enter(void* context) {
    QuickApp* app = context;

    // Load NFC device from file
    app->nfc        = nfc_alloc();
    app->nfc_device = nfc_device_alloc();

    if(!nfc_device_load(app->nfc_device, furi_string_get_cstr(app->selected_path))) {
        quick_nfc_emulate_show_error(app, "Failed to\nload NFC file");
        return;
    }

    NfcProtocol protocol = nfc_device_get_protocol(app->nfc_device);
    if(!quick_nfc_protocol_is_supported(protocol)) {
        quick_nfc_emulate_show_error(app, "Card type not\nsupported for\nemulation");
        return;
    }

    const NfcDeviceData* data = nfc_device_get_data(app->nfc_device, protocol);
    app->nfc_listener = nfc_listener_alloc(app->nfc, protocol, data);
    nfc_listener_start(app->nfc_listener, NULL, NULL);

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

bool quick_scene_nfc_emulate_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void quick_scene_nfc_emulate_on_exit(void* context) {
    QuickApp* app = context;

    if(app->nfc_listener) {
        nfc_listener_stop(app->nfc_listener);
        nfc_listener_free(app->nfc_listener);
        app->nfc_listener = NULL;
    }
    if(app->nfc_device) {
        nfc_device_free(app->nfc_device);
        app->nfc_device = NULL;
    }
    if(app->nfc) {
        nfc_free(app->nfc);
        app->nfc = NULL;
    }

    widget_reset(app->widget);
}
