#ifndef INPUT_KEYS
#define INPUT_KEYS 1

#include "sage.h"
#include "sage_input.h"
#include "sage_rend.h"

SgInputSignal inputSignals[] = {
    {
        .key     = GLFW_KEY_I,
        .keyName = "I",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_H,
        .keyName = "H",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_N,
        .keyName = "N",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_L,
        .keyName = "L",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_K,
        .keyName = "K",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_J,
        .keyName = "J",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_W,
        .keyName = "W",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_A,
        .keyName = "A",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_S,
        .keyName = "S",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_D,
        .keyName = "D",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_SPACE,
        .keyName = "SPACE",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_Q,
        .keyName = "Q",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_KEY_LEFT_SHIFT,
        .keyName = "LEFT_SHIFT",
        .type    = SG_INPUT_TYPE_KEYBOARD,
    },
    {
        .key     = GLFW_MOUSE_BUTTON_1,
        .keyName = "BUTTON_1",
        .type    = SG_INPUT_TYPE_MOUSE,
    },
};
#endif
