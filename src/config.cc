/*
xstress - xk0derz SMTP Stress Tester

(c) Amit Singh, amitcz@yahoo.com
xk0der.wordpress.com

This software and related files are licensed under GNU GPL version 2
Please visit the following webpage for more details
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include "common.h"
#include "sendmail.h"
#include "config.h"
#include "logger.h"

using namespace std;

const char * ConfigVarsStr[]=
{
  " ",
  "SERVER",
  "PORT",
  "THREADS",
  "MAILS_PER_THREAD",
  "LOG_FILE",
  "TIMEOUT",
  "REPORT_AFTER",
  "LOG_TIMEOUT",
  "MAX_RECIPIENTS"
  " "
};

const char *SectionsStr[]=
{
  " ",
  "[CONFIG]",
  "[TO]",
  "[FROM]",
  "[SUBJECT]",
  "[BODY]",
  "[ATTACHMENT]",
  " "
};

int Config::okay()
{
  return iOkay;
}

string Config::getTo()
{
  unsigned int size = toList.size(), ii;
  srand(time(0));
  ii = (rand()%size);
  if(size==0) return string("");
  return toList[ii]; 
}

string Config::getFrom()
{
  unsigned int size = fromList.size(), ii;
  srand(time(0));
  ii = (rand()%size);
  if(size==0) return string("");
  return fromList[ii]; 
}

string Config::getSubject()
{
  unsigned int size = subjectList.size(), ii;
  srand(time(0));
  ii = (rand()%size);
  if(size==0) return string("");
  return subjectList[ii]; 
}

string Config::getBody()
{
  unsigned int size = bodyList.size(), ii;
  srand(time(0));
  ii = (rand()%size);
  if(size==0) return string("");
  return bodyList[ii]; 
}

string Config::getAttachment()
{
  unsigned int size = attachList.size(), ii;
  srand(time(0));
  if(size==0) return string("");
  ii = (rand()%size);
  return attachList[ii]; 
}


Config::Config(string sConfigFile)
{
  char cBuffer[1024];
  int ii;
  string sLine;
  iOkay = 1;
  ifstream filp(sConfigFile.c_str());
  unsigned int currSection = 0;
  unsigned int configVar = 0;
  unsigned int configVarsFound = 0;
  string LValue;
  string RValue;

  //
  sServerIP = "";
  uiServerPort = 0;
  uiThreads = 0;
  uiMailsPerThread = 0;
  uiReportAfter = 0;
  uiLogTimeout = 1;
  uiMaxRecipients = 1;

  if(!filp.fail())
  {
    while(!filp.eof())
    {
      filp.getline(cBuffer,1023);
      sLine = cBuffer;

      // Comments
      if(sLine[0] == '#') continue;
      if(sLine.empty()) continue;

      // Check if it's a section
      if(sLine[0] =='[')
      {
        for(ii=1;ii<S_MAX;ii++)
        {
          if(sLine == SectionsStr[ii])
          {
            currSection = ii;
            break;
          }
        }
        continue;
      }

      switch(currSection)
      {
        case S_CONFIG:
          ii = sLine.find("=");
          LValue = sLine.substr(0,ii);
          RValue = sLine.substr(ii+1,sLine.length());

          configVar = C_NO_VAR;
          for(ii=1;ii<C_MAX;ii++)
          {
            if(LValue == ConfigVarsStr[ii])
            {
              configVar = ii;
              break;
            }
          }

          switch(configVar)
          {
            case C_SERVER:
              sServerIP = RValue;
              break;
            case C_PORT:
              uiServerPort = atoi(RValue.c_str());
              break;
            case C_THREADS:
              uiThreads = atoi(RValue.c_str());
              break;
            case C_MAILS_PER_THREAD:
              uiMailsPerThread = atoi(RValue.c_str());
              break;
            case C_TIMEOUT:
              uiTimeout = atoi(RValue.c_str());
              break;
            case C_LOG_FILE:
              sLogFile = RValue;
              break;
            case C_REPORT_AFTER:
              uiReportAfter = atoi(RValue.c_str());
              break;
            case C_LOG_TIMEOUT:
              if((RValue == "false") || (RValue=="0"))
                uiLogTimeout = 0;
              else if((RValue == "true") || (RValue=="1"))
                uiLogTimeout = 1;
              else 
              { 
                logger.log("Invalid value for LOG_TIMEOUT, use 'false' or 'true'.");
                uiLogTimeout = 1;
              }
              break;
            case C_MAX_RECIPIENTS:
              uiMaxRecipients = atoi(RValue.c_str());
              break;
          }

          break;
        case S_TO:
            //cout << "Add to TO list " << sLine << endl;
            toList.push_back(sLine);
          break;
        case S_FROM:
            fromList.push_back(sLine);
          break;
        case S_SUBJECT:
            subjectList.push_back(sLine);
          break;
        case S_BODY:
            addToList(sLine, bodyList);
          break;
        case S_ATTACH:
            addToList(sLine, attachList);
            //attachList.push_back(sLine);
          break;
      }

    }

    if(sServerIP.empty() || uiServerPort == 0 || uiThreads == 0)
    {
      iOkay = 0;
    }

  }
  else
  {
    iOkay = false;
  }
}

void Config::addToList(string sFilename, vector <string> &pList)
{
  struct stat filp_stat;
  string sFiletype = "";
  
  if(sFilename.find("!")!=string::npos)
  {
    string sTemp = sFilename;
    sFilename = sTemp.substr(0,sTemp.find("!"));
    sFiletype = sTemp.substr(sTemp.find("!")+1,sTemp.length());
  }

  if(stat(sFilename.c_str(),&filp_stat)==0)
  {
    if(S_ISDIR(filp_stat.st_mode))
    {
      struct dirent *d_ent;
      DIR *dirp = opendir(sFilename.c_str());
      if(dirp)
      {
        string sDir = sFilename;
        while(d_ent = readdir(dirp))
        {
          sFilename = sDir + "/" +d_ent->d_name;
          if(stat(sFilename.c_str(),&filp_stat)==0)
          {
            if(S_ISREG(filp_stat.st_mode))
            {
              if(!sFiletype.empty())
              {
                sFilename.append("!");
                sFilename.append(sFiletype);
              }
              pList.push_back(sFilename);
            }
          }
        }
      }
      else
      {
        char cBuf[255];
        snprintf(cBuf,254,"Unable to open Directory: %s", sFilename.c_str());
        cBuf[254]=0x00;
        logger.log(cBuf);
      }
    }
    else if(S_ISREG(filp_stat.st_mode))
    {
      if(!sFiletype.empty())
      {
        sFilename.append("!");
        sFilename.append(sFiletype);
      }
      pList.push_back(sFilename);
    }
    else
    {
      char cBuf[255];
      snprintf(cBuf,254,"Not a regular File or Directory: %s", sFilename.c_str());
      cBuf[254]=0x00;
      logger.log(cBuf);
    }
  }
  else
  {
    char cBuf[255];
    snprintf(cBuf,254,"Unable to get stats for file: %s", sFilename.c_str());
    cBuf[254]=0x00;
    logger.log(cBuf);
  }
}
