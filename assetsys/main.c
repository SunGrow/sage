#include "sg_assets.h"
#include "sg_log.h"
#include "sg_math.h"
#include "stdlib.h"

int main(int argc, const char* argv[]) {
	const char* projectPath;
	if (argc <= 1) {
		sgLogWarn(
		    "No args provided. Sample args: [path/to/project.json]");
		exit(0);
	}
	projectPath = argv[1];
	SgProjectLayout projectLayout;
	sgLoadProjectLayout(projectPath, &projectLayout);
	sgLogInfo("%s %s %s %s %s %s", projectLayout.generalSettingsPath, projectLayout.keyContextsPath, projectLayout.filesPath, projectLayout.scenesPath, projectLayout.dataPath, projectLayout.buildDir);

	sgLogInfo("Started file read");
	SgAssets inAssets;
	SgAssets outAssets;
	sgFilesRead(projectLayout.filesPath, ".", &inAssets);
	sgLogInfo("Files read successfully. Starting file compression");
	sgFilesCompress(&inAssets, &outAssets);
	sgLogInfo("Files compression completed. Starting file write");
	sgFilesWrite(&outAssets, projectLayout.buildDir);
	sgLogInfo("File write completed");
	sgFilesClear(&inAssets);
	sgFilesClear(&outAssets);
	//sgLogInfo("Started scene generation");
	//sgLogInfo("Scene generation completed");
	return 0;
}
