/*
 * gcc -O -o rdr_port_nft  rdr_port_nft.c
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
char *HANDLE=NULL;

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
     //execl("/sbin/iptables","iptables","-t","nat","-D","PREROUTING","-s",ip,"-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
     //execl("/sbin/iptables","iptables","-t","nat","-D","PREROUTING","-s",ip,"-d","172.12.34.5","-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
     //execl("/usr/sbin/nft","nft","delete","rule","ip","nat","PREROUTING","ip","saddr",ip,"tcp","dport",sport,"redirect","to",dport,(char *) NULL);
     execl("/usr/sbin/nft","nft","delete","rule","ip","nat","PREROUTING",HANDLE,(char *) NULL);
     // nft delete rule ip nat PREROUTING handle 10
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
  int len;
  char * pp;
  char buf[101];
  int fd1[2], fd2[2]; // pipe

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
  if(pipe(fd1) == -1) {
    syslog(LOG_ERR,"%s %s->%s,pipe(stdout) ERR",ip,sport,dport);
    return 1; // 退出
  }
  if(pipe(fd2) == -1) {
    syslog(LOG_ERR,"%s %s->%s,pipe(stderr) ERR",ip,sport,dport);
    return 1; // 退出
  }
  // 写入防火墙的端口重定向规则
  if(fork()==0) {
    close(fd1[0]); // close pipe read
    close(fd2[0]); // close pipe read
    if(dup2(fd1[1], STDOUT_FILENO) == -1) {
      syslog(LOG_ERR,"%s %s->%s,dup(stdout) ERR",ip,sport,dport);
      return 1; // 退出
    }
    if(dup2(fd2[1], STDERR_FILENO) == -1) {
      syslog(LOG_ERR,"%s %s->%s,dup(stderr) ERR",ip,sport,dport);
      return 1; // 退出
    }
    //execl("/sbin/iptables","iptables","-t","nat","-A","PREROUTING","-s",ip,"-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
    //execl("/sbin/iptables","iptables","-t","nat","-A","PREROUTING","-s",ip,"-d","172.12.34.5","-p","tcp","--dport",sport,"-j","REDIRECT","--to",dport,(char *) NULL);
    execl("/usr/sbin/nft","nft","-eann","add","rule","ip","nat","PREROUTING","ip","saddr",ip,"tcp","dport",sport,"redirect","to",dport,(char *) NULL);
    // nft -ea add rule ip nat PREROUTING ip saddr 192.168.12.34 tcp dport 2022 redirect to 22
  }
  close(fd1[1]); // close pipe write
  close(fd2[1]); // close pipe write
  wait(NULL); // wait for child
  if((len=read(fd2[0], &buf, 50)) > 0) { // stderr msg
    buf[len]='\0';
    if (strncmp("Error:",buf,6) == 0) {
      while(read(fd2[0], &buf, 50) > 0); //读完它
      close(fd1[0]);
      close(fd2[0]);
      syslog(LOG_ERR,"%s %s->%s,nft FAIL,quit",ip,sport,dport);
      return 1; // 退出
    }
    while(read(fd2[0], &buf, 50) > 0); //读完它
  }

  if((len=read(fd1[0], &buf, 100)) <= 0) { // stdout msg
    close(fd1[0]);
    close(fd2[0]);
    syslog(LOG_ERR,"%s %s->%s,nft NO output,quit",ip,sport,dport);
    return 1; // 退出
  }
  buf[len]='\0';
  HANDLE=strstr(buf," handle ");
  if(HANDLE == NULL) {
    syslog(LOG_ERR,"%s %s->%s,nft,handle not found,quit",ip,sport,dport);
    return 1; // 退出
  }
  HANDLE++;
  pp=strchr(HANDLE,'\n');
  if(pp != NULL) {
    pp[0]='\0';
  }
  //printf("handle: (%s)\n",HANDLE);
  while(read(fd1[0], &buf, 50) > 0); //读完它
  close(fd1[0]);
  close(fd2[0]);
  syslog(LOG_NOTICE,"%s %s->%s for %i sec #%s",ip,sport,dport,timeout,HANDLE); // send msg to system logger

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
