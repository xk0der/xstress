## xstress version 0.40 beta - xk0derz SMTP Stress Tester

(c) Amit Singh, amit@xkoder.com    
http://xkoder.com    
With contributions from [Matthias Egger](https://github.com/chmatse) ❤️    


This software and related files are licensed under GNU GPL version 2    
Please visit the following webpage for more details    
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html    
or read the accompanying LICENSE file.    

Special thanks to Bob Trower for the base64 utility he made    
http://base64.sourceforge.net/    
This utility is licensed under the MIT license    
please see the b64.c source file for the complete license.    



### Introduction
xstress is an SMTP Stress Testing Tool. Essentially xstress creates multiple
connections to (your) mail server and sends multiple mails repeatedly over all
connections.

xstress understands only SMTP protocol. No authentication mechanism is
supported at the moment.

xstress may be used to stress test normal SMTP servers, test Anti-Virus and
Anti-Spam softwares that accept or filter SMTP connections and the like.

using xstress you can specify a list of the following objects to be used for
sending mails:

    1) TO address list
    2) FROM address list
    3) SUBJECT list
    4) list of BODY content files/directories
    5) list of ATTACHMENT content files/directories

xstress randomly makes a combination from the available list and sends mails to
the configured SMTP server.

### Quick Setup
If you have Docker, clone the repo 

    $ git clone https://github.com/xk0der/xstress.git

And run the following command to test out xstress

    $ ./dev-setup

Dev-Setup will create two containers, a test SMPT server using https://maildev.github.io/maildev/    
And an xstress container, pre-configured with test server's IP and port

Now run the following command to get inside the xstress docker container

    $ docker exec -it xstress-container bash

From inside the container you can now execute ./xstress as needed

    $ cd /bin
    # ./xstress

Check the mails being sent at [127.0.0.1:1080](http://127.0.0.1:1080)

### Compiling from source

Make sure you the C++ tooling and make tools installed

Clone the repo 

    $ git clone https://github.com/xk0der/xstress.git

From inside the repo run

    # ./build

The above command will create the xtress binary in the bin folder and copy the
default xstress.conf file to the bin folder. If the file already exists you'll
be aksed if you want to over-write it or not.

The make command also creates the base64 utility used by xstress and copies it
to the bin directory

*NOTE: base64 utility is required for xstress to send binary attachments properly.*


### Usage and command line options

Move to the bin folder and type `./xstress` to invoke the tool.

Syntax:
    
    $ xstress [options]

Here options are one or more of the following:
```
-v, --ver, --version           Display xstress version and exit.

-h, --help                     Display this help message

-t <value>, --threads <value>  Set number of threads to spawn.
                               Range for value is 1 to 100, inclusive.

-m <value>, --mails <value>    Set number of mails to send per thread.
                               range for value is 0 to 60000, inclusive.
                               * A Value of 0 for mails means unlimited mail.

-s <IP>, --server <IP>         Set the mail server IP
                               IP is in dotted decimal format

-p <value>, --port <value>     Set the mail server port
                               value is the port number of the mail server

-u <username>                  You SMTP server username.
--username <username>

-P <password>                  Password for the username you specified using
--password <password>          the -u or --username switch.
                                
                               * Username and password both have to be provided
                               for authentication to work. If either is not
                               specified, authentication will be skipped.

-a <type>, --auth-type <type>  Authentication mechanism to use.
                               * Currently only PLAIN is supported.

--timeout <value>              Set connection timeout (in seconds)
                               values is a non zero positive integer

--conf <filename>              Configuration file to use, instead of the
                               default file 'xstress.conf'. `filename` is the
                               full path to the alternate configuration file.

-d, --debug                    If this switch is specified, verbose debug
                               messages with complete traffic will be printed
                               on screen.

-l <FQDN>, --helo <FQDN>       Which Full Quallified Domain Name should be used
                               as the HELO/EHLO Hostname. If not specified,
                               localhost is used (which is not a FQDN but at
                               least a valid entry).

--logfile <filename>           Set alternate log file to use, instead of the
                               default 'xstress.log' file. `filename` is the 
                               full path to the alternate log file.
```
*Note: command line options always override configuration file settings*



### Configuration file
By default xstress reads its configuration from the file 'xstress.conf'. This
file should be present from where you invoke the xstress program.

A typical configuration file consists of six sections. Out of these the CONFIG
section, TO section, SUBJECT section and FROM section are compulsory.

COMMENTS start with the hash (#) sign on the 'first column only'. Blank lines
are ignored. The parser is not designed to handle extra spaces so don't play
with them.

```
Var = Value :  will NOT work
Var=Val     : will work!
 Var=Val    : will NOT work

TO section
FROM section contains list of email addresses, one per line, to be used
    as recipent address and sender address respectively.

SUBJECT section contains list of subject text, one per line, to used for
    e-mails being sent by xstress. 

BODY section contains list of (text) files or directory. The content of these
    files is used as the mail's body text. If a directory is specified, Content
    of all the files under that directory is used as body text.

ATTACHMENT Sections contains list of (text) files or directory. The listed
    files will be  attached to the mails being sent out from xstress. If a
    directory is specified, all files present within that directory are used
    for attachment.

    The file list or directory name can be prfixed by an optinal FILETYPE tag
    as follows.

    [ATTACHMENT]
    path/to/file!type
    path/to/directory!type

    The exclamation sign (!) separates the attachment file-name/directory from
    the type.   
  
    The following types ares understood by xstress.

    1) bin    - binary files
                eg: MS Word document, compressed archives, MP3 files etc.
                ** Whenever in doubt use this type
    2) plain  - Plain text files
                eg: Source Code, Text files, scripts etc.
    3) html   - HTML source
    4) image  - Image files 
                Any image file, treated as `bin' but disposition is inline.

CONFIG Section contains variables that configure the mail server to be used,
    number of threads and the like. Variables understood by xstress are
    described below

    SERVER      Specify the SMTP Mail Server's IP here.
                Example: 172.16.0.1

    PORT        Specify the SMTP Mail Server's port number here.
                This is typically 25.

    USERNAME    Your SMTP server username.

    PASSWORD    Password for your username.
                * To skip authentication comment out USERNAME or PASSWORD
                variable.

    AUTH_TYPE   Authentication mechanism to use. Currently only PLAIN is
                supported.

    HELO        Define a Hostname which should be used for the HELO/EHLO
                command. You can uncomment this variable which then defaults to
                `localhost`. Some servers only accept a Full Qualified Domain
                Name (FQDN) in the format `hostname.domainname.tld`.

    DEBUG       set to true if you want debug messages with complete traffic
                printed on scree. Default value is false.

    THREADS     Specify number of threads to spwan. The value can range from
                1 to 60000
            
                A thread represents one connection to the Mails Server.  

                The higher the number of threads, the more mails per second
                can be sent by xstress.

MAILS_PER_THREAD    Speciy the number of mails to be sent per thread. The value
                    for this can range from 0 to 60,000.

                    value of ZERO means unlimited number of mails per thread.
                    You have to use ^C (control + C) to quit xstress if you use
                    ZERO for this variable.

REPORT_AFTER    Specify the number of mails, after which xstress will print a
                message on screen.

                A Value of ZERO means print no message.
              
                Message Printed is in the format

                    Thread:: <id>: T: <num1>, S: <num2>, F: <num3>

                Where 
                    id the thread ID
                    num1 is [T]otal number of mails to be sent for that thread
                    num2 is count of mails [S]ent succesfully
                    num3 is count of mails [F]ailed

    LOG_FILE    Set the log file to be used if you dont want to use the default
                log file 'xstress.log'. After xstress finishes, a thread by
                thread report is append to this log file.

    TIMEOUT     Specify connection timeout in seconds after which xstress will
                give up.

                TIP: If you are specifying ZERO for MAILS_PER_THREAD, put a low
                value here, as you would not like xstress to wait for too long
                before it sends a new mail. A low value will timeout that
                session sooner and xstress will try sending a new mail (taking
                corrective action if possible).

                If you specify some amount of mails, you may like to put a
                large number for TIMEOUT, so that before calling it quit,
                xstress will wait to see if the server responds.
```


### Example attachment and body files
The `bin` directory contains some example body and attachment files. The default
configuration file xstress.conf lists these files to be used as body and
attachment for the mail.

```
    Body Files:
        body1.txt - Plain Text file
        body_dir/body2.txt - Plain Text file

    Attachment Files:
        attach_dir/images_dir/xstress.jpg - Image file
        attach_dir/attach1.htm - HTML File
```
