

#include "by_logger.h"




int __log_level__   = 6;
int __cclog_level__ = 6;
bool __log_stdout__ = false; //是否前端打印输出

/**
 * Initial clib_log class.
 *
 * @param   fileName         Log timeformat, define in CLIB_LOG_TFORMAT_x.
 * @param   timeFormat       Log timeformat, define in CLIB_LOG_TFORMAT_x.
 * @param   maxFileNum       Max log file number (0 mean no change file, maxsize no use).
 * @param   maxFileSize      Max log file size (Byte, 0 mean unlimited).
 * @param   logRotate        logRotate type.
 * @param   eolType          End of line.
 * @param   bIsDisable       enable or disable switch.
 */
clib_log::clib_log(const char *fileName,
		const char *dirName, 
		int         maxFileNum,  
		int         maxFileSize, 
		bool        logDebug, 
		int         logRotate,   
		bool        logLevel, // 为了兼容老的日志， 是否对日志分类的开关
		bool bIsDisable)         
{
	QuickInitForLog(fileName, dirName, maxFileNum, maxFileSize, logDebug, logRotate, logLevel, bIsDisable);

}

int clib_log::QuickInitForLog(const char* fileName, const char* dirName,            
		int         maxFileNum,
		int         maxFileSize,
		bool        logDebug, 
		int         logRotate,
		bool        logLevel, // 为了兼容老的日志， 是否对日志分类的开关
		bool bIsDisable)
{
	if (fileName != NULL) {
		if (dirName != NULL)
		{
			mkdir(dirName, 0777);
			if (access(dirName, W_OK|X_OK) < 0)
			{
				fprintf(stderr, "error! logdir(%s): Not writable\n", dirName);
			}
			snprintf(m_sFileName, sizeof(m_sFileName), "%s/%s", dirName, fileName);
		}
		else
			snprintf(m_sFileName, sizeof(m_sFileName), "%s", fileName);
	} 

	if(maxFileNum > 0)
		m_iMaxFileNum = maxFileNum;

	if(maxFileSize> 0)
		m_iMaxFileSize = maxFileSize;
	m_iLogRotateType = logRotate;
	m_bLogLevel = logLevel;
	m_bLogDebug = logDebug;

	time_t now = time (NULL);
	struct tm tm;
	localtime_r(&now, &tm);

	if (!m_bLogLevel)
	{
		snprintf(m_sLogFile, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT ".%s",
				m_sFileName, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, "log");

		int flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;

		m_iLogFd= open (m_sLogFile, flags, 0644);

		if(m_iLogFd< 0)
		{
			char * message;
			message = strerror(errno);
			fprintf(stderr, "ATTENTION: open logdebu file failed, file[%s], error[%s]\n", m_sLogFile, message);
			return -1;
		}

	}
	else
	{
		snprintf(m_sLogDebug, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT ".%s",
				m_sFileName, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, "debug");

		int flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;

		m_iDebugFd = open (m_sLogDebug, flags, 0644);

		if(m_iDebugFd < 0)
		{
			char * message;
			message = strerror(errno);
			fprintf(stderr, "ATTENTION: open logdebu file failed, file[%s], error[%s]\n", m_sLogDebug, message);
			return -1;
		}

		snprintf(m_sLogError, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT  ".%s",
				m_sFileName, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, "error");

		flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;

		m_iErrorFd = open (m_sLogError, flags, 0644);

		if(m_iErrorFd < 0)
		{
			char * message;
			message = strerror(errno);
			fprintf(stderr, "ATTENTION: open logerror file failed, file[%s], error[%s]\n", m_sLogError, message);
			return -1;
		}


		snprintf(m_sLogKey, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT  ".%s",
				m_sFileName, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, "key");

		flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;

		m_iKeyFd= open (m_sLogKey, flags, 0644);

		if(m_iKeyFd < 0)
		{
			char * message;
			message = strerror(errno);
			fprintf(stderr, "ATTENTION: open logkey file failed, file[%s], error[%s]\n", m_sLogKey, message);
			return -1;
		}

	}
	memset(m_sFormat , 0, sizeof(m_sFormat));
	memset(m_sIp, 0, sizeof(m_sIp));
	memset(m_sLogType, 0, sizeof(m_sLogType));
	memset((char*)&m_cLogSrvAddr, 0, sizeof(m_cLogSrvAddr));
	m_nPort = 0; 
	m_nPid = 0;
	m_nSvid = 0;
	m_nUdpFd = 0; //socket句柄
	m_nSeq = 0;

	return 0;

}


int clib_log::SetLevel( int logLevel )
{
	if ( logLevel < CLIB_LOG_LEV_ERROR ) return(-1);
	m_iLevel = logLevel;
	return(0);
}
int clib_log::SetLogDebug( bool logDebug )
{
	m_bLogDebug = logDebug;
	return(0);
}

int clib_log::GetLevel()
{
	return m_iLevel;
}

//record log msg
void clib_log::LogMsgInfo(char const * sFormat,...)
{
	struct tm tm_current;
	struct timeval stLogTv;
	time_t now = time (NULL);

	localtime_r(&now, &tm_current);

	gettimeofday(&stLogTv, NULL);
	snprintf(m_sFormat, sizeof(m_sFormat) - 1, BY_LOG_FORMAT_PRE, 
			BY_LOG_DEBUG_MARK, tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday, 
			tm_current.tm_hour, tm_current.tm_min, tm_current.tm_sec, stLogTv.tv_usec, sFormat);

	va_list ap;
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogFile, m_iLogFd, "log");
	va_end(ap);

	//fprintf(stderr, "write udp m_nRemote=%d\n", m_nRemote);
	if (m_nRemote == REMOTE_LOG){
		udpwrite(m_sFormat, 0, 0);  // tid uid 目前都为0
	}
}
//record debug log msg
void clib_log::LogDebugMsg(char const * sFormat,...)
{
	if (!m_bLogDebug)
	{
		return;
	}
	struct tm tm_current;
	struct timeval stLogTv;
	time_t now = time (NULL);

	localtime_r(&now, &tm_current);

	gettimeofday(&stLogTv, NULL);
	snprintf(m_sFormat, sizeof(m_sFormat) - 1, BY_LOG_FORMAT_PRE, 
			BY_LOG_DEBUG_MARK, tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday, 
			tm_current.tm_hour, tm_current.tm_min, tm_current.tm_sec, stLogTv.tv_usec, sFormat);

	va_list ap;
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogDebug, m_iDebugFd, "debug");
	va_end(ap);
}
//record debug log msg
void clib_log::LogErrMsg(char const * sFormat,...)
{
	struct tm tm_current;
	struct timeval stLogTv;
	time_t now = time (NULL);

	localtime_r(&now, &tm_current);
	gettimeofday(&stLogTv, NULL);
	snprintf(m_sFormat, sizeof(m_sFormat) - 1, BY_LOG_FORMAT_PRE, 
			BY_LOG_ERROR_MARK, tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday, 
			tm_current.tm_hour, tm_current.tm_min, tm_current.tm_sec, stLogTv.tv_usec, sFormat);


	va_list ap;
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogError, m_iErrorFd, "error");
	va_end(ap);

	if (!m_bLogDebug)
	{
		return;
	}
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogDebug, m_iDebugFd, "debug");
	va_end(ap);

}

void clib_log::LogKeyMsg(char const * sFormat,...)
{
	struct tm tm_current;
	struct timeval stLogTv;
	time_t now = time (NULL);

	localtime_r(&now, &tm_current);
	gettimeofday(&stLogTv, NULL);

	snprintf(m_sFormat, sizeof(m_sFormat) - 1, BY_LOG_FORMAT_PRE, 
			BY_LOG_KEY_MARK, tm_current.tm_year + 1900, tm_current.tm_mon + 1, tm_current.tm_mday, 
			tm_current.tm_hour, tm_current.tm_min, tm_current.tm_sec, stLogTv.tv_usec, sFormat);

	va_list ap;
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogKey, m_iKeyFd, "key");
	va_end(ap);

	if (!m_bLogDebug)
	{
		return;
	}
	va_start(ap, sFormat);
	WriteLogGeneral(m_sFormat, ap, m_sLogDebug, m_iDebugFd, "debug");
	va_end(ap);
}

void clib_log::WriteLogGeneral(char const * format, va_list ap, char (&logFileName)[MAX_PATH_LEN], int & logFd, char const* logType)
{
	int savedErrNo = errno;
	int n_write;
	char buf[LOGSIZE];

	if(!m_sFileName[0])
	{
		fprintf(stderr, "ATTENTION: the prefix of the log file is not set\n");
		return;
	}

	if (m_iLogRotateType == CLIB_LOG_TYPE_DAY)
	{
		RotateFile(logFileName, logFd, logType);
	}

	// restore errno
	errno = savedErrNo;
	n_write = vsnprintf(buf, LOGSIZE - 2, format, ap);
	if(buf[n_write - 1] != '\n')
		buf[n_write++] = '\n';

	if(__log_stdout__)
	{
		write(1, buf, n_write);
	}
	else
	{
		if(write(logFd, buf, n_write) < 0)
		{
			fprintf(stderr, "write error\n");
		}
	}
}
void clib_log::RotateFile(char (&logFileName)[MAX_PATH_LEN], int& logFd, char const* logType)
{
	bool btruncate = false;
	struct stat stBuf;
	struct tm tm, tm_last_week;
	time_t now = time (NULL);
	time_t last_week = now - 21 * 24 * 3600;
	localtime_r(&now, &tm);
	localtime_r(&last_week, &tm_last_week);
	bool rotate = false;

	do
	{
		char logfile[MAX_PATH_LEN] = {0};
		rotate = false;

		snprintf(logfile, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT ".%s",
				m_sFileName, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, logType);

		// 文件名不同 或者文件被关掉(日志滚动的时候)
		if (strcmp(logFileName, logfile) || logFd == INVALID_FILE_FD)
		{	
			//文件名不同, 时间分割日志
			if (logFd != INVALID_FILE_FD)
			{
				close(logFd);
				logFd = INVALID_FILE_FD;
				btruncate = true;
				rotate = true;
			}

			// -1 时重新打开
			strncpy (logFileName, logfile, sizeof (logFileName) - 1);

			int flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;
			if(btruncate)
				flags |= O_TRUNC;

			logFd = open (logFileName, flags, 0644);

			if(logFd < 0)
			{
				fprintf(stderr, "RotateFile ATTENTION: open log file failed,  file[%s], error\n", logFileName);
				return;
			}
		}

		if (-1 == fstat(logFd, &stBuf))
		{
			fprintf(stderr, "RotateFile ATTENTION: stat log file failed, file[%s], error\n", logFileName);
			return;
		}

		if ((int)stBuf.st_size >= m_iMaxFileSize)
		{
			//大小分割文件
			close(logFd);
			logFd = INVALID_FILE_FD;
			btruncate = true;

			if(m_iMaxFileNum > 0 && m_iMaxFileSize> 0) 
			{
				char    s_oldfile[MAX_PATH_LEN] = {0};
				char    s_newfile[MAX_PATH_LEN] = {0};

				for(int i = m_iMaxFileNum - 1; i > 0; i--) {
					if(i == 1) {
						snprintf(s_newfile, sizeof(s_newfile), "%s.%d", logFileName, i);
						rename(logFileName , s_newfile );
					} else {
						snprintf(s_oldfile, sizeof(s_oldfile), "%s.%d", logFileName, i - 1);
						snprintf(s_newfile, sizeof(s_newfile), "%s.%d", logFileName, i);
						rename(s_oldfile, s_newfile);
					}
				}
			}

			char strLogTmp[MAX_PATH_LEN];
			snprintf(strLogTmp, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT ".%s",
					m_sFileName, tm_last_week.tm_year + 1900, tm_last_week.tm_mon + 1, tm_last_week.tm_mday, logType);

			if(m_iMaxFileSize > 0 && m_iMaxFileNum > 0)
			{
				char    s_oldfile[MAX_PATH_LEN] = {0};

				for(int i = m_iMaxFileNum; i >= 0; i--) {
					if( i == 0 ) {
						if(remove(strLogTmp) == 0) break;
					} else {
						snprintf(s_oldfile, sizeof(s_oldfile), "%s.%d", strLogTmp, i);
						if(remove(s_oldfile) == 0) break;
					}
				}
			}

			continue;
		}
		else
		{
			if (rotate)
			{
				char strLogTmp[MAX_PATH_LEN];
				snprintf(strLogTmp, MAX_PATH_LEN - 1, "%s" FILE_TIME_FORMAT ".%s",
						m_sFileName, tm_last_week.tm_year + 1900, tm_last_week.tm_mon + 1, tm_last_week.tm_mday, logType);

				if(m_iMaxFileSize > 0 && m_iMaxFileNum > 0)
				{
					char    s_oldfile[MAX_PATH_LEN] = {0};

					for(int i = m_iMaxFileNum; i >= 0; i--) {
						if( i == 0 ) {
							if(remove(strLogTmp) == 0) break;
						} else {
							snprintf(s_oldfile, sizeof(s_oldfile), "%s.%d", strLogTmp, i);
							if(remove(s_oldfile) == 0) break;
						}
					}
				}
			}

			break;
		}

	}while(true);

}
/* {{{ clib_clog::~clib_log() */
/**
 * Destory clib_log class.
 */
clib_log::~clib_log( void )
{
}
int clib_log::udpwrite(const char *strMsg, int tid, int uid)
{
	if (m_nRemote != REMOTE_LOG) {
		return -1;
	}
	if (NULL == strMsg || strlen(strMsg) <= 0) {
		return(-1);
	} 

	if (m_nUdpFd <= 0){
		m_nUdpFd = ::socket(PF_INET, SOCK_DGRAM, 0);
		int val = fcntl(m_nUdpFd, F_GETFL, 0);
		if (val == -1) {
			return errno ? -errno : val;
		}
		int ret = 0;
		if (! (val & O_NONBLOCK)) {
			ret = fcntl(m_nUdpFd, F_SETFL, val | O_NONBLOCK | O_NDELAY);
		}
		if(ret < 0) {return errno ? -errno : ret;}
	}
	char s_msg[MAX_PACKET_LEN] = {0};
	struct timeval tv;
	if (gettimeofday(&tv,NULL) != 0) { 
		//todo: log
		return -1;
	}

	++m_nSeq;
	if(m_nSeq >= 65535) { m_nSeq = 0;}
	size_t n = snprintf(s_msg, MAX_PACKET_LEN, UDP_LOG_FORMAT,
			tv.tv_sec, tv.tv_usec,
			m_nPid,
			m_nSvid,
			m_sLogType,
			m_nSeq,
			m_iLevel,
			tid,
			uid,
			m_sIp,
			m_nPort,
			strMsg);
	if (n >= MAX_PACKET_LEN) {
		//WARNING
	}

	int bytes = ::sendto(m_nUdpFd, s_msg, strlen(s_msg), 0,
			(struct sockaddr *)(&m_cLogSrvAddr),
			sizeof(struct sockaddr_in));
	if (bytes < 0){
		//printf("sendto faild. strMsg[%s],bytes[%d].\a\n",strMsg,bytes);
		::close(m_nUdpFd); 
		m_nUdpFd = -1;
		return -1;
	}
	return 0;
}
int clib_log::sremote(const int nPort, 
		const int nlocalPort,
		const int nSvid,
		const int nPid,
		const char* pszlocalIp,
		const char* pszlogIp
		)
{
	if (m_nUdpFd <= 0){
		m_nUdpFd = ::socket(PF_INET, SOCK_DGRAM, 0);
		int val = fcntl(m_nUdpFd, F_GETFL, 0);

		if (val == -1) {return errno ? -errno : val;}
		int ret = 0;
		if (! (val & O_NONBLOCK)) {
			ret = fcntl(m_nUdpFd, F_SETFL, val | O_NONBLOCK | O_NDELAY);
		}
		if (ret < 0) {return errno ? -errno : ret;}
	}
	bzero(&m_cLogSrvAddr, sizeof(struct sockaddr_in));
	m_cLogSrvAddr.sin_family = AF_INET;
	m_cLogSrvAddr.sin_port = htons(nPort);
	fprintf(stderr, "sendto\n");
	if (inet_aton(pszlogIp, &m_cLogSrvAddr.sin_addr)<0){
		int err_id = errno;
		fprintf(stderr, "Ip error:%s\n", strerror(err_id));
		if (m_nUdpFd > 0){ 
			::close(m_nUdpFd);
		}
		return err_id ? -err_id : -1;
	}
	m_nSvid = nSvid;
	m_nPid = nPid;
	m_nPort = nlocalPort;
	snprintf(m_sIp, MIN_LEN, "%s", pszlocalIp);

	return 0;
}
