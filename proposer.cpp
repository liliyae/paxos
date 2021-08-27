#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/shm.h>
#define htons1 1234
#define htons2 1235
#define htons3 1236

int indexofproposer;
int rnd;
char v[20];
int *currentrnd;
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
void writeacceptor(int h1,int h2)
{

    while(1)
    {
    
    rnd=*currentrnd+1;
    *currentrnd=*currentrnd+1;
    //connect with first proposer
  //创建套接字
    int sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //将套接字和IP、端口绑定
    struct sockaddr_in addr1;
    memset(&addr1, 0, sizeof(addr1));  //每个字节都用0填充
    addr1.sin_family = AF_INET;  //使用IPv4地址
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    addr1.sin_port = htons(h1);  //端口
    
 //connect with second proposer
   //创建套接字
    int sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //将套接字和IP、端口绑定
    struct sockaddr_in addr2;
    memset(&addr2, 0, sizeof(addr2));  //每个字节都用0填充
    addr2.sin_family = AF_INET;  //使用IPv4地址
    addr2.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    addr2.sin_port = htons(h2);  //端口
    
      int con1 = connect(sock1, (struct sockaddr*)&addr1, sizeof(addr1));  
      if(con1==-1)
         continue;
      int con2 = connect(sock2, (struct sockaddr*)&addr2, sizeof(addr2));
      if(con2==-1)
         continue;
      char buffer1[40];
      char rebuffer1[40];
      char buffer2[40];
      char rebuffer2[40];
      //phase1
      buffer1[0]='0';
      
      //向 acceptor 1 发送数据
      int tmp=rnd;
      for(int i=4;i>=1;i--)
      {   
         buffer1[i]='0'+tmp%10;
         tmp/=10;
      }

      write(sock1, buffer1, sizeof(buffer1));
      printf("Phase1:\n向 acceptor %d 发送数据 rnd = %d \n",h1, rnd);
      //读取 acceptor 1传回的数据
      read(sock1, rebuffer1, sizeof(rebuffer1)-1);     
      int lastrnd1;
      char v1[20];
      int vrnd1;
      if(rebuffer1[0]=='0')//拒绝请求
      {
         printf("拒绝请求\n");
         break;
      }
      else
      {
         lastrnd1=0;
         for(int i=1;i<5;i++)
         {
            lastrnd1*=10;
            lastrnd1+=rebuffer1[i]-'0';
         }
         for(int i=5;i<25;i++)
         {
            v1[i-5]=rebuffer1[i];
         }
         vrnd1=0;
         for(int i=25;i<29;i++)
         {
            vrnd1*=10;
            vrnd1+=rebuffer1[i]-'0';
         }
         printf("收到应答: last rnd = %d ,v = %s, vrnd = %d\n",lastrnd1,v1,vrnd1);
      }

      //向 acceptor 2 发送数据
      tmp=rnd;
      buffer2[0]='0';
      for(int i=4;i>=1;i--)
      {   
         buffer2[i]='0'+tmp%10;
         tmp/=10;
      }
      while(1)
      {
         int con = write(sock2, buffer2, sizeof(buffer2));
         if(con!=-1)
           break;
      }
      printf("向 acceptor %d 发送数据 rnd = %d \n",h2, rnd);
      	   
      //读取 acceptor 2传回的数据
      read(sock2, rebuffer2, sizeof(rebuffer2)-1);
       
      int lastrnd2;
      char v2[20];
      int vrnd2;
      if(rebuffer2[0]=='0')//拒绝请求
      {
         printf("拒绝请求\n");
         continue;
      }
      else
      {
         lastrnd2=0;
         for(int i=1;i<5;i++)
         {
            lastrnd2*=10;
            lastrnd2+=rebuffer2[i]-'0';
         }
         for(int i=5;i<25;i++)
         {
            v2[i-5]=rebuffer2[i];
         }
         vrnd2=0;
         for(int i=25;i<29;i++)
         {
            vrnd2*=10;
            vrnd2+=rebuffer2[i]-'0';
         }
         printf("收到应答: last rnd = %d ,v = %s, vrnd = %d\n",lastrnd2,v2,vrnd2);
      }
      sleep(1);
      close(sock1);
      close(sock2);
       
       //connect with first proposer
  //创建套接字
    sock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //将套接字和IP、端口绑定
    memset(&addr1, 0, sizeof(addr1));  //每个字节都用0填充
    addr1.sin_family = AF_INET;  //使用IPv4地址
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    addr1.sin_port = htons(h1);  //端口
    
 //connect with second proposer
   //创建套接字
    sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //将套接字和IP、端口绑定
    memset(&addr2, 0, sizeof(addr2));  //每个字节都用0填充
    addr2.sin_family = AF_INET;  //使用IPv4地址
    addr2.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    addr2.sin_port = htons(h2);  //端口
      //phase2
    connect(sock1, (struct sockaddr*)&addr1, sizeof(addr1));  
    connect(sock2, (struct sockaddr*)&addr2, sizeof(addr2));
      
      printf("Phase2: \n");
      if(lastrnd1>rnd||lastrnd2>rnd)
      {
         break;
      } 
      buffer1[0]='1';//表示phase2
      if(vrnd1==0&&vrnd2==0)//所有应答都为空
      {
         for(int i=1;i<21;i++)
         {
            buffer1[i]=v[i];
         }
         tmp=rnd;
         for(int i=24;i>=21;i--)
         {   
            buffer1[i]='0'+tmp%10;
            tmp/=10;
         }
         write(sock1, buffer1, sizeof(buffer1));
         write(sock2, buffer1, sizeof(buffer1));
         printf("phase1中所有应答都为空，发送 v= %s,  rnd =%d \n",v,rnd);
      }
      else //不为空，有人先写了
      {
         if(vrnd1>vrnd2)
         {
            for(int i=1;i<21;i++)
            {
               buffer1[i]=v1[i-1];
            }
            printf("phase1中所有应答不全为空，发送 v= %s,  rnd =%d \n",v1,rnd);
         }
         else
         {
            for(int i=1;i<21;i++)
            {
               buffer1[i]=v2[i-1];
            }
            printf("phase1中所有应答不全为空，发送 v= %s,  rnd =%d \n",v2,rnd);
         }
         tmp=rnd;
         for(int i=24;i>=21;i--)
         {   
            buffer1[i]='0'+tmp%10;
            tmp/=10;
         }
         write(sock1, buffer1, sizeof(buffer1));
         write(sock2, buffer1, sizeof(buffer1));                

      }
      read(sock1, rebuffer1, sizeof(rebuffer1));
      read(sock2, rebuffer2, sizeof(rebuffer2));  
      if(rebuffer1[0]=='1'&&rebuffer2[0]=='1')
      {  
         printf("%d写入成功\n",indexofproposer); 
         break;
      }
      else
      { 
          printf("%d写入失败\n",indexofproposer);
      }
     
     sleep(1);
    //关闭套接字
     close(sock1);
     close(sock2);
     }
}

int main(){
    key_t buff_key = 110;
    int shm_flg = IPC_CREAT | 0644;//共享内存读写权限
    currentrnd = (int *)set_shm(buff_key, 20, shm_flg);
    *currentrnd = 0;
    
    printf("hi, proposer, What is the index of you? 1 or 2 ?");
    scanf("%d",&indexofproposer);
    printf("What do you want wo write?");
    scanf("%s",v);
    if(indexofproposer==1)
        writeacceptor(htons1,htons2);
    else if(indexofproposer==2)
        writeacceptor(htons3,htons2);
    else
        printf("error index");
    return 0;
}

