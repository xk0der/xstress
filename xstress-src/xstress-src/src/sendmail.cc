/*
 * xstress - amItz SMTP Stress Tester
 *
 * (c) Amit Singh, amits@intoto.com, amitcz@yahoo.com
 * xk0der.wordpress.com
 *
 * This software and related files are licensed under GNU GPL version 2
 * Please visit the following webpage for more details
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */
#include "common.h"
#include "logger.h"
#include "sendmail.h"
#include <fstream>

using namespace std;

char *MIMETypeStr[]=
{
  " ",
  "plain",
  "html"
  "bin",
  "image",
  " "
};

/*SendMail Methods*/
SendMail::SendMail()
{
  uiLastState = IDLE;
  uiStateCounter = 0;
  iState = IDLE;
  iSock = -1;
  sFrom = sTo = "";
  sSubject = "Test Mail!";
  sAttachment = "";
  iConnected = false;
}

SendMail::SendMail
      (string _sTo, string _sFrom, string _sSubject, string _sBody, string _sAttachment)
{
  uiLastState = IDLE;
  uiStateCounter = 0;
  iConnected = false;
  iState = IDLE;
  iSock = -1;
  setTo(_sTo);
  setFrom(_sFrom);
  setSubject(_sSubject);
  setBody(_sBody);
  setAttachment(_sAttachment);
}

void SendMail::reset()
{
  string sBuf = "\r\n.";
  sBuf.append("\r\n");
  char cBuf[4092];
  cBuf[0]=0x00;
  
  if(iConnected && iState == MAIL)
  {
    send(iSock, sBuf.c_str(), sBuf.length(), 0);
  }

  uiLastState = IDLE;
  uiStateCounter = 0;
  iState = IDLE;
}

void SendMail::disconnect()
{
  iConnected = false;
}

void SendMail::setMailInfo
      (string _sTo, string _sFrom, string _sSubject, string _sBody, string _sAttachment)
{
  setTo(_sTo);
  setFrom(_sFrom);
  setSubject(_sSubject);
  setBody(_sBody);
  setAttachment(_sAttachment);
}

int SendMail::changeState(int sockState)
{
  int rv;
  if(iState == FINISHED) return 0;

  //cout << "last State = " << uiLastState << " Curr State " << iState << endl; 

  if(sockState==READ_READY)
  {
    //cout << "READ_READY" << endl;
    char cBuf[4092];
    cBuf[0]=0x00;
    rv = recv(iSock, cBuf, 4000, 0);
    cBuf[rv] = 0x00;
    sRecvBuf = "";
    sRecvBuf = cBuf;
    if(sRecvBuf[0] == '4' || sRecvBuf[0] == '5' )
    {
     logger.log(sRecvBuf);
     return ERR_IO;
    }
    //cout << "RECEIVED:" << sRecvBuf << endl;
  }
  else if(sockState==WRITE_READY)
  {
    //cout << "WRITE_READY" << endl;
    uiLastState = iState;
    switch(iState)
    {
      case IDLE:
        iState = CONNECT;
        //cout << "IDLE-->CONNECT" << endl;
        break;
      case CONNECT:
        //cout << "CONNECT-->EHLO" << endl;
        iState = EHLO;
        break;
      case EHLO:
        //cout << "EHLO->MAILFROM" << endl;
        iState = MAILFROM;
        break;
      case MAILFROM:
        //cout << "MAILFROM-->RCPTTO" << endl;
        iState = RCPTTO;
          break;
      case RCPTTO:
        //cout << "RCPTTO-->DATA" << endl;
        iState = DATA;
          break;
      case DATA:
        //cout << "DATA-->MAIL" << endl;
        iState = MAIL;
          break;
      case MAIL:
        //cout << "MAIL:" << sSendBuf << endl;
        if(sSendBuf.empty())
        {
          iSentBytes = 0;
          iState = FINISHED;
        }
        else if(iSentBytes>=sSendBuf.length())
        {
          //cout << "MAIL-->FINISHED" << endl;
          iState = FINISHED;
        }
        else if(iSentBytes<sSendBuf.length())
        {
          //cout << "MAIL-->MAIL" << endl;
          string sBuf;
          sBuf = sSendBuf.substr(iSentBytes, sSendBuf.length());
          sSendBuf = sBuf;
          iSentBytes = 0;
        }
        else
        {
          sSendBuf = "";
        }
        break;
    }
    /*
   if(iState==uiLastState) uiStateCounter++;
   if(uiStateCounter>10) return ERR_IO;
*/
  }
}

int SendMail::run(int sockState)
{
  if(iSock==-1) return ERR_INVALID_SOCKET;

  //cout <<  "[run(" << iState << ")]" << endl;

  switch(iState)
  {
    case CONNECT:
      {
        if(iConnected)
        {
          //string sBuf = "NOOP";
          //sBuf.append("\r\n");
          //send(iSock, sBuf.c_str(), sBuf.length(), 0);
          break;
        }
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(uiServerPort);
        inet_aton(sServerIP.c_str(), &addr.sin_addr);
        //cout << "Server : " << sServerIP <<  ":" << uiServerPort << endl;
        if(iSock!=-1)
        {
          
          if(connect(iSock, (const sockaddr*) &addr, sizeof(sockaddr))==-1 && 
                                 ( (errno == EINPROGRESS)|| (errno == EALREADY)))
          {
            iConnected = true;
          }
          else
          {
            return ERR_CONNECT;
            //cout << "ERR" << endl;
          }
          /*
          if (connect(iSock, (const sockaddr*) &addr, sizeof(sockaddr)) == -1)
          return ERR_CONNECT;
          */

          iConnected = true;

        }
      }
      break;         
    case EHLO:
        {
          string sBuf = "EHLO localhost";
          sBuf.append("\r\n");
          send(iSock, sBuf.c_str(), sBuf.length(), 0);
        }
      break;          
    case MAILFROM:
        {
          string sBuf = "MAIL FROM:";
          sBuf.append(sFrom);
          sBuf.append("\r\n");
          send(iSock, sBuf.c_str(), sBuf.length(), 0);
        }
      break;          
    case RCPTTO:
        {
          string sBuf = "RCPT TO:";
          sBuf.append(sTo);
          sBuf.append("\r\n");
          send(iSock,sBuf.c_str(), sBuf.length(), 0);
        }
      break;          
    case DATA:
        {
          string sBuf = "DATA";
          string sLine;
          char cBuffer[1024];
          sBuf.append("\r\n");
          send(iSock,sBuf.c_str(), sBuf.length(), 0);
          ifstream filp;
          int iMType = MT_BIN, ii;
          
          sSendBuf = "MIME-Version: 1.0\r\n";
          {
            /*Create Send Buffer*/
            char cBuffer[255];
            struct tm *currTime;
            time_t currTS;
            unsigned int tzHr, tzMin;
            char tzSign;
            char *pcWDay[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
            char *pcMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

            time(&currTS);
            currTime = localtime(&currTS);

            tzHr = (((currTime->tm_gmtoff)/60)/60);
            tzMin = ((currTime->tm_gmtoff)/60)-(tzHr*60);

            tzSign = '+';
            if(tzHr>12)
            {
              tzHr = 12-tzHr;
              tzSign = '-';
            }


            sprintf(cBuffer, "%s, %i %s %i %02i:%02i:%02i %c%02i%02i",
                pcWDay[currTime->tm_wday],
                currTime->tm_mday,
                pcMonth[currTime->tm_mon],
                1900+currTime->tm_year,
                currTime->tm_hour, currTime->tm_min, currTime->tm_sec,
                tzSign, tzHr, tzMin);
            
            sSendBuf.append("Date: ");
            sSendBuf.append(cBuffer);
            sSendBuf.append("\r\n");
          }

          sSendBuf.append("To:");
          sSendBuf.append(sTo);
          sSendBuf.append("\r\n");

          sSendBuf.append("From:");
          sSendBuf.append(sFrom);
          sSendBuf.append("\r\n");

          sSendBuf.append("Subject:");
          sSendBuf.append(sSubject);
          sSendBuf.append("\r\n");

          sSendBuf.append("Content-Type: multipart/mixed;");
          sSendBuf.append(" boundary=");
          sSendBuf.append(BOUNDARY);
          sSendBuf.append("\r\n\r\n");

          sSendBuf.append("This is a multi-part message in MIME format.\r\n");
          sSendBuf.append("\r\n--");
          sSendBuf.append(BOUNDARY);
          sSendBuf.append("\r\n");
          

          sSendBuf.append("Content-Type: text/plain; charset=ISO-8859-1; format=flowed\r\n");
          sSendBuf.append("Content-Transfer-Encoding: 7bit\r\n");
  
          sSendBuf.append("\r\n");

          filp.clear();
          /*Fetch Body here*/
          filp.open(sBody.c_str(), ifstream::in);
          if(!filp.fail())
          {
            while(!filp.eof())
            {
              filp.getline(cBuffer, 1000);
              cBuffer[1023]=0x00; 
              sLine = cBuffer;
              sLine.append("\r\n");

              sSendBuf.append(sLine);
            }
            filp.close();
          }
          else
          {
            sSendBuf.append("\r\nUnable to open body file'");
            sSendBuf.append(sBody);
            sSendBuf.append("'\r\n");
          }
          sSendBuf.append("\r\n");
        


          // Attachment goes here!!
          for(ii=1;ii<MT_MAX;ii++)
          {
            if(sAttachType==MIMETypeStr[ii])
            {
              iMType = ii;
              break;
            }
          }

          if(sAttachment.find("/")!=string::npos)
          {
            sLine = sAttachment.substr(sAttachment.rfind("/"),sAttachment.length());
          }
          else
          {
            sLine = sAttachment;
          }
          
          sSendBuf.append("\r\n--");
          sSendBuf.append(BOUNDARY);
          sSendBuf.append("\r\n");
          
          switch(iMType)
          {
            case MT_BIN:
              sSendBuf.append("Content-Type: application/octet-stream; name=");
              sSendBuf.append("\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n");

              sSendBuf.append("Content-Transfer-Encoding: base64\r\n");
              sSendBuf.append("Content-Disposition: attachment; filename=\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n\r\n");

              {
                char cBuf[255];
                sprintf(cBuf,"./base64 -e %s > .xstress.b64.%i.tmp",sAttachment.c_str(), 
                                                               logger.getThreadId());
                system(cBuf);
                
                sprintf(cBuf,".xstress.b64.%i.tmp", logger.getThreadId());
                sAttachment = cBuf;
              }

              break; 
            case MT_PLAIN:
              sSendBuf.append("Content-Type: text/plain; charset=UTF-8; name=");
              sSendBuf.append("\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n");
              sSendBuf.append("Content-Transfer-Encoding: 7bit\r\n");
              sSendBuf.append("Content-Disposition: attachment; filename=\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n\r\n");
              break;
            case MT_HTML:
              sSendBuf.append("Content-Type: text/html; charset=UTF-8; name=");
              sSendBuf.append("\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n");
              sSendBuf.append("Content-Transfer-Encoding: 7bit\r\n");
              sSendBuf.append("Content-Disposition: attachment; filename=\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n\r\n");
              break;
            case MT_IMAGE:
              sSendBuf.append("Content-Type: application/octet-stream; name=");
              sSendBuf.append("\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n");
              sSendBuf.append("Content-Transfer-Encoding: base64\r\n");
              sSendBuf.append("Content-Disposition: inline; filename=\"");
              sSendBuf.append(sLine);
              sSendBuf.append("\"\r\n\r\n");
              {
                char cBuf[255];
                sprintf(cBuf,"./base64 -e %s > .xstress.b64.%i.tmp",sAttachment.c_str(), 
                                                               logger.getThreadId());
                system(cBuf);
                
                sprintf(cBuf,".xstress.b64.%i.tmp", logger.getThreadId());
                sAttachment = cBuf;
              }
              break;
          }

          filp.clear();
          if(filp.is_open()) filp.close();
          filp.open(sAttachment.c_str(), ios::in | ios::binary);
          if(!filp.fail())
          {
            unsigned long int ulTS = time(NULL);
            while(!filp.eof())
            {
              memset(cBuffer,0,80);
              filp.read(cBuffer, 77);

              cBuffer[77]=0x00;
              sSendBuf.append(cBuffer);

              if(time(NULL)-ulTS>10)
              { 
                logger.log("Timeout while reading attachment file!");
                sSendBuf.append("Timeout while reading attachment file"); break; }
            }

            filp.close();
          }
          else
          {
            sSendBuf.append("\r\nUnable to open attachment file '");
            sSendBuf.append(sAttachment);
            sSendBuf.append("'\r\n");
          }
          sSendBuf.append("\r\n");
              
          {
            char cBuf[255];
            sprintf(cBuf,"rm -f .xstress.b64.%i.tmp",logger.getThreadId());
            system(cBuf);
          }

          sSendBuf.append("\r\n--");
          sSendBuf.append(BOUNDARY);
          sSendBuf.append("--\r\n");
          sSendBuf.append(".\r\n");
          iSentBytes = 0;
        }
      break;
    case MAIL:
        if(sSendBuf.length()>0)
        {
          iSentBytes = send(iSock, sSendBuf.c_str(), sSendBuf.length(), 0);
        }
        else iSentBytes = 1;
        break;
    case FINISHED:
         //cout << "This session has exhausted!" << endl;
      break;
  }

  return 0;
}

void SendMail::setTo(string _sTo)
{
  sTo = _sTo;
}

void SendMail::setFrom(string _sFrom)
{
  sFrom = _sFrom;
}

void SendMail::setSubject(string _sSubject)
{
  sSubject = _sSubject;
}

void SendMail::setBody(string _sBody)
{
  sBody = _sBody;
}

void SendMail::setAttachment(string _sAttachment)
{
  if(_sAttachment.find("!")!=string::npos)
  {
    sAttachment = _sAttachment.substr(0,_sAttachment.find("!"));
    sAttachType = _sAttachment.substr(_sAttachment.find("!")+1,_sAttachment.length());
  }
  else
  {
    sAttachment = _sAttachment;
    sAttachType = "bin";
  }
}

int SendMail::state(void)
{
  return iState;
}
  
int SendMail::setServer(string _ip, unsigned int _port)
{
  sServerIP = _ip;
  uiServerPort = _port;
}
