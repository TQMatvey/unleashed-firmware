#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/nfc/nfc_device.h>
#include <notification/notification_messages.h>

typedef struct FieldDetector FieldDetector;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

struct FieldDetector {
    ViewPort* view_port;
    Gui* gui;
    NotificationApp* notifications;
    FuriMessageQueue* input_queue;
};
