/*
 * gcc -O -o rdr_port_ipt  rdr_port_ipt.c
 */
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>
#include <regex.h>

// #define DEBUG
#define NO_PRINT

#define BUFSIZE 18
#define SEC 5
#define STEP 1

static int timeout=SEC,status=0;
static char ip[BUFSIZE+1],sport[6]="80",dport[6]="22";

void usage(char **self)
{
   fprintf(stderr,"Usage: \n %s ipaddress [second [a|b [a|b]]] \n       ipaddress: xxx.xxx.xxx.xxx ,source IP\n second: timeout, 3-36000sec (default:%d)\n   sport: a=80 b=443 (default:%s)\n dport: a=22 b=1080 (default:%s)\n Example: \n %s 10.1.2.3 \n %s 10.1.2.3 10 \n %s 10.1.2.3 25 b b\n\n",
        self[0],timeout,sport,dport,self[0],self[0],self[0]);
   exit(-1);
}

void create_deamon()
{
    long i, maxfd;
    if (fork())
        exit(0);
    setsid();
    signal(SIGHUP, SIG_IGN);
    if (fork())
        exit(0);
    chdir("/");
    maxfd = sysconf(_SC_OPEN_MAX); //获取一个进程能打开的最大文件数
    for (i = 0; i < maxfd; i++)
        close(i);
    open("/dev/null", O_RDWR); //open a file "0"
    dup(0); // dup "0" to "1" , 0=stdin, 1=stdout, 2=stderr
    dup(1); // dup "1" to "2"
    dup(2); // dup "2" to "3"
    /*
     if ((i = open("/dev/tty", O_RDWR)) > 0) {
       ioctl(i, TIOCNOTTY, 0);
       close(i);
     }
    */
    return;
}

void chk_time(int signo)
{
  timeout--;
  if(timeout<1) status=1;  // timeout
  return; // ok
}

void end_it(int signo)
{
// 删除防火墙的端口重定向规则
  if(fork()==0) {
     execl("/sbin/iptables","iptables","-t","nat","-D","PREROUTING","-s",ip,"-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
     //execl("/sbin/iptables","iptables","-t","nat","-D","PREROUTING","-s",ip,"-d","172.12.34.5","-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
  }
  wait(NULL); // wait for child
  closelog();  // close  system logger
  exit (0);
}

int main(int argc, char **argv)
{
  struct itimerval value;
  regex_t preg;
  int err;
  char * pp;

  if (argc <2) { usage(argv); }
  if (strlen(argv[1])>BUFSIZE) argv[1][BUFSIZE-1]='\0'; // 防止超过BUFSIZE
  // 获取第一个参数 IP
  strcpy(ip,argv[1]);
  regcomp(&preg,"^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$",REG_EXTENDED|REG_ICASE|REG_NEWLINE|REG_NOSUB);
  err=regexec(&preg,argv[1],0,NULL,0);
  regfree(&preg);
  if(err == REG_NOMATCH) { usage(argv); }

  if (argc >2)
  {
    if(strlen(argv[2])>BUFSIZE) argv[2][BUFSIZE-1]='\0'; // 防止超过BUFSIZE
    // 获取第二个参数，时间
    regcomp(&preg,"^[0-9]{1,5}$",REG_EXTENDED|REG_ICASE|REG_NEWLINE|REG_NOSUB);
    err=regexec(&preg,argv[2],0,NULL,0);
    regfree(&preg);
    if(err == REG_NOMATCH) { usage(argv); }
    timeout=atoi(argv[2]);
    if(timeout<3) timeout = 3;
    if(timeout>36000) timeout = 36000;
  }

  if (argc >3)
  { // 第三个参数
    if(argv[3][0]=='b') { sprintf(sport,"443"); }
    //else if(argv[3][0]=='c') { sprintf(sport,"995"); }
    else if(argv[3][0]!='a') { usage(argv); }
  }
  if (argc >4)
  { // 第四个参数
    if(argv[4][0]=='b') { sprintf(dport,"1080"); }
    //else if(argv[3][0]=='c') { sprintf(sport,"110"); }
    else if(argv[4][0]!='a') { usage(argv); }
  }

  openlog("RDR_PORT",LOG_CONS,LOG_USER); // open system logger
  // 写入防火墙的端口重定向规则
  if(fork()==0) {
     execl("/sbin/iptables","iptables","-t","nat","-A","PREROUTING","-s",ip,"-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
     //execl("/sbin/iptables","iptables","-t","nat","-A","PREROUTING","-s",ip,"-d","172.12.34.5","-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
  }
  syslog(LOG_NOTICE,"%s %s->%s for %i sec",ip,sport,dport,timeout);
  wait(NULL); // wait for child

  create_deamon();

  //初始化定时器中断信号
  signal(SIGALRM,(void *)chk_time);
  value.it_value.tv_sec=STEP;
  value.it_value.tv_usec=0;
  value.it_interval.tv_sec=STEP;
  value.it_interval.tv_usec=0;
  setitimer(ITIMER_REAL,&value,(struct itimerval *)0);

  //定义结束，终止 信号
  signal(SIGTERM,end_it);
  signal(SIGINT,end_it);

  while (status==0) { pause();}
  switch(status)
  {
    case 1:
      #ifndef NO_PRINT
      printf("Empty deny table,normal quit!\n");
      #endif
      break;
    default:
      syslog(LOG_NOTICE,"unknow err!");
  }

  end_it(1);

}
