#pragma once

#include "inc.h"

void bitbangSetup();
void bitbangWs2812(uint8_t ledCount, __xdata uint8_t * ledData);
