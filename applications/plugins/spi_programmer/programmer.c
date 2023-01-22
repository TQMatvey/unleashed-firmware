#include "programmer_i.h"

SpiProgrammer* spi_programmer_alloc() {
    SpiProgrammer* spi_programmer = malloc(sizeof(SpiProgrammer));

    spi_programmer->view_dispatcher = view_dispatcher_alloc();
    spi_programmer->scene_manager = scene_manager_alloc(&spi_programmer_scene_handlers, spi_programmer);
    view_dispatcher_enable_queue(spi_programmer->view_dispatcher);
    view_dispatcher_set_event_callback_context(spi_programmer->view_dispatcher, spi_programmer);
    view_dispatcher_set_custom_event_callback(
        spi_programmer->view_dispatcher, spi_programmer_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        spi_programmer->view_dispatcher, spi_programmer_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        spi_programmer->view_dispatcher, spi_programmer_tick_event_callback, 100);

    // Open GUI record
    spi_programmer->gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(
        spi_programmer->view_dispatcher, spi_programmer->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    spi_programmer->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Submenu
    spi_programmer->submenu = submenu_alloc();
    view_dispatcher_add_view(
        spi_programmer->view_dispatcher, NfcMagicViewMenu, submenu_get_view(spi_programmer->submenu));

    // Popup
    spi_programmer->popup = popup_alloc();
    view_dispatcher_add_view(
        spi_programmer->view_dispatcher, NfcMagicViewPopup, popup_get_view(spi_programmer->popup));

    // Loading
    spi_programmer->loading = loading_alloc();
    view_dispatcher_add_view(
        spi_programmer->view_dispatcher, NfcMagicViewLoading, loading_get_view(spi_programmer->loading));

    // Text Input
    spi_programmer->text_input = text_input_alloc();
    view_dispatcher_add_view(
        spi_programmer->view_dispatcher,
        NfcMagicViewTextInput,
        text_input_get_view(spi_programmer->text_input));

    // Custom Widget
    spi_programmer->widget = widget_alloc();
    view_dispatcher_add_view(
        spi_programmer->view_dispatcher, NfcMagicViewWidget, widget_get_view(spi_programmer->widget));

    return spi_programmer;
}

void spi_programmer_free(NfcMagic* spi_programmer) {
    furi_assert(spi_programmer);

    // Submenu
    view_dispatcher_remove_view(spi_programmer->view_dispatcher, NfcMagicViewMenu);
    submenu_free(spi_programmer->submenu);

    // Popup
    view_dispatcher_remove_view(spi_programmer->view_dispatcher, NfcMagicViewPopup);
    popup_free(spi_programmer->popup);

    // Loading
    view_dispatcher_remove_view(spi_programmer->view_dispatcher, NfcMagicViewLoading);
    loading_free(spi_programmer->loading);

    // TextInput
    view_dispatcher_remove_view(spi_programmer->view_dispatcher, NfcMagicViewTextInput);
    text_input_free(spi_programmer->text_input);

    // Custom Widget
    view_dispatcher_remove_view(spi_programmer->view_dispatcher, NfcMagicViewWidget);
    widget_free(spi_programmer->widget);

    // Worker
    spi_programmer_worker_stop(spi_programmer->worker);
    spi_programmer_worker_free(spi_programmer->worker);

    // View Dispatcher
    view_dispatcher_free(spi_programmer->view_dispatcher);

    // Scene Manager
    scene_manager_free(spi_programmer->scene_manager);

    // GUI
    furi_record_close(RECORD_GUI);
    spi_programmer->gui = NULL;

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    spi_programmer->notifications = NULL;

    free(spi_programmer);
}

int32_t programmer_app(void* p) {
    UNUSED(p);
    SpiProgrammer* spi_programmer = spi_programmer_alloc();

    scene_manager_next_scene(spi_programmer->scene_manager, SpiProgrammerSceneStart);

    view_dispatcher_run(spi_programmer->view_dispatcher);

    spi_programmer_free(spi_programmer);

    return 0;
}