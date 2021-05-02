#include "sg_base.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

uint32_t fsize(FILE* fp, const char* path) {
	struct stat st;
	if (stat(path, &st) == 0)
		return st.st_size;
	sgLogError("[Base]: Cannot determine size of %s: %s", path, strerror(errno));
	return -1;
}
#else
#ifdef _WIN32
uint32_t fsize(FILE* fp, const char* path) {
	sgLogWarn("[Base]: File handling on Windows is not fully supported yet");
	uint32_t prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	uint32_t sz = ftell(fp);
	fseek(fp, prev, SEEK_SET);  // go back to where we were
	return sz;
}

#endif
#endif

SgResult sgOpenFile(const char* path, SgFile** ppFile) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		sgLogWarn("[Base]: On path < %s > file not found", path);
		return 1;
	}
	uint32_t length = fsize(file, path);

	char* buffer;
	SG_CALLOC_NUM(buffer, length);
	if (!buffer) {
		sgLogError("[Base]: File allocation failure");
		return -1;
	}
	size_t rc = fread(buffer, sizeof(buffer[0]), length, file);
	if (rc != (size_t)length) {
		sgLogWarn("[Base]: File read failure");
	}
	fclose(file);

	SgFile* pFile;
	SG_CALLOC_NUM(pFile, 1);
	pFile->pBytes = (uint32_t*)buffer;
	pFile->size = length;
	*ppFile = pFile;

	return SG_SUCCESS;
}

SgResult sgWriteFile(const char* path, SgFile* pFile) {
	FILE* file = fopen(path, "wb");
	if (!file) {
		sgLogWarn("[Base]: On path < %s > file could not be created/opened", path);
		return 1;
	}
	fwrite(pFile->pBytes, sizeof(char), pFile->size, file);
	fclose(file);
	return SG_SUCCESS;
}

SgResult sgCloseFile(SgFile** ppFile) {
	if ((*ppFile)->pBytes) {
		free((*ppFile)->pBytes);
		free(*ppFile);
	}
	*ppFile = NULL;
	return SG_SUCCESS;
}
