#include "sage_base.h"
#include "stdlib.h"
#include "stdio.h"
#include "log.h"

SgResult sgOpenFile(const char* path, SgFile **ppFile) {
	FILE *file = fopen(path, "rb");
	if (!file) {
		log_warn("[Base]: On path < %s > file not found", path);
		return 1;
	}
	fseek(file, 0, SEEK_END);
	uint32_t length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer;
	SG_MALLOC_NUM(buffer, length);
	if (!buffer) {
		log_error("[Base]: File allocation failure");
		return -1;
	}
	size_t rc = fread(buffer, sizeof(char), length, file);
	if (rc != (size_t)length) {
		log_warn("[Base]: File read failure");
	}
	fclose(file);
	SgFile *pFile = malloc(sizeof(pFile[0]));
	pFile->pBytes = (uint32_t*) buffer;
	pFile->size   = length;
	*ppFile = pFile;

	return SG_SUCCESS;
}

SgResult sgCloseFile(SgFile **ppFile) {
	if((*ppFile)->pBytes){
		free((*ppFile)->pBytes);
		free(*ppFile);
	}
	*ppFile = NULL;
	return SG_SUCCESS;
}

