
/*
 * xstress - xk0derz SMTP Stress Tester
 *
 * (c) Amit Singh amitcz@yahoo.com
 * xk0der.wordpress.com
 *
 * This software and related files are licensed under GNU GPL version 2
 * Please visit the following webpage for more details
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

#ifndef __COMMON_H__

#define __COMMON_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <iostream>

/*Common*/
using namespace std;

const unsigned int MAX_THREADS=10;

const unsigned int DEFAULT_TIMEOUT=15; // seconds


const string BOUNDARY="AABBCCDDEEFFGGHHIIaabbccdd";
const string DEFAULT_CONFIG_FILE="xstress.conf";
const string DEFAULT_LOG_FILE="xstress.log";
const int DEFAULT_CACHE_THRESHOLD = 1024*4;

const string VERSION="xstress version 0.34 - xk0derz SMTP Stress Tester";
const string COPYRIGHT="(c) 2007 Amit Singh (aka xk0der), amitcz@yahoo.com\n"
           "This software and related files are licensed under GNU GPL version 2\n"
           "Please visit the following webpage for more details\n"
           "http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\n"
           "\nThis software uses the base64 utility by Bob Trower\n"
           "Please visit the following link to know the term of use of this utility\n"
           "http://base64.sourceforge.net/\n";


const string HELP_MSG = 
"Syntax:\n"
"       $ xstress [options]\n"
"\n"
"Here options are one or more of the following...\n"
"\n"
"-v, --ver, --version           Display xstress version and exit.\n"
"-h, --help                     Display this help message\n"
"-t <value>, --threads <value>  Set number of threads to spawn.\n"
"                               Range for value is 1 to 100, inclusive.\n"
"-m <value>, --mails <value>    Set number of mails to send per thread.\n"
"                               range for value is 0 to 60000, inclusive.\n"
"                               * A Value of 0 for mails means unlimited mail.\n"
"-s <IP>, --server <IP>         Set the mail server IP\n"
"                               IP is in dotted decimal format\n"
"-p <value>, --port <value>     Set the mail server port\n"
"                               value is the port number of the mail server\n"
"--timeout <value>              Set connection timeout (in seconds)\n"
"                               values is a non zero positive integer\n"
"--conf <filename>              Configuration file to use, instead of the\n"
"                               default file 'xstress.conf'.\n"
"                               `filename` is the full path to the alternate\n"
"                               configuration file.\n"
"--logfile <filename>           Set alternate log file to use, instead of the\n"
"                               default 'xstress.log' file.\n"
"                               `filename` is the full path to the alternate\n"
"                               log file.\n"
"--license                      View disclaimer and short license notice.\n"
"\n"
"**Note: command line options always override configuration file settings\n";

enum eCmdOpts
{
  CO_NONE,
  CO_V,
  CO_VER,
  CO_VERSION,
  CO_HELP,
  CO_H,
  CO_THREADS,
  CO_T,
  CO_M,
  CO_MAILS_PER_THREADS,
  CO_SERVER,
  CO_S,
  CO_PORT,
  CO_P,
  CO_TIMEOUT,
  CO_CONF,
  CO_LOGFILE,
  CO_LICENSE,
  CO_MAX
};

enum eSections
{
  S_NO_SECTION,
  S_CONFIG,
  S_TO,
  S_FROM,
  S_SUBJECT,
  S_BODY,
  S_ATTACH,
  S_MAX
};

extern const char *SectionsStr[];

enum eConfigVars 
{
  C_NO_VAR,
  C_SERVER,
  C_PORT,
  C_THREADS,
  C_MAILS_PER_THREAD,
  C_LOG_FILE,
  C_TIMEOUT,
  C_REPORT_AFTER,
  C_LOG_TIMEOUT,
  C_MAX
};

extern const char * ConfigVarsStr[];

enum eError
{
  NO_ERR,
  ERR_TIMEOUT,
  ERR_INVALID_SOCKET,
  ERR_CONNECT,
  ERR_IO
};

enum eState
{
  IDLE,     // 0
  CONNECT,  // 1
  EHLO,     // 2
  MAILFROM, // 3
  RCPTTO,   // 4 
  DATA,     // 5
  MAIL,     // 6
  FINISHED  // 7
};

enum eSockState
{
  WRITE_READY,
  READ_READY
};

enum eMIMETypes
{
  MT_NONE,
  MT_PLAIN,
  MT_HTML,
  MT_BIN,
  MT_IMAGE,
  MT_MAX
};

#endif
