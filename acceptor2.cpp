#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<stdio.h>
#include <sys/shm.h>
const int ht[3]={1234,1235,1236};
int indexofacceptor;
//int lastrnd,vrnd;
//char v[20];
int *lastrnd;
int *vrnd;
char *v;
char *set_shm(key_t shm_key, int shm_num, int shm_flg) 
{
      int i, shm_id;
      char *shm_buf;
      //shmget 新建 ⼀个⻓度为 shm_num 字节的共享内存,其标号返回shm_id
      if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
          perror("shareMemory set error"); exit(EXIT_FAILURE);
       }
       //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
       if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
          perror("get shareMemory error"); exit(EXIT_FAILURE);
       }
      for (i = 0; i < shm_num; i++) shm_buf[i] = 0; //初始为 0
      //shm_key 标识的共享内存区已经建⽴,将由 shm_id 标识的共享内存附加给指针 shm_buf
      if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
         perror("get shareMemory error");
         exit(EXIT_FAILURE);
      }
      return shm_buf;
}
void print()
{
   printf("\n当前 last rnd = %d , v = %s, vrnd = %d\n",*lastrnd,v,*vrnd);
}
void writeproposer(int h1)
{
     //创建套接字 
     int sock = socket(AF_INET, SOCK_STREAM, 0);
     struct sockaddr_in serv_addr;
     memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
     serv_addr.sin_family = AF_INET;  //使用IPv4地址
     serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
     serv_addr.sin_port = htons(h1);  //端口
     bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //进入监听状态，等待用户发起请求
     listen(sock, 20);
     *vrnd=0;
     *lastrnd=0;

    while(1)
     {
        //printf("dsdsdsd\n");
        //接收客户端请求
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        int clnt_sock = accept(sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
           
        printf("\naccept one requier %d\n",clnt_sock);
        //printf("%d",*lastrnd);
        //printf("%d",*vrnd);
        //printf("%s",v);
        
        char buffer[40];
        char rebuffer[40];
        read(clnt_sock, buffer, sizeof(buffer));
    
        if(buffer[0]=='0')//phase1
        {
           printf("Phase 1: \n");
           print();
           int rnd=0;
           for(int i=1;i<5;i++)//读取rnd
           {
              rnd*=10;
              rnd+=buffer[i]-'0';
           }
           printf("读取rnd: %d\n",rnd);
           if(rnd<=*lastrnd)
           {
              printf("rnd < last rnd ,拒绝请求\n");
              rebuffer[0]='0';    //拒绝请求       
           }  
           else
           {
              rebuffer[0]='1'; 
              int tmp=*lastrnd;
              for(int i=4;i>=1;i--)
              {   
                 rebuffer[i]='0'+tmp%10;
                 tmp/=10;
              }
              for(int i=5;i<25;i++)
              {
                 rebuffer[i]=v[i-5];
              }
              tmp=*vrnd;
              for(int i=28;i>=25;i--)
              {   
                 rebuffer[i]='0'+tmp%10;
                 tmp/=10;
              }
              *lastrnd=rnd;
              
           }         
           //向客户端发送数据
           write(clnt_sock, rebuffer, sizeof(rebuffer));
       
           printf("返回应答");
           print();
        }
        else if(buffer[0]=='1')//phase2
        {
           printf("Phase 2:\n");
           print();
           int rnd=0;
           for(int i=21;i<25;i++)
           {
              rnd*=10;
              rnd+=buffer[i]-'0';
           }
           if(rnd!=*lastrnd)
           {
              rebuffer[0]='0';  //拒绝请求 
              printf("收到请求 rnd = %d,不等于 last rnd = %d ,拒绝请求 \n",rnd,*lastrnd);
           }  
           else 
           {
              rebuffer[0]='1'; //接受请求
              for(int i=1;i<=20;i++)//写入v
              {
                 v[i-1]=buffer[i];
              }
              *vrnd=rnd;
              printf("收到请求 rnd = %d, v = %s ,接受请求\n",rnd,v);
           }
           write(clnt_sock, rebuffer, sizeof(rebuffer));
        
           print();
        }
        sleep(2);
     }
}

int main(){
     key_t buff_key = 104;
     int shm_flg = IPC_CREAT | 0644;//共享内存读写权限
     v = (char *)set_shm(buff_key, 20, shm_flg);
     buff_key=105;
     lastrnd=(int *)set_shm(buff_key, 1, shm_flg);
     buff_key=106;
     vrnd=(int *)set_shm(buff_key, 1, shm_flg);
     writeproposer(ht[1]);
    
     return 0;
 }


