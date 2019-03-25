#include "net.h"

int main(int argc, const char *argv[])
{
//socket->填充->绑定->监听->等待连接->数据交互->关闭 
 	
	int sockfd;
	int k;
	struct sockaddr_in serveraddr;
	int choice; //用户选项
	MSG msg;	

	//输入检测
	 if(argc != 3)
	 {
		 printf("Usag:%s serverip port\n",argv[0]);
		 return -1;
	 }
	 //创建套接字
	 if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	 {
		 perror("创建失败\n");
		 return -1;
	 }
	 //绑定
	 bzero(&serveraddr,sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	 serveraddr.sin_port = htons(atoi(argv[2]));
	 //链接
	 if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	 {
		 perror("链接失败");
		 return -1;
	 }

	while(1){
		printf("*******************************************************\n");
		printf("******************1:登录   2:退出  ********************\n");
		printf("*******************************************************\n");

		printf("请输入命令:");
		scanf("%d",&choice);
		getchar();

		switch(choice){
		case 1:
			k = land(sockfd,&msg);
			if(k == 0)
			{
				goto next1;
			}
			if(k == 1)
			{
				goto next2;
			}
			printf("msg.data = %s\n",msg.data);
			break;
		case 2:
			close(sockfd);
			exit(0);
			break;
		default:
			printf("在命令行输入的参数是无效的\n");
		}
	}
next1:
	while(1){

		printf("********************************************************\n");
		printf("*****1:添加 2:修改密码 3:查找 4:删除用户 5:更改信息*****\n");
		printf("****6:退出                                         ****\n" );
		printf("********************************************************\n");

		printf("请输入命令:");
		scanf("%d",&choice);
		getchar();
		(msg.user).type = 0;
		switch(choice){
		case 1:
			add(sockfd,&msg);
			break;
		case 2:
			change(sockfd,&msg);
			break;
		case 3:
			find(sockfd,&msg);
			break;
		case 4:
			delete1(sockfd,&msg);
			break;
		case 5:
			change1(sockfd,&msg);
			break;
		case 6:
			close(sockfd);
			exit(0);
			break;
		default:
			printf("在命令行输入的参数是无效的\n");
		}
	}
next2:
	while(1){

		printf("********************************************************\n");
		printf("*********1:修改密码 2:查找 3:修改信息 4: 5:退出*********\n");
		printf("********************************************************\n");

		printf("请输入命令:");
		scanf("%d",&choice);
		getchar();
		switch(choice){
		case 1:
			change(sockfd,&msg);
			break;
		case 2:
			find(sockfd,&msg);
			break;
		case 3:
			change1(sockfd,&msg);
			break;
		case 4:
			close(sockfd);
			exit(0);
			break;
		default:
			printf("在命令行输入的参数是无效的\n");
		}
	}

return 0;

}


//登录
int land(int sockfd, MSG *msg)
{
	//登陆选项1
	msg->type = 1;
	printf("请输入用户名:\n");
	scanf("%s",msg->user.name);
	printf("请输入密码(6位):\n");
	scanf("%s",msg->user.passwd);

	//发送账号密码
	if(send(sockfd,msg,sizeof(MSG),0) < 0)
	{
		printf("发送失败\n");
		return -1;
	}
	//接收用户等级
	if(recv(sockfd,msg,sizeof(MSG),0)< 0)
	{
		printf("接收失败\n");
		return	-1;
	}
	//本次操作如果失败直接返回
	if(msg->sign==0)
	{
		printf("**********发送失败***********\n");
		return -1;
	}
	//如果是超级用户返回0否则返回1
	if(msg->user.type==	0)
			return 0;
	if(msg->user.type==	1)
		return	1;
}
//增加
int add(int sockfd, MSG *msg) 
{ 
	//确定操作类型 
	msg->type=2; 
	printf("请选择用户级别 (超级用户：0 或 普通用户：1): "); 
	scanf("%d",&(msg->user).type);
	printf("请输入添加用户的姓名: "); 
	scanf("%s",(msg->user).name);
	strcpy((msg->info).name,(msg->user).name); 
	printf("请输入添加用户的密码（6位）: "); 
	scanf("%s",(msg->user).passwd); 
	printf("请输入添加用户的地址: "); 
	scanf("%s",(msg->info).addr); 
	printf("请输入添加用户的年龄: "); 
	scanf("%d",&((msg->info).age)); 
	printf("请输入添加用户的等级: "); 
	scanf("%d",&((msg->info).level)); 
	printf("请输入添加用户的等级:"); 
	scanf("%d",&((msg->info).no)); 
	printf("请输入添加用户的工资:"); 
	scanf("%lf",&(msg->info).salary); 
	printf("请输入添加用户的手机号:"); 
	scanf("%s",(msg->info).phone); 
	printf("请输入添加用户的级别:");

	scanf("%d",&((msg->info).type)); 
	if(send(sockfd,msg,sizeof(MSG),0)<0)
	{ 
		printf("发送失败\n"); 
			return -1; 
	} 
	//接收成功与否 
	if(recv(sockfd,msg,sizeof(MSG),0)<0)
	{ 
		printf("接收失败\n"); 
			return -1; 
	} 
	//本次操作如果失败直接返回 
	if(msg->sign == 0)
	{ 
		printf("******失败**********\n"); 
		return -1; 
	} 
	//操作成功打印成功信息 
	printf("添加成功!\n"); 
	return 0; 
}
//修改密码
int change(int sockfd, MSG *msg) 
{ 
	//操作类型 
	msg->type=3; 
	//选择名字权限 
	if((msg->user).type == 0) { 
		printf("请输入您要更改的用户名:"); 
		scanf("%s",(msg->user).name); 
	} 
	//修改的密码 
	printf("请输入要更改用户名的密码（6位）:"); 
	scanf("%s",(msg->user).passwd);
	if(send(sockfd,msg,sizeof(MSG),0)< 0) { 
		printf("修改失败\n"); 
			return -1; 
	} 
	//接收是否成功
	if(recv(sockfd,msg,sizeof(MSG),0)<0)
	{ 
		printf("接收失败.\n"); 
			return -1; 
	} 
	//本次操作如果失败直接返回 
	if(msg->sign == 0)
	{ 
		printf("*********失败********\n"); 
		return -1;
	} 
	//操作成功打印成功信息 
	printf("修改成功!\n");
	return 0; 
}
//查找
int find(int sockfd, MSG *msg) 
{ 
	//确定操作类型 
	msg->type=4; 
	//看是否有选择名字权限 
	if((msg->user).type == 0) {
		printf("请输入要查找的用户名:"); 
		scanf("%s",(msg->user).name); 
	} 
	if(send(sockfd,msg,sizeof(MSG),0) <0) { 
		printf("发送失败\n"); 
		return -1; 
	} 
	//接收成功与否 
	if(recv(sockfd,msg,sizeof(MSG),0)< 0) { 
		printf("接收失败\n"); 
			return -1; 
	} 
	//本次操作如果失败直接返回 
	if(msg->sign == 0)
	{ 
		printf("*********失败*********\n"); 
		return -1; 
	}
	//操作成功打印成功信息 
	printf("%s\n",msg->data); 
	return 0; 
} 
//删除用户

int delete1(int sockfd, MSG *msg) 
{ 
	//操作类型 
	msg->type=5; 
	printf("请输入要删除的用户名:"); 
	scanf("%s",(msg->user).name); 
	if(send(sockfd,msg,sizeof(MSG),0) < 0) { 
		printf("发送失败\n"); 
		return -1; 
	} 
	//接收成功与否 
	if(recv(sockfd,msg,sizeof(MSG),0)< 0) { 
		printf("接收失败\n"); 
			return -1; 
	} 
	//本次操作如果失败直接返回 
	if(msg->sign == 0) { 
		printf("********失败***********\n"); 
		return -1; 
	}
	//操作成功打印成功信息 
	printf("%s\n",msg->data); 
	return 0; 
} 
//更改信息
int change1(int sockfd, MSG *msg) 
{ 
	//操作类型 
	msg->type=6; 
	//看是否有权限
	if((msg->user).type == 0) { 
		printf("请输入要修改用户的姓名:"); 
		scanf("%s",(msg->user).name); 
		printf("请输入要更改用户的工资:"); 
		scanf("%lf",&(msg->info).salary); 
	} 
	//修改的地址 
	printf("请输入要更改用户的地址:"); 
	scanf("%s",(msg->info).addr); 
	if(send(sockfd,msg,sizeof(MSG),0)< 0) { 
		printf("发送失败\n"); 
		return -1;
	} 
	//接收成功与否 
	if(recv(sockfd,msg,sizeof(MSG),0)<0)
	{ 
		printf("接收失败\n"); 
			return -1; 
	} 
	//本次操作如果失败直接返回 
	if(msg->sign == 0)
	{ 
		printf("**********失败*********\n"); 
		return -1; 
	} 
	//操作成功打印成功信息 
	printf("修改成功!\n"); 
	return 0; 
}
