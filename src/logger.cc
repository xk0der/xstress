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
#include <fstream>
#include "common.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;


Logger::Logger(string _LogFile,int _cacheThreshold)
{
  threadId = -1;
  sCache = "";
  sLogFile = _LogFile;
  iThreshold = _cacheThreshold;
}

void Logger::setThreadId(int _id)
{
  threadId = _id;
}

int Logger::getThreadId()
{
  return threadId;
}

void Logger::resetThreshold(int _cacheThreshold)
{
  iThreshold = _cacheThreshold;
}

void Logger::log(string _msg)
{
  char cBuffer[255];
  struct tm *currTime;
  time_t currTS;

  time(&currTS);
  currTime = localtime(&currTS);

  sprintf(cBuffer, "%i/%i/%i %02i:%02i:%02i - Thread(%i) ",
      1900+currTime->tm_year,
      currTime->tm_mon+1,
      currTime->tm_mday,
      currTime->tm_hour, currTime->tm_min, currTime->tm_sec,
      threadId);

  sCache.append(cBuffer);
  sCache.append(_msg);
  sCache.append("\n");
  if(sCache.size()>iThreshold)
  {
    flush();
  }
}

void Logger::flush()
{
    int fd=-1;
    
    do
    {
      fd = open(".xstress.log.lock",O_WRONLY | O_CREAT | O_EXCL);
      if(fd<0 && errno==EEXIST) continue;
      else if(fd<0)
      { 
        remove(".xstress.log.lock");
        cout << "ERROR: Cannot take log lock" << endl;
        return;
      }
    }while(0);

    
    fstream filp(sLogFile.c_str(), ios::app | ios::out);

    if(!filp.fail())
    {
      filp.write(sCache.c_str(), sCache.size());
      sCache = "";
      filp.close();
    }
    else
    {
      cout << "ERROR: Unable to write to log file `" << sLogFile << "'" << endl;
    }

    if(fd>=0) close(fd);  
    remove(".xstress.log.lock");
}

Logger::~Logger()
{
  flush();
}
  
void Logger::setLogFile(string sFileName)
{
  sLogFile = sFileName;
}
