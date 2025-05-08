#include "haptics.h"
#include <chai3d.h>

using namespace chai3d;

cGenericHapticDevicePtr device;

bool Haptics::initDevice() {
    cHapticDeviceHandler handler;
    return handler.getDevice(device, 0) && device->open();
}

void Haptics::shutdown() {
    if(device) device->close();
}
