#include<iostream>
#include<string>
#include<mysql/mysql.h>
using namespace std;
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