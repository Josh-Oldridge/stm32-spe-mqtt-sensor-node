#ifndef FRAMES_H
#define FRAMES_H
#ifndef USE_LWIP
#include <stdint.h>
#include "adi_mac.h"

extern uint8_t macAddr[2][6];
extern uint8_t testFrames[TEST_FRAMES_COUNT][MAX_FRAME_SIZE];
#endif /* USE_LWIP */
#endif /* FRAMES_H */
