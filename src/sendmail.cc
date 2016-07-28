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
#include "common.h"
#include "logger.h"
#include "sendmail.h"
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace std;

const char * const MIMETypeStr[]=
{
    " ",
    "plain",
    "html",
    "bin",
    "image",
    " "
};

const char * const AuthType[] = {
    "NULL", // NONE
    "PLAIN",
    "LOGIN",
    "CRAM-MD5",
    NULL
};

/*SendMail Methods*/
SendMail::SendMail()
{
    uiLastState = IDLE;
    uiStateCounter = 0;
    iState = IDLE;
    iAuthState = 0;
    bAuthDone = false;
    iSock = -1;
    sFrom = sTo = "";
    sSubject = "Test Mail!";
    sAttachment = "";
    iConnected = false;
    sPassword = "";
    sUsername = "";
    uiAuthType = AUTH_NONE;
    bAuthenticated = false;
}

    SendMail::SendMail
(string _sTo, string _sFrom, string _sSubject, string _sBody, string _sAttachment)
{
    uiLastState = IDLE;
    uiStateCounter = 0;
    iConnected = false;
    iState = IDLE;
    iAuthState = 0;
    bAuthDone = false;
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
    int rv = NO_ERR;
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
        debug("S: " + sRecvBuf);
    }
    else if(sockState==WRITE_READY)
    {
        //cout << "WRITE_READY" << endl;
        uiLastState = iState;
        switch(iState)
        {
            case IDLE:
                iState = CONNECT;
                break;
            
            case CONNECT:
                iState = EHLO;
                break;
            
            case EHLO:
                if(uiAuthType == AUTH_NONE) 
                {
                    bAuthenticated = true;
                }

                if(bAuthenticated)
                {
                    iState = MAILFROM;
                }
                else
                {
                    iState = AUTH;
                    iAuthState = 0;
                }
                break;
            
            case AUTH:
                if(bAuthDone)
                {
                    bAuthenticated = true;
                    iState = MAILFROM;
                }
                else
                {
                    iAuthState++;
                }
                break;
            
            case MAILFROM:
                iState = RCPTTO;
                break;
            
            case RCPTTO:
                iState = DATA;
                break;
            
            case DATA:
                iState = MAIL;
                break;
            
            case MAIL:
                if(sSendBuf.empty())
                {
                    iSentBytes = 0;
                    iState = FINISHED;
                }
                else if(iSentBytes>=sSendBuf.length())
                {
                    iState = FINISHED;
                }
                else if(iSentBytes<sSendBuf.length())
                {
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
    }
    return NO_ERR;
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
                debug("Trying to connect to " + sServerIP);
                //cout << "Server : " << sServerIP <<  ":" << uiServerPort << endl;
                if(iSock!=-1)
                {

                    if(connect(iSock, (const sockaddr*) &addr, sizeof(sockaddr))==-1 && 
                            ( (errno == EINPROGRESS)|| (errno == EALREADY)))
                    {
                        iConnected = true;
                        debug("Connected to the server!");
                    }
                    else
                    {
                        return ERR_CONNECT;
                        //cout << "ERR" << endl;
                        debug("Error connecting!");
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
                string sBuf = "EHLO " + sFQDNHelo;
                sBuf.append("\r\n");
                // cout << "Send Buffer = " << sBuf << "HELO content = " << sFQDNHelo << endl;
                debug("C: EHLO " + sFQDNHelo);
                send(iSock, sBuf.c_str(), sBuf.length(), 0);
            }
            break;          
        case AUTH:
            {
                if(uiAuthType != AUTH_NONE)
                {
                    bAuthDone = false;
                    switch(uiAuthType)
                    {
                        case AUTH_PLAIN:
                            {
                                string sAuth64;
                                char cBuf[255];
                                string sBuf = "AUTH ";
                                //switch(uiAuthType)
                                sBuf.append(AuthType[uiAuthType]);
                                sBuf.append(" ");

                                string sFilenameTMP;
                                string sFilename;
                                sprintf(cBuf, ".auth.b64.%i.tmp", logger.getThreadId());
                                sFilenameTMP = cBuf;
                                sprintf(cBuf, ".auth.b64.%i", logger.getThreadId());
                                sFilename = cBuf;

                                cBuf[0] = 0x00;
                                ofstream ofilp;
                                ofilp.open(sFilenameTMP.c_str(), ofstream::out | ofstream::binary);
                                if(!ofilp.fail())
                                {
                                    ofilp.write(sUsername.c_str(), sUsername.length());
                                    ofilp.write(cBuf, 1);
                                    ofilp.write(sUsername.c_str(), sUsername.length());
                                    ofilp.write(cBuf, 1);
                                    ofilp.write(sPassword.c_str(), sPassword.length());
                                    ofilp.close();
                                }

                                sprintf(cBuf, "./base64 -e %s > %s", sFilenameTMP.c_str(), sFilename.c_str());
                                system(cBuf);

                                ifstream filp;
                                filp.open(sFilename.c_str(), ifstream::in);

                                char cBuffer[1024];
                                if(!filp.fail())
                                {
                                    if(!filp.eof())
                                    {
                                        filp.getline(cBuffer, 1000);
                                        cBuffer[1023]=0x00; 
                                        sAuth64 = cBuffer;
                                    }

                                    sBuf.append(sAuth64);
                                    sBuf.append("\r\n");

                                    debug("C: " + sBuf);

                                    send(iSock, sBuf.c_str(), sBuf.length(), 0);

                                }

                                system("rm -f .auth.*");
                                bAuthDone = true;
                            }
                            break; // AUTH_PLAIN
                        case AUTH_LOGIN:
                            {
                                string sBuf;
                                char cBuf[1024];
                                string sFilename;
                                string sFilenameTMP;

                                sprintf(cBuf, ".auth.b64.%i", logger.getThreadId());
                                sFilename = cBuf;

                                sprintf(cBuf, ".auth.b64.%i.tmp", logger.getThreadId());
                                sFilenameTMP = cBuf;

                                ifstream filp;
                                ofstream ofilp;

                                switch(iAuthState)
                                {
                                    case 0: // AUTH LOGIN
                                        sBuf = "AUTH ";
                                        sBuf.append(AuthType[uiAuthType]);
                                        break;
                                    case 1: // Username
                                        ofilp.open(sFilenameTMP.c_str(), ofstream::out | ofstream::binary);

                                        if(!ofilp.fail())
                                        {
                                            ofilp.write(sUsername.c_str(), sUsername.length());
                                        } 
                                        ofilp.close();

                                        sprintf(cBuf,"./base64 -e %s > %s", sFilenameTMP.c_str(), sFilename.c_str());
                                        system(cBuf);

                                        filp.open(sFilename.c_str(), ifstream::in);
                                        if(!filp.fail())
                                        {
                                            filp.getline(cBuf, 1000);
                                            cBuf[1023] = 0x00;
                                            sBuf = cBuf;
                                            sBuf.append("\r\n");
                                        }
                                        filp.close();

                                        //system("rm -f .auth.*");
                                        break;
                                    case 2: // Password
                                        ofilp.open(sFilenameTMP.c_str(), ofstream::out | ofstream::binary);

                                        if(!ofilp.fail())
                                        {
                                            ofilp.write(sPassword.c_str(), sPassword.length());
                                        } 
                                        ofilp.close();

                                        sprintf(cBuf,"./base64 -e %s > %s", sFilenameTMP.c_str(), sFilename.c_str());
                                        system(cBuf);

                                        filp.open(sFilename.c_str(), ifstream::in);
                                        if(!filp.fail())
                                        {
                                            filp.getline(cBuf, 1000);
                                            cBuf[1023] = 0x00;
                                            sBuf = cBuf;
                                            sBuf.append("\r\n");
                                        }
                                        filp.close();
                                        //system("rm -f .auth.*");
                                        bAuthDone = true;
                                        break;
                                }
                                debug("C: " + sBuf);
                                send(iSock, sBuf.c_str(), sBuf.length(), 0);
                            }
                            break; // AUTH_LOGIN
                    } // switch(uiAuthType)
                }
                else
                {
                    bAuthDone = true;
                }
            }
            break;
        case MAILFROM:
            {
                string sBuf = "MAIL FROM:";
                sBuf.append(sFrom);
                debug("C: " + sBuf);
                sBuf.append("\r\n");
                send(iSock, sBuf.c_str(), sBuf.length(), 0);
            }
            break;          
        case RCPTTO:
            {
                string sBuf = "RCPT TO:";
                sBuf.append(sTo);
                debug("C: " + sBuf);
                sBuf.append("\r\n");
                send(iSock,sBuf.c_str(), sBuf.length(), 0);
            }
            break;          
        case DATA:
            {
                debug("C: DATA");
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
                    const char *pcWDay[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
                    const char *pcMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
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



                if(!sAttachment.empty()) // If we have an attachment
                {
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


                }

                sSendBuf.append("\r\n--");
                sSendBuf.append(BOUNDARY);
                sSendBuf.append("--\r\n");
                sSendBuf.append(".\r\n");
                iSentBytes = 0;
            }
            break;
        case MAIL:
            debug("Data sending in progress...");
            if(sSendBuf.length()>0)
            {
                iSentBytes = send(iSock, sSendBuf.c_str(), sSendBuf.length(), 0);
                debug("C: " + sSendBuf.substr(0, iSentBytes));
            }
            else iSentBytes = 1;
            break;
        case FINISHED:
            //cout << "This session has exhausted!" << endl;
            break;
    }

    return NO_ERR;
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
    return NO_ERR;
}

int SendMail::setHelo(string _helo)
{
    sFQDNHelo = _helo;
    return NO_ERR;
}

void SendMail::setAuthInfo(string _username, string _password, string _authType)
{
    sUsername = _username;
    sPassword = _password;

    unsigned int ii = 0;
    while(AuthType[ii])
    {
        if(_authType == AuthType[ii])
        {
            uiAuthType = ii;
            break;
        }
        ii++;
    }


    if(sUsername.empty() || sPassword.empty())
    {
        uiAuthType = AUTH_NONE;
    }
    else if(!sUsername.empty() && !sPassword.empty())
    {
        // We default to AUTH PLAIN if none is provided.
        if(uiAuthType == AUTH_NONE)
        {
            uiAuthType = AUTH_PLAIN;
        }
    }
}

void SendMail::resetAuth()
{
    bAuthenticated = false;
}
