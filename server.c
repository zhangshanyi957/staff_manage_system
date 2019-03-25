#include "net.h"

int main(int argc, const char *argv[]) 
{ 
	sqlite3 * db; //数据库 
	char *errmsg; //数据库错误信息 
	int sockfd; //服务端套接字 
	int acceptfd; //接收到的套接字
	struct sockaddr_in serveraddr; 
	MSG msg;
	USER user;
	pid_t pid; //输入检测

	if(argc != 3) {
		printf("Usag:%s serverip port\n",argv[0]); 
		return -1;
	}

	//打开数据库
	if(sqlite3_open(DATABASE,&db) != SQLITE_OK) { 
		printf("%s\n",sqlite3_errmsg(db)); 
		return -1;
	} 

	//创建密码表 
	sqlite3_exec(db,"create table book1(type Integer,name text primary key,passwd text,no Integer);",NULL,NULL,&errmsg);

	//创建信息表 
	sqlite3_exec(db,"create table book2(name text primary key,addr char,age char,level char,no char,salary float,phone char,type char);",NULL,NULL,&errmsg); 
	//注册超级用户    
	register_super(db);
	//创建套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("创建失败\n"); 
			return -1; 
	} 
	//优化允许绑定地址快速重用 
	int b_reuse = 1; 
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int)); 
	//填充sockaddr_in结构体 
	bzero(&serveraddr,sizeof(serveraddr)); 
	serveraddr.sin_family = AF_INET; 
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); 
	serveraddr.sin_port = htons(atoi(argv[2])); 
	//绑定 
	if(bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
	{ 
		perror("绑定失败"); 
			return -1; 
	} 
	//监听
	if(listen(sockfd,5)<0)
	{ 
		printf("监听失败\n");
		return -1; 
	} 
	//处理僵尸进程 
	signal(SIGCHLD,SIG_IGN); 
	//并发服务器 
	while(1)
	{ 
		//接收客户端请求 
		if((acceptfd = accept(sockfd,NULL,NULL))<0)
		{ 
			perror("接收失败"); 
				return -1; 
		}
		//创建子进程 
		if((pid = fork())<0)
		{ 
			perror("创建失败"); 
				return -1; 
		}
		else
			if(pid == 0)//子进程执行操作
			{ 
				close(sockfd); 
				do_client(acceptfd,db); 
			}
			else
				//父进程等待接收客户端请求 
			{ 
				close(acceptfd); 
			} 
	} 
	return 0;
}
//超级用户
int register_super(sqlite3 *db) { 
	char * errmsg; 
	char sql[128]; 
	//创建超级用户
	sprintf(sql,"insert into book1 values(0,'root','123',0);"); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{ 
		printf("用户已存在\n"); 
	} else { 
		printf("创建成功\n"); 
	} return 0; 
}
//处理客户端请求
int do_client(int acceptfd,sqlite3 *db) 
{ 
		MSG msg;
			while(recv(acceptfd, &msg, sizeof(msg),0) > 0) { 
				switch(msg.type) {
				case 1:  //登陆
					land1(acceptfd,&msg,db);
					break; 
				case 2: //增加用户 
					add1(acceptfd,&msg,db); 
					break; 
				case 3: //修改用户密码 
					change2(acceptfd,&msg,db); 
					break; 
				case 4: //查找用户 
					find1(acceptfd,&msg,db);
					break; 
				case 5: //删除用户 
					delete(acceptfd,&msg,db);
					break;
				case 6: //修改用户信息 
					change3(acceptfd,&msg,db); 
					break; 
				default: 
					printf("Invalid data msg.\n"); 
				} 
			} 
			printf("client exit.\n"); 
			close(acceptfd); 
			exit(0); 
			return 0; 
} 
//登录
int land1(int acceptfd, MSG *msg,sqlite3 *db) { 
	int nrow; 
	int ncloumn; 
	char sql[128];
	char *errmsg = NULL;
	char **resultp = NULL; 
	char chaoji[] = "0" ; //用来判断用户类型 

	//匹配用户信息是否与密码表中相同 
	sprintf(sql,"select * from book1 where name = '%s' and passwd = '%s';" ,msg->user.name,msg->user.passwd); 
	if(sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg) != SQLITE_OK) { 
		printf("%s\n",errmsg); 
		return -1; 
	} 

	//密码表中存在改用户
		if(nrow == 1) { 
			strcpy(msg->data,"登录成功");
			msg->sign = 1; //代表操作成功 

			//判断是管理员还是普通用户 
				if(strcmp(chaoji,resultp[ncloumn]) == 0) //管理员 
				{
					msg->user.type = 0;
				} else { //普通用户
					msg->user.type = 1; 
				} 
		} 
	//密码表中不存在改用户
		if(nrow == 0) { 
			strcpy(msg->data,"用户密码错误"); 
			msg->sign = 0; //代表操作失败 
		} 
	//返回用户等级信息 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
			perror("返回错误"); 
		} 
	return 0; 
}
//添加用户
int add1(int acceptfd, MSG *msg,sqlite3 *db) { 
	char * errmsg; 
	char sql[521]; 
	int k;
	//添加用户登陆信息 
	sprintf(sql,"insert into book1 values('%d','%s','%s',0);" ,msg->user.type,msg->user.name,msg->user.passwd);
	k = 1;
	//添加登陆信息成功 
	strcpy(msg->data,"添加用户成功"); 
	//添加用户详细信息 
	sprintf(sql,"insert into book2 values('%s','%s','%d','%d','%d','%lf','%s','%s');" ,\
			msg->info.name,msg->info.addr,msg->info.age,msg->info.level,msg->info.no,msg->info.salary,msg->info.phone,msg->info.type); 
	k = k+1;//添加用户详细信息成功 
	strcpy(msg->data,"info OK"); 
	if(k==2)//两者都成功 
	{
		msg->sign = 1; //成功标志 
		strcpy(msg->data,"添加用户成功!");//成功术语
	} else {
		msg->sign = 0;//失败标志 
		strcpy(msg->data,"添加用户失败!");//失败术语 
	}
	//将是否成功的消息返回去 
	if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
		perror("返回失败"); 
	} return 0; 
} 
//修改密码
int change2(int acceptfd, MSG *msg,sqlite3 *db) 
{ 
	char *errmsg = NULL; 
	char sql[521]; 
	int k; 
	char **resultp = NULL; 
	int nrow;
	int ncloumn;//互斥检测 
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回 
	{ 
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
			perror("发送失败"); 
		}
		return 0; 
	}
	//更新密码 
	sprintf(sql,"update book1 set passwd = %s where name = '%s' ", msg->user.passwd,msg->user.name); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK) { 
		printf("更改失败\n"); 
		strcpy(msg->data,"更改失败");
		msg->sign = 0; //失败标志       
	} else { 
		strcpy(msg->data,"更改成功"); 
		msg->sign = 1; //成功标志 
	}
	//将是否成功的消息返回去 
	if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
		perror("发送失败"); 
	}
	//互斥解锁 
	mutex_after(msg,db); 
	return 0; 
} 
//查询
int find1(int acceptfd, MSG *msg,sqlite3 *db) 
{
	char sql[128] = {0}; 
	char *errmsg = NULL; 
	char **resultp = NULL; 
	int nrow;
	int ncloum;
	int i,j;
	int index = 0;
	sprintf(sql,"select *from book2 where name = '%s' ",msg->user.name); 
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloum,&errmsg) != SQLITE_OK) { 
		printf("%s\n",errmsg); 
		msg->sign = 0; //失败标志 
	} else { 
		msg->sign = 1; //成功标志 
	} 
	strcpy(msg->data,":"); 
	index = ncloum; 
	for(j = 0; j < ncloum; j++) { 
		strcat(msg->data,resultp[index++]); 
		strcat(msg->data," - "); 
	}
	//将是否成功的消息返回去
	if(send(acceptfd,msg,sizeof(MSG),0) < 0) {
		perror("发送失败"); 
	} 
	return 0; 
} 
//删除
int delete(int acceptfd, MSG *msg,sqlite3 *db) { 
	int k = 0; 
	int nrow; 
	int ncloumn; 
	char * errmsg = NULL; 
	char sql[521] = {0}; 
	char **resultp = NULL; //互斥检测 
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回 
	{ 
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
			perror("fail to send"); 
		} 
		return 0; 
	}
	//删除用户对应的密码表 
	sprintf(sql,"delete from book1 where name = '%s' ",msg->user.name); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK) { 
		printf("更改失败\n"); 
		strcpy(msg->data,"更改失败"); 
		k = 1; 
	} else { 
		strcpy(msg->data,"删除成功"); 
		msg->sign = 1; //成功标志       
	}
	//删除用户对应的信息表 
	sprintf(sql,"delete from book2 where name = '%s' ",msg->user.name); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK) { 
		printf("删除失败\n"); 
		strcpy(msg->data,"删除失败"); 
		k = 1; 
	} else { 
		strcpy(msg->data,"删除成功"); 
		msg->sign = 1; //成功标志   
	} 
	if(k == 1) msg->sign = 0; //失败标志
	//将是否成功的消息返回去 
	if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
		perror("发送失败"); 
	}
	
	//互斥解锁 
	mutex_after(msg,db); 
	return 0; 
} 
//修改信息
int change3(int acceptfd, MSG *msg,sqlite3 *db) 
{
	char * errmsg = NULL; 
	char sql[521] = {0}; 
	int k; 
	char **resultp = NULL; 
	int nrow; 
	int ncloumn; //互斥检测 
	if(mutex_front(msg,db) == -1)//如果有锁，直接返回 
	{ 
		msg->sign = 0; 
		if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
			perror("发送失败"); 
		} 
		return 0; 
	}
	//如果是超级用户就执行改工资操作 
	if((msg->user).type == 0) { 
		sprintf(sql,"update book2 set salary = %lf where name = '%s' ",msg->info.salary,msg->user.name); 
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK) { 
			printf("修改成功\n"); 
			strcpy(msg->data,"修改失败"); 
			msg->sign = 0; //失败标志       
		} else { 
			strcpy(msg->data,"修改成功"); 
			msg->sign = 1; //成功标志 
		} 
	}
	//普通用户和超级用户都可以改以下信息 
	sprintf(sql,"update book2 set addr = '%s' where name = '%s' ",msg->info.addr,msg->user.name); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK) { 
		printf("修改失败\n"); 
		strcpy(msg->data,"修改失败"); 
		msg->sign = 0; //失败标志       
	} else { 
		strcpy(msg->data,"修改成功"); 
		msg->sign = 1; //成功标志 
	}
	//将是否成功的消息返回去 
	if(send(acceptfd,msg,sizeof(MSG),0) < 0) { 
		perror("返回失败"); 
	}
	//互斥解锁 
	mutex_after(msg,db); 
	return 0; 
} 

//互斥检测
int mutex_front(MSG *msg,sqlite3 *db) 
{
	char * errmsg = NULL; 
	char sql[521] = {0}; 
	char **resultp =NULL; 
	int nrow; 
	int ncloumn;
	//互斥检测开始 
	sprintf(sql,"select * from book1 where name = '%s' and no = 1;",msg->user.name); 
	sqlite3_get_table(db, sql, &resultp, &nrow, &ncloumn, &errmsg); 
	if(nrow == 1) { 
		strcpy(msg->data,"互斥"); 
		msg->sign = 0; //代表操作失败发送失败信息 
		return -1; 
	}
	//没找到说明没有人操作这个数据，将no置1 
	if(nrow == 0) { 
		sprintf(sql,"update book1 set no = 1 where name = '%s' ",msg->user.name); //上锁 
		sqlite3_exec(db,sql,NULL,NULL,&errmsg); 
	} 
	return 0; 
} 
//互斥解锁
int mutex_after(MSG *msg,sqlite3 *db) 
{ 
	char * errmsg = NULL; 
	char sql[521] = {0}; 
	sprintf(sql,"update book1 set no = 0 where name = '%s' ",msg->user.name); 
	sqlite3_exec(db,sql,NULL,NULL,&errmsg); 
	return 0;
}

