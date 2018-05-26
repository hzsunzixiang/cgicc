#pragma once

#include <sys/cdefs.h>
#include <stdio.h>
#include <sys/types.h>  
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "singleton_t.h"
#include "sync_lock.h"


__BEGIN_DECLS


#define LOGSIZE              1 * 1024 * 1024
#define MAX_PATH_LEN         512 


// end of line.
#define CLIB_LOG_EOL_LF         "\n"
#define CLIB_LOG_EOL_CRLF       "\r\n"
#define CLIB_LOG_EOL_NULL       ""

// log file size
#define CLIB_LOG_SIZE_UNLI      0                   ///< Unlimited
#define CLIB_LOG_SIZE_SMAL      1024 * 1024         ///< 1MB
#define CLIB_LOG_SIZE_NORM      1024 * 1024 * 10    ///< 10MB
#define CLIB_LOG_SIZE_LARG      1024 * 1024 * 100   ///< 100MB
#define CLIB_LOG_SIZE_HUGE      1024 * 1024 * 1024  ///< 1GB


// log type.
#define CLIB_LOG_TYPE_NOT      0   ///< Not roll log.
#define CLIB_LOG_TYPE_DAY      1   ///< Day roll log.
#define CLIB_LOG_TYPE_MON      2   ///< Month roll log.

#define FILE_TIME_FORMAT "%04d%02d%02d"

#define BY_LOG_FORMAT_PRE             "%s[%04d-%02d-%02d %02d:%02d:%02d.%06ld]%s" 
#define BY_LOGERR_FORMAT_PRE             "%s[%04d-%02d-%02d %02d:%02d:%02d.%06ld]%s" 
#define BY_LOG_FORMAT_PRE_BY          "%s[%04d-%02d-%02d %02d:%02d:%02d.%06ld]" 
#define BY_LOG_ERR_INFO_FORMAT        "[%s:%d]<%s> EN:%d,"
#define BY_LOG_DEBUG_INFO_FORMAT      "[%s:%d]<%s> "
#define BY_LOG_KEY_INFO_FORMAT        "[%s:%d]<%s> "
#define BY_LOG_DEBUG_INFO_CONTENT     basename(__FILE__),__LINE__,__func__

enum 
{
	LOCAL_LOG    = 0,
	REMOTE_LOG   = 1,
};
#define LOCAL_IP                "127.0.0.1"
#define DEFAULT_PID             0
#define DEFAULT_SVID            0
#define UDP_LOG_FORMAT          "{\"tv\":\"%ld.%03ld\",\"sid\":\"%d\",\"svid\":\"%d\",\"logtype\":\"%s\",\"seq\":\"%d\",\"loglevel\":\"%d\",\"tid\":\"%d\",\"uid\":\"%d\",\"ip\":\"%s\",\"port\":\"%d\",\"msg\":\"%s\"}"

// 为了兼容udp日志  
#define CLIB_LOG_LEV_ERROR      0
#define CLIB_LOG_LEV_INFO       1
#define CLIB_LOG_LEV_WARNING    2
#define CLIB_LOG_LEV_DEBUG      3
#define CLIB_LOG_LEV_TRACE      4

class clib_log
{
	private:
		//char m_sLogDir[128];    
		char m_sFileName[512];    

		int  m_iLogRotateType;    
		int  m_iMaxFileNum;    
		int  m_iMaxFileSize;   
		int  m_iDebugFd;  
		int  m_iErrorFd; 
		int  m_iKeyFd;  
		int  m_iLogFd;
		char m_sLogDebug[MAX_PATH_LEN];
		char m_sLogError[MAX_PATH_LEN];
		char m_sLogKey[MAX_PATH_LEN];
		char m_sLogFile[MAX_PATH_LEN]; // 不区分日志级别的时候

		int  m_iRemote;//是否远程写log
		char m_sFormat[LOGSIZE];
		bool m_bLogLevel;
		bool m_bLogDebug;

		int m_nRemote;//是否远程写log

		enum{MIN_LEN = 120,	MAX_PACKET_LEN = 2048};
		char m_sIp[MIN_LEN];
		char m_sLogType[MIN_LEN];
		int m_nPort; 
		int m_nPid;
		int m_nSvid;
		int m_nUdpFd; //socket句柄
		int m_nSeq;
		sockaddr_in m_cLogSrvAddr;
    	int  m_iLevel; // 为了兼容旧的接口

	public:
		clib_log() {}
		/**
		 * Initial clib_log class.
		 *
		 * @param   fileName         Log timeformat, define in CLIB_LOG_TFORMAT_x.
		 * @param   dirName          directory  
		 * @param   maxFileNum       Max log file number (0 mean no change file, maxsize no use).
		 * @param   maxFileSize      Max log file size (Byte, 0 mean unlimited).
		 * @param   logRotate        logRotate type.
		 * @param   eolType          End of line.
		 * @param   bIsDisable       enable or disable switch.
		 */
		clib_log(const char *fileName,
				const char *dirName = "./log", 
				int         maxFileNum   = 5,
				int         maxFileSize  = CLIB_LOG_SIZE_NORM,
				bool        LogDebug = true, 
				int         logRotate    = CLIB_LOG_TYPE_DAY,
				bool        logLevel= true, // 为了兼容老的日志， 是否对日志分类的开关
				bool bIsDisable          = false);
		/**
		 * Destory clib_log class.
		 */
		~clib_log();

		int QuickInitForLog(const char* fileName, const char* dirName = "./log",            
				int         maxFileNum   = 5,
				int         maxFileSize  = CLIB_LOG_SIZE_NORM,
				bool        LogDebug = true, 
				int         logRotate    = CLIB_LOG_TYPE_DAY,
				bool        logLevel= true, // 为了兼容老的日志， 是否对日志分类的开关
				bool bIsDisable          = false);

		int SetLevel(int logLevel);
		int SetLogDebug( bool logDebug);

		int GetLevel();
		void RotateFile(char (&logFileName)[MAX_PATH_LEN], int& logFd, char const* logType);

		void WriteLogGeneral(char const * format, va_list ap, char (&logFileName)[MAX_PATH_LEN], int & logFd, char const* logType);
		/**
		 * @brief 调试log
		 */          
		void LogDebugMsg(char const* sFormat,...)__attribute__((__format__ (__printf__, 2, 3)));

		/**
		 * @brief 错误log
		 */          
		void LogErrMsg(char const* sFormat,...)__attribute__((__format__ (__printf__, 2, 3)));


		/**
		 * @brief 运营log 
		 */          
		void LogKeyMsg(char const* sFormat,...)__attribute__((__format__ (__printf__, 2, 3)));

		// 不区分日志级别的时候
		void LogMsgInfo(char const* sFormat,...)__attribute__((__format__ (__printf__, 2, 3)));

		void SetMaxFileNum( int maxFileNum )
		{
			m_iMaxFileNum = maxFileNum;
		}

		void SetMaxFileSize( int maxFileSize)
		{
			m_iMaxFileSize = maxFileSize;
		}

		void setLogRotateType(int logRotateType)
		{
			m_iLogRotateType = logRotateType;
		}
		void setLogLevel(int logLevel)
		{
			m_bLogLevel = logLevel;
		}

		int udpwrite(const char *strMsg, int tid, int uid);

		void start_udp_log(int iflog, const char* strLogType)
		{
			m_nRemote = iflog;
			snprintf(m_sLogType, MIN_LEN, "%s", strLogType);
		}

		int sremote( const int nPort, const int nlocalPort,
				const int nSvid    = DEFAULT_SVID,
				const int nPid     = DEFAULT_PID,
				const char* pszlocalIp  = LOCAL_IP,
				const char* pszlogIp = LOCAL_IP);




};

extern int __log_level__;  //reactor log
extern int __cclog_level__;//server log
extern bool __log_stdout__;

#define INVALID_FILE_FD     -1
#define DEFAULT_FILE_NUM    3
#define DEFAULT_FILE_SIZE   50*1024*1024



const char * const BY_LOG_DEBUG_MARK      = "[DEG]";
const char * const BY_LOG_ERROR_MARK      = "[ERR]";
const char * const BY_LOG_KEY_MARK        = "[KEY]";

#define LogMsg(sformat,args...) LogMsgInfo(BY_LOG_DEBUG_INFO_FORMAT sformat, BY_LOG_DEBUG_INFO_CONTENT, ##args)

//for singleton use, in service.
typedef CSingletonT<clib_log, CDummyLock> BYLogSingleton;

#define BY_LOGMSG BYLogSingleton::Instance()

#define LogError(errCode,sformat,args...)do{\
	BY_LOGMSG->LogErrMsg(BY_LOG_ERR_INFO_FORMAT sformat, BY_LOG_DEBUG_INFO_CONTENT, errCode,  ##args);\
}while(0)

#define LogDebug(sformat,args...)do{\
	BY_LOGMSG->LogDebugMsg(BY_LOG_DEBUG_INFO_FORMAT sformat, BY_LOG_DEBUG_INFO_CONTENT, ##args);\
}while(0)

#define LogKey(sformat,args...)do{\
	BY_LOGMSG->LogKeyMsg(BY_LOG_DEBUG_INFO_FORMAT sformat, BY_LOG_DEBUG_INFO_CONTENT, ##args);\
}while(0)

__END_DECLS

