#ifndef ___NET_H_
#define ___NET_H_

#include <stdio.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sqlite3.h>

#define DATABASE "user.db"
#define N 512
//登录信息
typedef struct user{
	int type;		//判断是否是管理员
	char name[N];
	char passwd[N];
	int no;
}USER;
/*员工信息 */
typedef struct info{
	int no;			//编号
	char name[N];			  //姓名
	int age;					  //年龄
	char phone[N];		  //电话号码
	char addr[N]; 		  //家庭住址
	char type[N];		  //职位
	int level;					  //水平
	double salary;				  //工资
	int usertype;				  //admin 1 user 2
}INFO;
/*通信结构体*/
typedef struct msg{
	int sign;					//操作是否成功,0成功，1失败
	int type;				   //请求的类型，想干嘛，定义功能的类型
	USER user;			   		//user
	INFO info;		   		//员工信息
	int flags;					   //标志位 标志寄存器
	char data[N];			   //操作结果返回的消息
}MSG;
/*线程处理数据*/
struct thread_data{
	int acceptfd;					
	pthread_t pthread;			
	int state;						//线程状态
	MSG *msg;						//通信的消息
	void *prvi_data;				//扩展的私有数据
};


int acceptfd; //接收的套接字
int sockfd;		//服务端套接字
sqlite3 * db; 		//数据库
char *errmsg;	//错误信息

//注册第一个超级用户
int register_super(sqlite3 *db);
//处理函数
int do_client(int acceptfd,sqlite3 *db);
//登陆函数
int land1(int acceptfd, MSG *msg,sqlite3 *db);
//增加函数
int add1(int acceptfd, MSG *msg,sqlite3 *db);
//修改密码函数
int change2(int acceptfd, MSG *msg,sqlite3 *db);
//查找函数
int find1(int acceptfd, MSG *msg,sqlite3 *db);
//删除函数
int delete(int acceptfd, MSG *msg,sqlite3 *db);
//修改信息函数
int change3(int acceptfd, MSG *msg,sqlite3 *db);
//互斥函数
int mutex_front(MSG *msg,sqlite3 *db);
//解锁函数
int mutex_after(MSG *msg,sqlite3 *db);


int land(int socket,MSG *MSG);
int add(int socket,MSG *MSG);
int change(int sockfd, MSG *MSG);
int find(int sockfd, MSG *MSG);
int change1(int sockfd, MSG *MSG);
int delete1(int sockfd,MSG *MSG);


#endif 
