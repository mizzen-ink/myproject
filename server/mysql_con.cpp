#include"mysql_con.h"
bool mysqlclient::connectserver()
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
  }