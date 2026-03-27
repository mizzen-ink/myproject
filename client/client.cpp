#include"client.h"

bool client::connect_ser()
{
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==sockfd)
    {
        return false;
    }
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    saddr.sin_addr.s_addr=inet_addr(ip.c_str());
    int res=connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res==-1)
    {
        return false;
    }
    return true;
}
void client::print()
{
    if(!flg)
    {
        cout<<"1.注册 2.登陆 3.退出"<<endl;
        cout<<"请输入选择："<<endl;
        //cin>>op;
        while(!(std::cin>>op))
        {
            std::cin.clear();
            std::cin.ignore(10000,'\n');
            std::cerr<<"Invalid input. Please enter a valid integer:";

        }
        if(3==op)
        {
            op=USEREXIT;
        }
    }
    else{
        cout<<"------用户："<<username<<"-------"<<endl;
        cout<<"1.查看服务器文件  2.下载   3.上传    4.新建目录"<<endl;
        cout<<"5.删除文件       6.重命名 7.进入目录 8.返回"<<endl;
        cout<<"9.拷贝文件      10.退出"<<endl;
        cout<<"请输入要执行的操作编号："<<endl;
        while(!(std::cin>>op))
        {
            std::cin.clear();
            std::cin.ignore(10000,'\n');
            std::cerr<<"Invalid input. Please enter a valid integer:";
            
        }
        if(10==op)
        {
            op=USEREXIT;
            return;
        }
        op+=2;
    }
  
}
void client:: Register()
{
    
    //设计协议 json、
    cout<<"请输入手机号码"<<endl;
    cin>>usertel;
    cout<<"请输入用户名"<<endl;
    cin>>username;
    cout<<"请输入密码"<<endl;
    string pw1,pw2;
    cin>>pw1;
    cout<<"请再次输入密码"<<endl;
    cin>>pw2;
    if(usertel.empty()||username.empty())
    {
        cout<<"用户名或手机号码不能为空"<<endl;
        return ;
    }
    if(pw1.empty()||pw2.empty()||pw1!=pw2)
    {
        cout<<"密码不能为空"<<endl;
        return ;
    } 
    Json::Value val;
    val["type"]= REGISTER;
    val["usertel"]=usertel;
    val["passwd"]=pw1;
    val["username"]=username;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[128]={0};
    int n=recv(sockfd,buff,127,0);
    if(n<=0)
    {
        cout<<"ser close"<<endl;
        return;
    }
    Json::Value resval;
    Json::Reader Read;
    if(!Read.parse(buff,resval))
    {
        cout<<"解析json失败"<<endl;
        return ;
    }
    string s=resval["status"].asString();
    if(s!="OK")
    {
        cout<<"注册失败"<<endl;
    }
    cout<<"注册成功"<<endl;
    flg=true;
    return ;
}
void client::Login()
{
    string usertel;
    string passwd;
    cout<<"请输入手机号码："<<endl;
    cin>>usertel;
    cout<<"请输入密码："<<endl;
    cin>>passwd;
    if(usertel.empty()||passwd.empty())
    {
        cout<<"用户名密码不能为空"<<endl;
        return;
    }
    Json::Value val;
    val["type"]=LOGIN;
    val["usertel"]=usertel;
    val["passwd"]=passwd;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);//序列化

    char buff[128]={0};//接受数据反序列化
    if(recv(sockfd,buff,127,0)<=0)
    {
        cout<<"ser close"<<endl;
        return ;
    }
    Json::Reader Read;
    Json::Value resval;
    if(!Read.parse(buff,resval))
    {
        cout<<"json无法解析"<<endl;
        return;
    }
    string s=resval["status"].asString();
    if(s!="OK")
    {
        cout<<"登陆失败"<<endl;
        return ;
    }
    cout<<"登陆成功"<<endl;
    username=resval["username"].asString();
    flg=true;
    return ;
    
}
void client::showfiles()
{
    Json::Value val;
    val["type"]=SHOWFILES;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    //接受数据
    char buff[4096]={0};
    int n=recv(sockfd,buff,4096,0);
    if(n<=0)
    {
        cout<<"ser close"<<endl;
        return ;
    }
    Json::Value resval;
    Json::Reader Read;
    if(!Read.parse(buff,resval))
    {
        cout<<"无法解析json"<<endl;
        return ;
    }
    string s=resval["status"].asString();
    if(s!="OK")
    {
        cout<<"查看失败"<<endl;
        return ;
    }
    int ndirs=resval["ndirs"].asInt();
    cout<<"目录文件"<<endl;
    for(int i=0;i<ndirs;i++)
    {
        cout<<i<<". "<<resval["arrdir"][i]["filename"].asString()<<endl;
    }
    cout<<"普通文件："<<endl;
    int nfiles=resval["nfiles"].asInt();
    for(int i=0;i<nfiles;i++)
    {
        cout<<i<<". "<<resval["arrfile"][i]["filename"].asString()<<endl;

    }
}
void client::get_file()
{
    string fname;
    cout << "请输入要下载那个文件:" << endl;
    cin >> fname;
    if (fname.empty())
    {
        cout << "文件名不能为空" << endl;
        return;
    }

    string q_str = "get start " + fname;                   // get start code.deb
    send(sockfd, q_str.c_str(), strlen(q_str.c_str()), 0); // 要下载那个文件
    char buff[128] = {0};
    int n = recv(sockfd, buff, 127, 0);
    if (n <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    char *s = strtok(buff, " "); // ok 3245, err
    if (s == nullptr)
    {
        cout << "报文解析错误" << endl;
        return;
    }

    if (strcmp(s, "OK") != 0) // err
    {
        cout << "无法下载" << endl;
        return;
    }

    s = strtok(NULL, " "); // filesize
    if (s == nullptr)
    {
        cout << "报文解析错误" << endl;
        return;
    }

    int filesize = atoi(s);
    cout << "文件大小：" << filesize << endl;
    cout << "是否下载(y/n?)" << endl;
    string sel;
    cin >> sel;
    int fd=-1;
    if(sel=="y")
    {
    int fd = open(fname.c_str(), O_CREAT | O_WRONLY, 0600);
    }
    if (fd == -1 || sel == "n"||filesize==0)
    {
        // send(sockfd, "n", 1, 0);
        const char *s_stop = "get stop";
        send(sockfd, s_stop, strlen(s_stop), 0);
        if(fd!=-1)
        {
            close(fd);
        }
        return;
    }

    const char *s_continue = "get continue";
    char data[1024];
    int num = 0;
    int cur_filesize = 0;
    while (true)
    {
        // 客户端向服务器发送一次下载请求
        send(sockfd, s_continue, strlen(s_continue), 0);
        // 接收服务器返回的数据
        num = recv(sockfd, data, 1024, 0);
        if (num <= 0)
        {
            cout << "接收数据错误或服务器关闭" << endl;
            const char *s_stop = "get stop";
            send(sockfd, s_stop, strlen(s_stop), 0);
            break;
        }

        write(fd, data, num);
        cur_filesize += num;
        float f = cur_filesize * 100.0 / filesize;
        
        printf("下载：%.2f%%\r", f);
        fflush(stdout);

        if (cur_filesize == filesize)
        {
            cout << "\n下载完成" << endl;
            break;
        }
    }

    //计算md5值，和服务器返回的md5比较
    close(fd);
  
}
void client::Mkdir()
{
    cout << "请输入新建目录的名称：" << endl;
    string dname;
    cin >> dname;

    if (dname.empty())
    {
        cout << "目录名不能为空" << endl;
        return;
    }

    Json::Value val;
    val["type"] = MKDIR;
    val["dirname"] = dname;

    // 发送创建目录请求
    send(sockfd, val.toStyledString().c_str(), val.toStyledString().length(), 0);
    
    char buff[128] = {0};
    int n = recv(sockfd, buff, 127, 0);
    if (n <= 0)
    {
        cout << "服务器连接关闭" << endl;
        return;
    }

    Json::Value rval;
    Json::Reader Read;
    if (!Read.parse(buff, rval))
    {
        cout << "json 无法解析" << endl;
        return;
    }

    string s = rval["status"].asString();
    if (s != "OK")
    {
        cout << "创建失败：" << rval["msg"].asString() << endl;
        return;
    }

    cout << "创建成功" << endl;
}
void client::Rmfile()
{
    cout<<"请输入要删除的文件"<<endl;
    string fname;
    cin>>fname;
    if(fname.empty())
    {
        cout<<"文件名🙈为空"<<endl;
        return;
    }
    Json::Value val;
    val["type"]=RMFILE;
    val["filename"]=fname;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[128]={0};
    int n=recv(sockfd,buff,127,0);
    if(n<=0)
    {
        cout<<"ser close"<<endl;
        return;
    }
    Json::Reader Read;
    Json::Value rval;
    if(!Read.parse(buff,rval))
    {
        cout<<"删除失败"<<endl;
        return;
    }
    cout<<"删除成功"<<endl;
    return ;
}
void client::Rename()
{
    cout<<"输入要修改的文件名"<<endl;
    string sname;
    cin >>sname;
    cout<<"输入新名字"<<endl;
    string tname;
    cin>>tname;
    Json::Value val;
    val["type"]=MVNAME;
    val["sname"]=sname;
    val["tname"]=tname;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[128]={0};
    int n=recv(sockfd,buff,127,0);
    Json::Value rval;
    Json::Reader Read;
    if(!Read.parse(buff,rval))
    {
        cout<<"Json解析失败"<<endl;
        return;
    }
    string s=rval["status"].asString();
    if(s!="OK")
    {
        cout<<"修改失败"<<endl;
        return;
    }
    cout<<"修改成功"<<endl;
    
    
}
void client::Chdir()
{
    cout<<"请输入要进入的目录名"<<endl;
    string dname;
    cin>>   dname;
    Json::Value val;
    val["type"]=CHDIR;
    val["dirname"]=dname;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[128]={0};
    int n=recv(sockfd,buff,127,0);
    if(n<=0)
    {
        cout<<"ser close"<<endl;
        return;
    }
    Json::Value rval;
    Json::Reader Read;
    if(!Read.parse(buff,rval))
    {
        cout<<"无法解析Json"<<endl;
        return;

    }
    string s=rval["status"].asString();
    if(s!="OK")
    {
        cout<<"切换路径失败"<<endl;
        return;
    }

}
void client::Ret()
{
    Json::Value val;
    val["type"]=RET;
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char buff[218]={0};
    int n=recv(sockfd,buff,127,0);
    if(n<=0)
    {
        cout<<"ser close"<<endl;
        return ;
    }    
    Json::Value rval;
    Json::Reader Read;
    if(!Read.parse(buff,rval))
    {
        cout<<"json无法解析"<<endl;
        return;
    }
    string s=rval["status"].asString();
    if(s!="OK")
    {
        cout<<"返回失败"<<endl;
        return;
    }
    cout<<"返回成功"<<endl;
    return;
}
void client:: run()
{
    bool r=true;//也可以break
    while(r)
    {
        //界面提示
        print();
        //用户输入USEREXIT=0,REGISTER,LOGIN,SHOWFILES,GET,POST,MKDIR,RMFILE,MVNAME,CHDIR,RET,MVFILE
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
            get_file();
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
            break;
            case CHDIR:
            Chdir();
            break;
            case RET:
            Ret();
            break;
            case MVFILE:
            Rename();
            break;
            case USEREXIT:
            r=false;
            break;
            default:
            break;
        }
    }
}
int main()
{
    client cli;
    if(!cli.connect_ser())
    {
        cout<<"连接服务器失败"<<endl;
        return 1;
    }
    cli.run();
    return 0;
} 