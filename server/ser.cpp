
#include <iostream>
#include <cstring>
#include <jsoncpp/json/json.h>
#include <event2/event.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
using namespace std;
const int LIS_MAX=20;
const string PATH="/home/mzz/PROJECT2411/server/download/";
//定义用户的操作
extern "C" {
void Accept_CallBack(int,short,void*);
void Read_CallBack(int,short,void*);
}
class Server//处理事件的监听
{
    public:
    //构造函数e
    Server()
    {
        cout<<"server"<<endl;
        ip="127.0.0.1";
        port=6000;
    }
    Server(string ip,int port)
    {
        this->ip=ip;
        this->port=port;
        sockfd=-1;
        base=NULL;//管理事件的结构体
    }
    bool Ser_Init();//套接子初始化
    bool Accept_Client();//客户端的连接
    void Run();//服务器的运行
    ~Server()
    {
        cout<<"server"<<endl;
        close(sockfd);
        if(base!=NULL)
        {
            event_base_free(base);
        }
    }
    private:
    bool Create_Socket();//创见套接子
    bool Libevent_Init();//创建libevent实例
    private:
     string ip;
    short port;
    int sockfd;//创建监听套接字
    struct event_base* base;//定义libevent示例
};
//处理客户端的连接//·请求
class Con_Client
{
    public:
    Con_Client(int c,struct event_base* b)
    {
        this->c=c;
        base =b;
        fd=-1;
    }
    void Set_event(struct event* e)
    {
        ev=e;
    }
    void Recv_Data();
    ~Con_Client()
    {
        if(ev!=NULL)
        {
            event_free(ev);
        }
        close(c);
        cout<<"client clodse"<<endl;
    }
    private:
    bool is_json(const char buff[]);
    void send_ok();
    void send_err();
    void do_run(int op);//处理json
    void Register();
    void Login();
    void send_Json(Json::Value& v);
    void showfiles();
    void get_file(string fname);
    void get_file(char* ptr);
    void Mkdir();
    void Rmfile();
    void Rename();
    void Chdir();
    void Ret();
    private:
    Json::Value val;
    int c;
    struct event_base* base;
    struct event* ev;
    string mypath;//存放文件的位置，不同用户位置不同
    string userpath;
    int fd;
};
class mysqlclient
{
    private:
    string dbusername;//用户名
    string dbpasswd;//数据库密码
    string dbip;//
    string dbname;//数据库名字
    short dbport;
    MYSQL mysql_con;//连接句柄
    public:
    mysqlclient()
    {
    dbusername="root";
    dbpasswd="123456";
    dbip="127.0.0.1";
    dbname="project_db";
    dbport=3306;
    }
    bool connectserver();
   ~mysqlclient()
   {
    
   }
    bool db_register(const string usertel,const string username,const string passwd);
    bool db_login(const string usertel, string& username,const string passwd );
};
enum OPS_TYPE{USEREXIT=0,REGISTER,LOGIN,SHOWFILES,GET,POST,MKDIR,RMFILE,MVNAME,CHDIR,RET,MVFILE};
void Con_Client::send_ok()
{
    Json::Value v;
    v["status"]="OK";
    send(c,v.toStyledString().c_str(),strlen(v.toStyledString().c_str()),0);

}
void Con_Client::send_err()
{
    Json::Value v;
    v["status"]="ERR";
    send(c,v.toStyledString().c_str(),strlen(v.toStyledString().c_str()),0);
}
void Con_Client::send_Json(Json::Value& v)
{
    send(c,v.toStyledString().c_str(),strlen(v.toStyledString().c_str()),0);
}
 bool Con_Client::is_json(const char buff[])
 {
    if(buff[0]=='{')
    {
        return true;
    }
    return false;
 } 
 void Con_Client::Register()
 {
    string usertel=val["usertel"].asString();
    string username=val["username"].asString();
    string userpasswd=val["passwd"].asString();
    mysqlclient mysqlcli;
    if(!mysqlcli.connectserver())
    {
        send_err();
        return ;
    }
    if(!mysqlcli.db_register(usertel,username,userpasswd))
    {
        send_err();
        return ;
    }
    //创建该用户的目录
    mypath=PATH+usertel;
    userpath=mypath;
    if(mkdir(mypath.c_str(),0775)==-1)
    {
        send_err();
        return ;
    }
    send_ok();
    return;
 }
 void Con_Client::Login()
 {
    string usertel=val["usertel"].asString();
    string passwd=val["passwd"].asString();
    string username="";
    if(usertel.empty()||passwd.empty())
    {
        send_err();
        return ;
    }
    mysqlclient cli;
    if(!cli.connectserver())
    {
        send_err();
        return ;
    }
    if(!cli.db_login(usertel,username,passwd))
    {
        send_err();
        return ;
    }
       mypath=PATH+usertel;
       userpath=mypath;
    if(mkdir(mypath.c_str(),0775)==-1)
    {
        send_err();
        return ;
    }
    Json::Value v;
    v["status"]="OK";
    v["username"]=username;
    send_Json(v);
    return ;
 }
  void Con_Client::showfiles()
  {
    DIR* ptr=opendir(mypath.c_str());//返回一个目录流，即结构体指针，里面是目录路径
    if(ptr==NULL)
    {
        cout<<"opendir err:"<<mypath<<endl;
        send_err();
        return ;
    }
    int ndirs=0;
    int nfiles=0;
    Json::Value resval;
    struct dirent *s=nullptr;
    struct stat st;
    while((s=readdir(ptr))!=nullptr)
    {
        if(strncmp(s->d_name,".",1)==0)
        {
            continue;
        }
        string filename=mypath+"/"+s->d_name;
        if(lstat (filename.c_str(),&st)==-1)
        {
            cout<<"lstat err"<<endl;
            continue;
        }
        //获取指定路径文件（或目录）的详细属性信息，放到ST中
        if(S_ISDIR(st.st_mode))//这行代码的作用是 判断一个文件（或目录）是否为目录。
        {
            //目录文件
            Json::Value tmp;
            tmp["filename"]=string(s->d_name);
            resval["arrdir"].append(tmp);
            ndirs++;
        }
        else{
            Json::Value tmp;
            tmp["filename"]=string(s->d_name);
            resval["arrdir"].append(tmp);
            nfiles++;
        }

    }
    closedir(ptr);
    resval["ndirs"]=ndirs;
    resval["status"]="OK";
    resval["nfiles"]=nfiles;
    send(c,resval.toStyledString().c_str(),strlen(resval.toStyledString().c_str()),0);
    
  }
  void Con_Client::Chdir()
  {
    string dname=val["dirname"].asString();
    string testpath=mypath+"/"+dname;
    DIR*ptr=opendir(testpath.c_str());
    if(ptr==nullptr)
    {
        send_err();
        return;
    }
    mypath=testpath;
    closedir(ptr);
    send_ok();
    return;
  }
void Con_Client::Ret()
{
    if(userpath==mypath)
    {
        send_ok();
        return ;
    }
    size_t pos =mypath.find_last_of("/");
    if(pos==string::npos)
    {
        send_err();
        return;
    }
    mypath=mypath.substr(0,pos);
    send_ok();
}bool mysqlclient::connectserver()
{
    if(mysql_init(&mysql_con)==NULL)
    {
        return false;
    }
    if(mysql_real_connect(&mysql_con,dbip.c_str(),dbusername.c_str(),dbpasswd.c_str(),dbname.c_str(),dbport,NULL,0)==NULL)
    {
        return false;
    }
    return true;
}
bool mysqlclient::db_register(const string usertel,const string username,const string passwd)
{
    if(usertel.empty()||username.empty()||passwd.empty())
    {
        return false;
    }
    string sql="insert into User_Info values(0,'"+usertel+"','"+username+"','"+passwd+"','1',now())";
    if(mysql_query(&mysql_con,sql.c_str())!=0)
    {
        return false;
    }
    return true;
}
  bool mysqlclient::db_login(const string usertel, string& username,const string passwd )
  {
    string sql="select name,passwd from User_Info where tel="+usertel;
    if(mysql_query(&mysql_con,sql.c_str())!=0)
    {
        return false;
    }
    MYSQL_RES* r =mysql_store_result(&mysql_con);//返回结果集
    if(r==NULL)
    {
        return false;
    }
    //查询结果集是否有一条
    if(mysql_num_rows(r)==0)//手机号码不存在
    {
        return false;
    }
    //提取该记录
    MYSQL_ROW row=mysql_fetch_row(r);
    string pw=row[1];
    if(passwd!=pw)
    {
        return false;
    }
    username=row[0];//用户名
    mysql_free_result(r);//释放结果集 
    return true;
  } void Con_Client::do_run(int op)
 {
    switch(op)
    {
        case REGISTER:
        Register();
            break;
            case LOGIN:
            Login();
            break;
            case SHOWFILES:
            showfiles();
            break;
            case GET:
           
            break;
            case POST:
            break;
            case MKDIR:
            Mkdir();
            break;
            case RMFILE:
            Rmfile();
            break;
            case MVNAME:
            Rename();
            break;
            case CHDIR:
            Chdir();
            break;
            case RET:
            Ret();
            break;
            case MVFILE:
            break;
            case USEREXIT:
            break;
            default:
            break;
    }
 }
 void  Con_Client::get_file(char* ptr)
 {
    char *status=strtok_r(NULL," ",&ptr);
    if(status==nullptr)
    {
        send(c,"ERR",3,0);
        return;
    }
    if(strcmp(status,"start")==0)
    {
        char* fname=strtok_r(NULL," ",&ptr);
        string pathname=mypath+"/"+fname;
        fd=open(pathname.c_str(),O_RDONLY);
        if(fd==-1)
        {
            send(c,"ERR",3,0);
            return;
        }
        int filesize=lseek(fd,0,SEEK_END);//获取文件大小
        lseek(fd,0,SEEK_SET);//文件便宜量移动到起始位置
        string r_str="OK "+to_string(filesize);
        send(c,r_str.c_str(),strlen(r_str.c_str()),0);//ok 2345
    }
    else if(strcmp(status,"continue")==0)
    {
        char buff[128]={0};
        int num=read(fd,buff,128);
        send(c,buff,num,0);
    }
    else if(strcmp(status,"stop")==0)
    {
        close(fd);
    }
 }
 void  Con_Client::get_file(string fname)
 {
    string pathname=mypath+"/"+fname;
    int fd=open(pathname.c_str(),O_RDONLY);
    if(fd==-1)
    {
        send(c,"ERR",3,0);
        return;
    }
    int filesize=lseek(fd,0,SEEK_END);//获取文件大小
    lseek(fd,0,SEEK_SET);//文件便宜量移动到起始位置
    string r_str="OK "+to_string(filesize);
    send(c,r_str.c_str(),strlen(r_str.c_str()),0);//ok 2345
    char s_buff[128]={0};
    int n=recv(c,s_buff,127,0);
    if(n<=0)
    {
        close(fd);
        return ;
    }
    if(strcmp(s_buff,"y")!=0)
    {
        close(fd);
        return;
    }
    char data[512]={0};
    int num=0;
    while((num=read(fd,data,512))>0)
    {
        send(c,data,num,0);
    }
    close(fd);
    return ;
 }
 void Con_Client::Mkdir()
 {
    string dname=val["dirname"].asString();
    string filepath=mypath+"/"+dname;
    if(mkdir(filepath.c_str(),0775)==-1)
    {send_err();
        return;
    }
    send_ok();
    return;
 }
 void  Con_Client::Rmfile()
 {
    string filename=val["filename"].asString();
    string filepath=mypath+"/"+filename;

    if(access(filepath.c_str(),F_OK)==-1)//判断文件是否存在
    {
        send_err();
        return;
    }
struct stat st;
if(stat(filepath.c_str(),&st)==-1)
{
    send_err();
    return ;
}
if(S_ISDIR(st.st_mode))
{
    if(rmdir(filepath.c_str())==-1)
    {
        send_err();
        return;
    }
}
else
{
   if( unlink(filepath.c_str())==-1)
   {
    send_err();
    return;
   }
   send_ok();
   return;
}
 }
 void Con_Client::Rename()
 {
    string s_name=val["sname"].asString();
    string t_name=val["tname"].asString();
    string s_filepath=mypath+"/"+s_name;
    string t_filepath=mypath+"/"+t_name;
    if(rename(s_filepath.c_str(),t_filepath.c_str())==-1)
    {
        send_err();
        return;

    }
    send_ok();
    return;
 }
void Con_Client:: Recv_Data()
{
    char buff[256]={0};
    int n=recv(c,buff,255,0);
    if(n<=0)
    {
        //event_free(ev);//移除libevent中的管理事件

        delete this;//删除对象
        //关闭事件描述符c
      
        return ;
    }
    cout<<"recv:"<<buff<<endl;
    if(is_json(buff))
    {
        Json::Reader Read;
    if(!Read.parse(buff,val))
        {
        cout<<"json无法解析"<<endl;
        send_err();
        return ;
        }
        int op=val["type"].asInt();
        do_run(op);
    }
    else
    {
        //自定义协议 下载
        char*ptr=nullptr;
        char* s=strtok_r(buff," ",&ptr);
        if(s == nullptr)
        {
            send(c,"ERR",3,0);
            return;
        }
        if(strcmp(s,"get")==0)//下载
        {
            // s=strtok_r(nullptr," ",&ptr);
            // if(s==nullptr)
            // {
            // send(c,"ERR",3,0);
            // return;
            // }
            get_file(ptr);//新版处理
        }
        else if(strcmp(s,"up")==0)//上传
        {
            cout<<"无法解析的操作"<<endl;
            send(c,"ERR",3,0);
            return;
        }
    }

}
bool Server::Accept_Client()
{
    //1.创建监听套接子
    //2.创建事件，加入事件
    int c=accept(sockfd,NULL,NULL);//1.监听套接字2.存储客户段的ip和端口3.addr长度
    if(c==0)
    {
    return false;
    }
    Con_Client* p=new Con_Client(c,base);//通过这个类来处理客户端的请求，手动销毁，在堆区申请销毁不了，临时变量会随着程序结束而销毁
    struct event* c_ev=event_new(base,c,EV_READ|EV_PERSIST,Read_CallBack,static_cast<void*>(p));//数据类型的转换
    if(c_ev==NULL)
    {
        return false;
    }
    p->Set_event(c_ev);//new出来的事件放进这个类对象里（地址传入）
    event_add(c_ev,NULL);
    return true;
}
bool Server::Create_Socket()
{//类内
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if( -1 == sockfd )
    {
        return false;
    }

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip.c_str());

    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if( -1 == res)
    {
        return false;
    }

    res = listen(sockfd,LIS_MAX);
    if( -1 == res )
    {
        return false;
    }

    return true;
}
bool Server::Libevent_Init()
{

    base = event_base_new();
    if( nullptr == base)
    {
        return false;
    }

    if( -1 == sockfd )
    {
        return false;
    }
    struct event* sock_ev = event_new(base,sockfd,EV_READ|EV_PERSIST,Accept_CallBack,static_cast<void*>(this) );
    if( sock_ev == nullptr)
    {
        return false;
    }

    event_add(sock_ev,nullptr);

    return true;
}

void Server::Run()
{
    event_base_dispatch(base);
}
bool Server::Ser_Init()
{
    if(!Create_Socket())
    {
        return false;
    }
    if(!Libevent_Init())
    {
        return false;
    }
    return true;
}
int main()
{
    Server ser;
    if(!ser.Ser_Init())
    {
        cout<<"!init err"<<endl;
        return 1;
    }
    ser.Run();
    return 0;
}
