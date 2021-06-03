#include "sg_assets.h"
#include "sg_log.h"
#include "sg_math.h"
#include "stdlib.h"

int main(int argc, const char* argv[]) {
	const char* projectsPath;
	if (argc <= 1) {
		sgLogWarn("No args provided. Sample args: [path/to/projects.json]");
		exit(0);
	}
	projectsPath = argv[1];
	SgProjects projects;
	sgLoadProjectDirs(projectsPath, &projects);
	SgProjectLayout projectLayout;
	SgProject*      pProjectArr;
	SG_CALLOC_NUM(pProjectArr, projects.count);
	for (SgSize i = 0; i < projects.count; ++i) {
		sgLoadProjectLayout(projects.ppProjectDirs[i], &projectLayout);
		sgLogInfo("Started file read");

		sgProjectRead(projects.ppProjectDirs[i], &projectLayout, &pProjectArr[i]);
		sgProjectWrite(&pProjectArr[i]);

		sgLogInfo("File write completed");
	}

	return 0;
}
