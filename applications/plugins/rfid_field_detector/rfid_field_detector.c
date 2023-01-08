#include "rfid_field_detector.h"

bool magic_wupa() {
    uint8_t tx_data[32] = {};
    uint8_t rx_data[32] = {};
    uint16_t rx_len = 0;

    // Start communication
    tx_data[0] = 0x52;
    furi_hal_nfc_ll_txrx_bits(
        tx_data,
        7,
        rx_data,
        sizeof(rx_data),
        &rx_len,
        FURI_HAL_NFC_LL_TXRX_FLAGS_CRC_TX_MANUAL | FURI_HAL_NFC_LL_TXRX_FLAGS_AGC_ON,
        furi_hal_nfc_ll_ms2fc(20));

    if(rx_data[0] == 0x0) return true;

    return false;
}

static void render_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    //FieldDetector* field_detector = context;

    canvas_set_font(canvas, FontPrimary);

    if(!magic_wupa()) {
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "NFC detected");
    }
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    AppEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

FieldDetector* field_detector_alloc() {
    FieldDetector* field_detector = malloc(sizeof(FieldDetector));

    //furi_hal_power_suppress_charge_enter();

    FuriHalNfcReturn ret = 0;

    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_ll_txrx_on();
    furi_hal_nfc_ll_poll();

    ret = furi_hal_nfc_ll_set_mode(
        FuriHalNfcModePollNfca, FuriHalNfcBitrate106, FuriHalNfcBitrate106);
    if(ret != FuriHalNfcReturnOk) return false;

    furi_hal_nfc_ll_set_fdt_listen(FURI_HAL_NFC_LL_FDT_LISTEN_NFCA_POLLER);
    furi_hal_nfc_ll_set_fdt_poll(FURI_HAL_NFC_LL_FDT_POLL_NFCA_POLLER);
    furi_hal_nfc_ll_set_error_handling(FuriHalNfcErrorHandlingNfc);
    furi_hal_nfc_ll_set_guard_time(FURI_HAL_NFC_LL_GT_NFCA);

    // Open Notification record
    field_detector->notifications = furi_record_open(RECORD_NOTIFICATION);

    return field_detector;
}

void field_detector_free(FieldDetector* field_detector) {
    furi_assert(field_detector);

    furi_hal_nfc_ll_txrx_off();
    furi_hal_nfc_start_sleep();

    // View Port
    view_port_enabled_set(field_detector->view_port, false);
    gui_remove_view_port(field_detector->gui, field_detector->view_port);
    view_port_free(field_detector->view_port);

    // GUI
    furi_record_close(RECORD_GUI);
    field_detector->gui = NULL;

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    field_detector->notifications = NULL;

    free(field_detector);
}

static const NotificationSequence field_detector_sequence_blink_start_blue = {
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence field_detector_sequence_blink_start_red = {
    &message_blink_start_10,
    &message_blink_set_color_red,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence field_detector_sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

void field_detector_blink_blue_start(FieldDetector* field_detector) {
    notification_message(field_detector->notifications, &field_detector_sequence_blink_start_blue);
}

void field_detector_blink_red_start(FieldDetector* field_detector) {
    notification_message(field_detector->notifications, &field_detector_sequence_blink_start_red);
}

void field_detector_blink_stop(FieldDetector* field_detector) {
    notification_message(field_detector->notifications, &field_detector_sequence_blink_stop);
}

int32_t field_detector_app(void* p) {
    UNUSED(p);
    FieldDetector* field_detector = field_detector_alloc();

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    field_detector->view_port = view_port_alloc();
    view_port_draw_callback_set(field_detector->view_port, render_callback, NULL);
    view_port_input_callback_set(field_detector->view_port, input_callback, event_queue);

    field_detector->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(field_detector->gui, field_detector->view_port, GuiLayerFullscreen);

    AppEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    if(event.input.key == InputKeyBack) processing = false;
                }
            }
        }
        view_port_update(field_detector->view_port);
    }

    furi_message_queue_free(event_queue);
    field_detector_free(field_detector);
    return 0;
}
