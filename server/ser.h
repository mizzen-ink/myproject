#include<iostream>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<event2/event.h>
#include<jsoncpp/json/json.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fcntl.h>
using namespace std;
const int LIS_MAX=20;
const string PATH="/home/mzz/PROJECT2411/server/download/";
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
    bool Create_Socket();//创见套介子
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