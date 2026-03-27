 #include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<jsoncpp/json/json.h>
#include<fcntl.h>
using namespace std;
//定义用户的操作
enum OPS_TYPE{USEREXIT=0,REGISTER,LOGIN,SHOWFILES,GET,POST,MKDIR,RMFILE,MVNAME,CHDIR,RET,MVFILE};
class client
{
private:
void print();
void Register();
void Login();
void showfiles();
void get_file();
void Mkdir();
void Rmfile();
void Rename();
void Chdir();
void Ret();
private:
    string ip;
    short port;
    int sockfd;//和服务器通信的套接子
    bool flg;//显示登陆没登陆
    int op;//用户操作选项ID
    string usertel;
    string username;

public:
    bool connect_ser();
    client();
    client(string ips,short port);
    ~client();
    void  run();
};

client::client()//·创建套接子不建议用构造函数，失败了怎么正，用bool类型的函数来推断成没成功
{
    ip="127.0.0.1";
    port=6000;
    sockfd=-1;
    flg=false;
    op=-1;
}
client::client( string ips,short port)
{
    ip=ips;
    this->port=port;
    sockfd=-1;
    flg=false;
    op=-1;
}
client::~client()
{
}
