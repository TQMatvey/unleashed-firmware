#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>

#include <input/input.h>

#include "scenes/nfc_magic_scene.h"

#include <storage/storage.h>
#include <lib/toolbox/path.h>

struct SpiProgrammer {
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;

    FuriString* text_box_store;

    // Common Views
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextInput* text_input;
    Widget* widget;
};

typedef enum {
    SpiProgrammerViewMenu,
    SpiProgrammerViewPopup,
    SpiProgrammerViewLoading,
    SpiProgrammerViewTextInput,
    SpiProgrammerViewWidget,
} SpiProgrammerView;