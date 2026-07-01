#include "quick_ir_parser.h"
#include "quick.h"

#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <infrared.h>
#include <string.h>

#define TAG            "QuickIR"
#define IR_FILE_HEADER "IR signals file"

static FlipperFormat* quick_ir_open(const char* path) {
    Storage*      storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff      = flipper_format_buffered_file_alloc(storage);
    // Note: storage record stays open; it's closed when ff is freed
    if(!flipper_format_buffered_file_open_existing(ff, path)) {
        flipper_format_free(ff);
        furi_record_close(RECORD_STORAGE);
        return NULL;
    }
    return ff;
}

static void quick_ir_close(FlipperFormat* ff) {
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
}

static bool quick_ir_validate_header(FlipperFormat* ff) {
    FuriString* tmp = furi_string_alloc();
    uint32_t    version;
    bool ok = flipper_format_read_header(ff, tmp, &version) &&
              furi_string_equal_str(tmp, IR_FILE_HEADER);
    furi_string_free(tmp);
    return ok;
}

bool quick_ir_list_signals(QuickApp* app, const char* path) {
    FlipperFormat* ff = quick_ir_open(path);
    if(!ff) return false;

    bool ok = false;
    app->ir_signal_count = 0;

    if(quick_ir_validate_header(ff)) {
        FuriString* name = furi_string_alloc();
        while(app->ir_signal_count < QUICK_MAX_IR_SIGS) {
            if(!flipper_format_read_string(ff, "name", name)) break;
            strlcpy(
                app->ir_signal_names[app->ir_signal_count],
                furi_string_get_cstr(name),
                QUICK_NAME_LEN);
            app->ir_signal_count++;
        }
        furi_string_free(name);
        ok = (app->ir_signal_count > 0);
    }

    quick_ir_close(ff);
    return ok;
}

bool quick_ir_load_signal(QuickApp* app, const char* path, size_t index) {
    FlipperFormat* ff = quick_ir_open(path);
    if(!ff) return false;

    bool ok = false;

    do {
        if(!quick_ir_validate_header(ff)) break;

        FuriString* tmp = furi_string_alloc();

        // Advance past (index) signal names to reach signal[index]
        for(size_t i = 0; i <= index; i++) {
            if(!flipper_format_read_string(ff, "name", tmp)) {
                furi_string_free(tmp);
                goto done;
            }
        }

        // Read signal type
        if(!flipper_format_read_string(ff, "type", tmp)) {
            furi_string_free(tmp);
            break;
        }

        if(furi_string_equal_str(tmp, "raw")) {
            uint32_t timings_size = 0;
            if(!flipper_format_read_uint32(ff, "frequency", &app->ir_frequency, 1) ||
               !flipper_format_read_float(ff, "duty_cycle", &app->ir_duty_cycle, 1) ||
               !flipper_format_get_value_count(ff, "data", &timings_size)) {
                furi_string_free(tmp);
                break;
            }
            if(timings_size == 0 || timings_size > QUICK_MAX_TIMINGS) {
                FURI_LOG_W(TAG, "Timings count %lu out of range", (unsigned long)timings_size);
                timings_size = timings_size > QUICK_MAX_TIMINGS ? QUICK_MAX_TIMINGS : 0;
            }
            if(timings_size > 0 &&
               !flipper_format_read_uint32(ff, "data", app->ir_timings, timings_size)) {
                furi_string_free(tmp);
                break;
            }
            app->ir_timings_count = timings_size;
            app->ir_is_raw        = true;
            ok                    = true;

        } else if(furi_string_equal_str(tmp, "parsed")) {
            if(!flipper_format_read_string(ff, "protocol", tmp)) {
                furi_string_free(tmp);
                break;
            }
            app->ir_message.protocol =
                infrared_get_protocol_by_name(furi_string_get_cstr(tmp));

            if(!infrared_is_protocol_valid(app->ir_message.protocol)) {
                FURI_LOG_W(TAG, "Unknown IR protocol: %s", furi_string_get_cstr(tmp));
                furi_string_free(tmp);
                break;
            }

            // address and command are stored as hex bytes (4 bytes each)
            if(!flipper_format_read_hex(
                   ff, "address", (uint8_t*)&app->ir_message.address, 4) ||
               !flipper_format_read_hex(
                   ff, "command", (uint8_t*)&app->ir_message.command, 4)) {
                furi_string_free(tmp);
                break;
            }
            app->ir_message.repeat = false;
            app->ir_is_raw         = false;
            ok                     = true;
        }

        furi_string_free(tmp);
    } while(false);

done:
    quick_ir_close(ff);
    return ok;
}
