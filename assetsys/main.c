#include "sg_assets.h"
#include "sg_log.h"
#include "sg_math.h"
#include "stdlib.h"

int main(int argc, const char* argv[]) {
	const char* filesPath;
	const char* outDir;
	const char* inDir;
	if (argc <= 1) {
		sgLogWarn(
		    "No args provided. Sample args: [path/to/files.json] [output/dir] "
		    "[in/dir]");
		exit(0);
	}
	if (argc <= 2) {
		sgLogInfo("No output dir provided. Will use '.' ");
		outDir = ".";
	} else {
		outDir = argv[2];
	}
	if (argc <= 3) {
		sgLogInfo("No input dir provided. Will use '.' ");
		inDir = ".";
	} else {
		inDir = argv[3];
	}
	filesPath = argv[1];
	SgAssets inAssets;
	SgAssets outAssets;
	sgAssetsRead(argv[1], inDir, &inAssets);
	sgAssetsCompress(&inAssets, &outAssets);
	sgAssetsWrite(&outAssets, outDir);
	sgAssetsClear(&inAssets);
	sgAssetsClear(&outAssets);
	return 0;
}
