#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "log.h"


int main() {
	SgAppCreateInfo createInfo = {
		.pName = "Space Invaders",
		.size  = {640, 480},
//		.flags = SG_APP_WINDOW_FULLSCREEN,
	};

	SgApp app;

	log_info("[AppInit]: %d", sgCreateApp(&createInfo, &app));
	while(sgAppUpdate(&app)){
	}
	return 0;
}
