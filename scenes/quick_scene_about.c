#include "../quick.h"

static const char about_text[] =
    "Developed by artful\n"
    "\n"
    "Quick lets you emulate NFC\n"
    "cards and send IR signals\n"
    "quickly without navigating\n"
    "the native apps.\n"
    "\n"
    "-- NFC --\n"
    "Select your favourite cards\n"
    "and press OK to emulate\n"
    "Press Back to stop.\n"
    "\n"
    "-- IR --\n"
    "Select your remote/s,\n"
    "pick a button, then hold OK\n"
    "to transmit. Release to stop.\n"
    "\n"
    "-- Favourites --\n"
    "Use 'Select from saved' to\n"
    "browse your NFC or IR files.\n"
    "It will automatically detect\n"
    "available saved files within\n"
    "the system.\n"
    "[+] = already a favourite\n"
    "[ ] = not yet added\n"
    "Tap any file to toggle it.";

void quick_scene_about_on_enter(void* context) {
    QuickApp* app = context;
    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    text_box_set_text(app->text_box, about_text);
    view_dispatcher_switch_to_view(app->view_dispatcher, QuickViewTextBox);
}

bool quick_scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void quick_scene_about_on_exit(void* context) {
    QuickApp* app = context;
    text_box_reset(app->text_box);
}
