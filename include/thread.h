
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

#ifndef __THREAD_H__
#define __THREAD_H__

#include "config.h"
#include "sendmail.h"

/*Thread class*/
class Thread
{
  unsigned long ulLastTS; 
  unsigned int uiTimeoutCounter;
  int id;
  int iSockFd;
  unsigned long int uiMailsToSend;
  unsigned long int uiNotSent;
  unsigned long int uiMailsSend;
  Config *configObj;
  SendMail mailObj;
  int iOkay;
  unsigned int uiEventToPoll;
  unsigned int uiReportCnt;

  void createSocket();
  void closeSocket();

  public:
  Thread(void);
  Thread(int _id, int _mailsToSend, Config *_configObj);

  void process();
  int okay();
  void initThread(int _id, int _mailsToSend, Config *_configObj);
  void initMailObj();
  int finished();
  int getid();
  void writeStatus();
  void reportStatus();
  void stop(int iSig);

  ~Thread();

};
#endif
