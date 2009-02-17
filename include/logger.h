
/*
 * xstress - xk0derz SMTP Stress Tester
 *
 * (c) Amit Singh amit@xkoder.com
 * http://xkoder.com
 *
 * This software and related files are licensed under GNU GPL version 2
 * Please visit the following webpage for more details
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "common.h"

using namespace std;

class Logger
{
  string sLogFile;
  string sCache;
  int iThreshold;
  int threadId;

  public:
  Logger(string _LogFile=DEFAULT_LOG_FILE,int _cacheThreshold=DEFAULT_CACHE_THRESHOLD);
  void log(string _msg);
  void resetThreshold(int _cacheThreshold);
  void flush();
  void setLogFile(string sFileName);
  void setThreadId(int _id);
  int  getThreadId();
  ~Logger();
};

extern Logger logger;

#endif
