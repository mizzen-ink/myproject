

all:server
   
  
 

server:ser.cpp mysql_con.cpp	
	g++ -o server ser.cpp mysql_con.cpp -ljsoncpp -levent -lmysqlclient
   
clean:
	rm -f *.o server
