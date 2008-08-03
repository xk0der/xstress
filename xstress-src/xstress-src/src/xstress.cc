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

#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "common.h"
#include "sendmail.h"
#include "config.h"
#include "thread.h"
#include "logger.h"
#include "userinterface.h"

using namespace std;



Logger logger;

unsigned int *pidList;
unsigned int pidCount;
unsigned int totThreads;
int threadForceQuit = false;

void sigHandler(int iSig)
{
  unsigned int ii;
  
  if(pidCount>=totThreads && pidList)
  {
    for(ii=0;ii<pidCount;ii++)
    {
      kill(pidList[ii], SIGHUP);
    }
  }
}

void threadSigHandler(int iSig)
{
  threadForceQuit = true;
}

int main(int argc, char *argv[])
{

  char buffer[1024];
  int  fdSet;
  sockaddr_in addr;
  int Done = 0;
  unsigned int ii;
  int rv;

  pidList = NULL;
  pidCount = 0;
 
  UserInterface uiObj;
  
  signal(SIGINT, sigHandler);
  
  if(uiObj.processOptions(argc, argv)==false)
  {
    return 0;
  }
  
  cout << VERSION << endl;
  cout << COPYRIGHT << endl;
  
  Config configObj(uiObj.sConfigFile);
  if(!configObj.okay())
  {
    cout << "Config file " << uiObj.sConfigFile << " Not found " << endl;
    cout << "Please provide complete path to the xstress configuration file :";
    cin >> uiObj.sConfigFile;
    if(uiObj.sConfigFile.empty()) return 0;
    Config configObj(uiObj.sConfigFile);
    if(!configObj.okay()) 
    {
      cout << "Config file " << uiObj.sConfigFile << " Not found " << endl;
      cout << "Exiting!" << endl;
      return 1;
    }
  }
  

  if(uiObj.override())
  {
    uiObj.setConfig(configObj);
  }
  
  logger.setLogFile(configObj.sLogFile);


  pidList = new unsigned int[configObj.uiThreads];
  totThreads = configObj.uiThreads;
  pidCount = 0;

  if(!pidList)
  {
    cout << "Error: Memory allocation error at" << __FUNCTION__ << " : " << __LINE__ << endl;
    return 1;
  }

  for(ii=0;ii<configObj.uiThreads;ii++)
  {
    socket(AF_INET, SOCK_STREAM, 0);

    rv = fork();

    if(rv==0) /*This is the child*/
    {

      Thread threadObj(ii, configObj.uiMailsPerThread, &configObj);
      if(threadObj.okay()) 
      {
        signal(SIGHUP, threadSigHandler);
        cout << "Thread " << threadObj.getid() << " started!" << endl;
        threadForceQuit = false;
        while(!threadObj.finished() && !threadForceQuit)
        {
          threadObj.process();
          if(!threadObj.okay())
          {
            ;
          }
        }
      }
      if(threadForceQuit)
      cout << "Thread " << threadObj.getid() << " forced to stop!" << endl;
      else
      cout << "Thread " << threadObj.getid() << " finished!" << endl;
      return 0;
    }
    else if(rv!=-1)
    {
      pidList[ii] = rv;
      pidCount++;
    }
    else
    {
      cout << "Error creating thread with id " << ii  << endl;
      totThreads--;
    }
  }

  if(totThreads>0)
  {
    cout << "Waiting for " << configObj.uiThreads << " threads to finish " << endl; 

    for(ii=0;ii<configObj.uiThreads;ii++)
    {
      waitpid(pidList[ii], NULL, 0);
      cout << "Number of threads remaining " << configObj.uiThreads - (ii+1) << endl;
    }

    cout << "All Threads finished, exiting!" << endl;
  }
  else
  {
    cout << "FATAL Error: Unable to spwan even a single thread!!" << endl;
  }

  return 0;
}

