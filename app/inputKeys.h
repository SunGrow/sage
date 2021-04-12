#ifndef INPUT_KEYS
#define INPUT_KEYS 1

#include "sage.h"
#include "sage_rend.h"
#include "sage_input.h"


SgInputSignal inputSignals[] = {
	{
		.key     = GLFW_KEY_I,
		.keyName = "i",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_H,
		.keyName = "h",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_N,
		.keyName = "n",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_L,
		.keyName = "l",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_K,
		.keyName = "k",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_J,
		.keyName = "j",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_W,
		.keyName = "w",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_A,
		.keyName = "a",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_S,
		.keyName = "s",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_D,
		.keyName = "d",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_SPACE,
		.keyName = "space",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_KEY_Q,
		.keyName = "q",
		.type    = SG_INPUT_TYPE_KEYBOARD,
	},
	{
		.key     = GLFW_MOUSE_BUTTON_1,
		.keyName = "left",
		.type    = SG_INPUT_TYPE_MOUSE,
	},
};
#endif
