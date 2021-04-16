#ifndef SAGE_LOG
#define SAGE_LOG

#include "log.h"

enum { SG_LOG_TRACE, SG_LOG_DEBUG, SG_LOG_INFO, SG_LOG_WARN, SG_LOG_ERROR, SG_LOG_FATAL };
#define sgLog(...) log_log(__VA_ARGS__)

#define sgLogTrace(...) sgLog(SG_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogDebug(...) sgLog(SG_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogInfo(...)  sgLog(SG_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define sgLogWarn(...)  sgLog(SG_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define sgLogError(...) sgLog(SG_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogFatal(...) sgLog(SG_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#ifdef _DEBUG
#define sgLogTrace_Debug(...) sgLog(SG_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogDebug_Debug(...) sgLog(SG_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogInfo_Debug(...)  sgLog(SG_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define sgLogWarn_Debug(...)  sgLog(SG_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define sgLogError_Debug(...) sgLog(SG_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define sgLogFatal_Debug(...) sgLog(SG_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#define sgLogTrace_Debug(...) 
#define sgLogDebug_Debug(...) 
#define sgLogInfo_Debug(...)  
#define sgLogWarn_Debug(...)  
#define sgLogError_Debug(...) 
#define sgLogFatal_Debug(...) 
#endif

#endif
