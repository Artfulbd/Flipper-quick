#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/text_box.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_listener.h>
#include <nfc/protocols/nfc_protocol.h>
#include <infrared/worker/infrared_worker.h>
#include <infrared.h>
#include <lfrfid/lfrfid_worker.h>
#include <lfrfid/lfrfid_dict_file.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <toolbox/protocols/protocol_dict.h>
#include <notification/notification_messages.h>

#include "scenes/quick_scene.h"

#define QUICK_NFC_FAV_PATH  EXT_PATH("apps_data/quick/nfc_favourites.txt")
#define QUICK_IR_FAV_PATH   EXT_PATH("apps_data/quick/ir_favourites.txt")
#define QUICK_RFID_FAV_PATH EXT_PATH("apps_data/quick/rfid_favourites.txt")
#define QUICK_DATA_DIR      EXT_PATH("apps_data/quick")

#define QUICK_MAX_FAVS      16
#define QUICK_MAX_IR_SIGS   32
#define QUICK_NAME_LEN      48
#define QUICK_PATH_LEN      256
#define QUICK_MAX_TIMINGS   1024

#define QUICK_SELECT_INDEX  0xFE

typedef enum {
    QuickModeNfc,
    QuickModeIr,
    QuickModeRfid,
} QuickMode;

typedef enum {
    QuickViewSubmenu,
    QuickViewWidget,
    QuickViewButtonMenu,
    QuickViewTextBox,
} QuickView;

typedef enum {
    QuickCustomEventIrTxStart = 0x0100, // lower byte encodes signal index
    QuickCustomEventIrTxStop  = 0x0200,
} QuickCustomEvent;

typedef struct QuickApp {
    SceneManager*     scene_manager;
    ViewDispatcher*   view_dispatcher;
    Submenu*          submenu;
    Widget*           widget;
    ButtonMenu*       button_menu;
    TextBox*          text_box;
    NotificationApp*  notifications;

    QuickMode       mode;
    FuriString*     selected_path;   // full path of selected NFC or IR file
    char            selected_name[QUICK_NAME_LEN]; // display name of selected file

    // NFC emulation
    Nfc*            nfc;
    NfcDevice*      nfc_device;
    NfcListener*    nfc_listener;

    // RFID (125kHz LFRFID) emulation
    ProtocolDict*   rfid_dict;
    LFRFIDWorker*   rfid_worker;
    ProtocolId      rfid_protocol_id;

    // IR transmission
    InfraredWorker* ir_worker;
    bool            ir_transmitting;

    // IR signal names (kept alive for ButtonMenu)
    char            ir_signal_names[QUICK_MAX_IR_SIGS][QUICK_NAME_LEN];
    size_t          ir_signal_count;

    // IR signal data (loaded on demand when button pressed)
    uint32_t        ir_timings[QUICK_MAX_TIMINGS];
    size_t          ir_timings_count;
    uint32_t        ir_frequency;
    float           ir_duty_cycle;
    bool            ir_is_raw;
    InfraredMessage ir_message;
} QuickApp;

QuickApp* quick_app_alloc(void);
void      quick_app_free(QuickApp* app);
