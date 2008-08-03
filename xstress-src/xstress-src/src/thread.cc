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
#include "thread.h"
#include "logger.h"

Thread::Thread(void)
{
  ulLastTS = 0;
  uiTimeoutCounter = 0;
  id = -1;
  iOkay = true;
  uiMailsToSend = 1;
  uiReportCnt = 0;
  
  uiMailsSend = 0;
  uiNotSent = 0;

  configObj = NULL;
  iSockFd = -1;
  uiEventToPoll = POLLOUT;
  logger.setThreadId(id);
}

Thread::Thread(int _id, int _mailsToSend, Config *_configObj)
{
  initThread(_id, _mailsToSend, _configObj);
}

int Thread::getid()
{
  return id;
}

void Thread::initThread(int _id, int _mailsToSend, Config *_configObj)
{
  uiReportCnt = 0;

  ulLastTS = 0;
  uiTimeoutCounter = 0;
  id = _id;
  logger.setThreadId(id);

  uiEventToPoll = POLLOUT;
  iOkay = true;
  uiMailsToSend = _mailsToSend; 
  
  uiMailsSend = 0;
  uiNotSent = 0;
  
  configObj = _configObj;
  iSockFd = -1;

  createSocket();
  mailObj.setServer(configObj->sServerIP, configObj->uiServerPort);
  mailObj.iSock = iSockFd;
  initMailObj();
}

void Thread::createSocket()
{
  if(iSockFd>=0) { iOkay = false; return; }
  
  iSockFd = socket(AF_INET, SOCK_STREAM, 6);

  if(iSockFd<0){ iOkay = false;  return; }
  
  fcntl(iSockFd, F_SETFL, O_NONBLOCK);
}

void Thread::closeSocket()
{
  if(iSockFd>=0)
  { 
    mailObj.disconnect();
    close(iSockFd);
    iSockFd = -1;
  }
  
}

void Thread::initMailObj()
{
  iOkay = true;
  mailObj.iSock = iSockFd;
  mailObj.reset();
  mailObj.setMailInfo(configObj->getTo(),
                      configObj->getFrom(),
                      configObj->getSubject(),
                      configObj->getBody(),
                      configObj->getAttachment());
}

int Thread::okay()
{
  return iOkay;
}

void Thread::process()
{
  int rv;
  pollfd fds[1];
  fds[0].fd = iSockFd;
  fds[0].events = uiEventToPoll;


  //cout << "events = " << uiEventToPoll << " POLLIN,POLLOUT,POLLERR,POLLHUP " << POLLIN <<"," << POLLOUT << "," << POLLERR << "," << POLLHUP << " CurrState=" << mailObj.state() <<endl;

  rv = poll(fds, 1, 1000);
  
  //cout << "revents = " << fds[0].revents << " POLLIN,POLLOUT " << POLLIN <<"," << POLLOUT <<endl;

  if(rv==1 && (fds[0].revents == POLLHUP || fds[0].revents == POLLNVAL))
  {
    ulLastTS = time(NULL);
    logger.log("Received POLLHUP or POLLNVAL, restarting thread.");
    
    uiEventToPoll = POLLOUT;
    closeSocket();
    createSocket();
    initMailObj();

    return;
  }


  if(rv==1 && ((fds[0].revents & POLLIN)  || (fds[0].revents & POLLOUT)))
  {
    ulLastTS = time(NULL);
    if((fds[0].revents & POLLIN)==POLLIN)
    {
      uiEventToPoll = POLLOUT;
      if(mailObj.changeState(READ_READY)==ERR_IO)
      {
        logger.log("Mail server didn't understand command OR invalid TO/FROM address.");
        closeSocket();
        createSocket();
        initMailObj();
      }

      //return;
    }

    if((fds[0].revents & POLLOUT)==POLLOUT || mailObj.state()==MAIL)
    {

      mailObj.changeState(WRITE_READY);
      
      if(mailObj.state()==MAIL) uiEventToPoll = POLLOUT;
      else uiEventToPoll = POLLIN;

      if(mailObj.state()!=FINISHED)
      {
        if((rv = mailObj.run(0))!=0)
        {
          if(rv == ERR_CONNECT)
            logger.log("Error connecting to mail server");
          if(rv == ERR_INVALID_SOCKET)
            logger.log("Error Invalid Socket");
        }
        
        if(mailObj.state()==FINISHED)
        {
          mailObj.changeState(READ_READY);
        }
      }
      
    }

    if(mailObj.state()==FINISHED)
    {
      uiMailsSend++;
      uiReportCnt++;

      if(uiMailsSend>=0xFFFFFFFF)
      {
        char cBuf[200];
        sprintf(cBuf,"Sent %ld mails, reseting counter to Zero.",0xFFFFFFFF);
        logger.log(cBuf);
        uiMailsSend=0;
      }
      initMailObj();
      uiEventToPoll = POLLOUT; 
    }
  }
 
  //cout << "Elapsed Time: " << (time(NULL)-ulLastTS) << endl;

  if((time(NULL)-ulLastTS)>configObj->uiTimeout)
  {
    ulLastTS = 0;
    //uiTimeoutCounter++;
    
    {
      logger.log("Timeout!");
      uiMailsSend++;
      uiNotSent++;
      uiReportCnt++;
      
      logger.log("Timeout!");

      if(uiMailsSend>=0xFFFFFFFF)
      {
        char cBuf[200];
        sprintf(cBuf,"Sent %ld mails, reseting counter to Zero.",0xFFFFFFFF);
        logger.log(cBuf);
        uiMailsSend = 0;
      }
      if(uiMailsSend>=0xFFFFFFFF)
      {
        char cBuf[200];
        sprintf(cBuf,"Failed sending %ld mails, reseting counter to Zero.",0xFFFFFFFF);
        logger.log(cBuf);
        uiNotSent=0;
      }
      
      uiTimeoutCounter = configObj->uiTimeout;
      closeSocket();
      createSocket();
      initMailObj();
      uiEventToPoll = POLLOUT; 
    }
  }
      
  if(configObj->uiReportAfter!=0 && uiReportCnt>=configObj->uiReportAfter)
  {
    uiReportCnt=0;
    reportStatus();
  }
}

void Thread::stop(int iSig)
{
  uiMailsToSend=1;
  uiMailsSend=1;
}

int Thread::finished()
{
  if(uiMailsToSend==0) return false;

  if(uiMailsSend == uiMailsToSend) return true;
  else return false;
}

void Thread::writeStatus()
{
  char cBuffer[1024];
  sprintf(cBuffer, "Total Mails to send: %ld, Mails Sent: %ld, Mails failed: %ld", uiMailsToSend,
          uiMailsSend-uiNotSent, uiNotSent);
  string sBuf = cBuffer;
  logger.log(sBuf);
}

void Thread::reportStatus()
{
  char cBuffer[1024];
  sprintf(cBuffer, "T: %ld,  S: %ld, F: %ld", uiMailsToSend,
          uiMailsSend-uiNotSent, uiNotSent);
  cout << "Thread: "<< logger.getThreadId() << ":: "<< cBuffer << endl;
}

Thread::~Thread()
{
  writeStatus();
  closeSocket();
}
