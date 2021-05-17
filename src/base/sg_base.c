#include "sg_base.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

uint64_t fsize(FILE* fp, const char* path) {
	// Get file length
	struct stat st;
	if (stat(path, &st) == 0)
		return st.st_size;
	sgLogError("[Base]: Cannot determine size of %s: %s", path, strerror(errno));
	return -1;
}

SgResult sgOpenFile(const char* path, SgFile** ppFile) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		sgLogWarn("[Base]: On path < %s > file not found", path);
		return 1;
	}
	uint64_t length = fsize(file, path);

	char* buffer;
	SG_CALLOC_NUM(buffer, length + 1);
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
	pFile->pBytes = buffer;
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

#else
#ifdef _WIN32

SgResult sgOpenFile(const char* path, SgFile** ppFile) {
	SgFile* pFile;
	SG_CALLOC_NUM(pFile, 1);
	pFile->hFile =
	    CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
	               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS, NULL);
	if (pFile->hFile == INVALID_HANDLE_VALUE) {
		sgLogWarn("[Base]: On path < %s > file not found", path);
		return 1;
	}

	GetFileSizeEx(pFile->hFile, &pFile->sizeLI);
	pFile->size = pFile->sizeLI.QuadPart;
	SG_CALLOC_NUM(pFile->pBytes, pFile->size + 1);

	WINBOOL res = ReadFile(pFile->hFile, pFile->pBytes, pFile->size, NULL, NULL);
	WaitForSingleObjectEx(pFile->hFile, 0, TRUE);
	if (!res) {
		sgLogInfo("\n%d", GetLastError());
	}

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
		CloseHandle((*ppFile)->hFile);
		free((*ppFile)->pBytes);
		free(*ppFile);
	}
	*ppFile = NULL;
	return SG_SUCCESS;
}
#endif
#endif
