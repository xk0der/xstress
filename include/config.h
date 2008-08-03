
/*
 * xstress - xk0derz SMTP Stress Tester
 *
 * (c) Amit Singh, amitcz@yahoo.com
 * xk0der.wordpress.com
 *
 * This software and related files are licensed under GNU GPL version 2
 * Please visit the following webpage for more details
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <vector>
using namespace std;

/*Config class*/
class Config
{
  public:
  unsigned int uiServerPort;
  string sServerIP;
  string sLogFile;
  unsigned int uiThreads;
  unsigned int uiMailsPerThread;
  unsigned int uiTimeout;
  unsigned int uiReportAfter;
  unsigned int uiLogTimeout;

  private:
  vector <string> toList; 
  vector <string> fromList; 
  vector <string> subjectList; 
  vector <string> bodyList;
  vector <string> attachList;

  void addToList(string sFilename, vector <string> &pList);

  int iOkay;
  
  public:
  Config(string sConfigFile);
  int okay();
  string getTo();
  string getFrom();
  string getSubject();
  string getBody();
  string getAttachment();
};

#endif

