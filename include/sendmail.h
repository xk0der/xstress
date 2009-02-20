
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

#ifndef __SENDMAIL_H__

#define __SENDMAIL_H__

#include "common.h"

using namespace std;

/*SendMail Class*/
class SendMail
{
  public:
  int iSock;

  private:
  unsigned int uiLastState;
  unsigned int uiStateCounter;
  unsigned int uiServerPort;
  string sServerIP;
  int iState;
  int iAuthState;
  int bAuthDone;
  int bAuthenticated;
  int iConnected;

  string sTo;
  string sFrom;
  string sSubject;
  string sBody; /*File Name*/
  string sAttachment; /*File name*/
  string sAttachType;

  string sUsername;
  string sPassword;
  string sAuthType;

  string sRecvBuf;
  string sSendBuf;
  int iSentBytes;
  unsigned int uiAuthType;
  

  public:
  SendMail();
  SendMail(string _sTo, 
      string _sFrom, string _sSubject, string _sBody, string  _sAttachment);
  void setMailInfo
      (string _sTo, string _sFrom, string _sSubject, string _sBody, string _sAttachment);
  int changeState(int sockState);
  int run(int sockState);
  void setTo(string _sTo);
  void setFrom(string _sFrom);
  void setSubject(string _sSubject);
  void setBody(string _sBody);
  void setAttachment(string _sAttachment);
  
  int setServer(string _ip, unsigned int _port);
  void reset();
  void disconnect();

  void setAuthInfo(string _username, string _password, string _authType);

  int state();
  void resetAuth();
};

#endif
