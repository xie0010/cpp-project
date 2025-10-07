#include "ConnectionPool.h"
#include "public.h"

ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    
    FILE* ptr = fopen("mysql.cnf","r");
    if(ptr == nullptr)
    {
        LOG("file error");
        return;
    }
    while(!feof(ptr))
    {
        char line[1024] = {0};
        fgets(line, 1024, ptr);
        std::string str = line;
        int idx = str.find("=",0);
        if(idx == -1){
            continue;
        }
        int endidx = str.find("\n");
        std::string key = str.substr(0,idx);
        std::string value = str.substr(idx+1,endidx-1);

        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeOut")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

ConnectionPool::ConnectionPool()
{
    if(loadConfigFile()){
        return;
    }

    for(int i = 0; i < _initSize; i++)
    {
        Connection* p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime();
        _connectionQue.push(p);
        _connectionCnt++;
    }

    std::thread product([&]()
    {
        this->produceConnectionTask();
    });
    product.detach();

    std::thread scanner([&]()
    {
        this->scannerConnectionTask();
    });
    scanner.detach();
};

void ConnectionPool::produceConnectionTask()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        if(!_connectionQue.empty())
        {
            cv.wait(lock);
        }
        if(_connectionCnt < _maxSize)
        {
            Connection* p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            p->refreshAliveTime();
            _connectionQue.push(p);
            _connectionCnt++;
        }
        cv.notify_all();
    }
}

std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    while(_connectionQue.empty())
    {
        if(std::cv_status::timeout == cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)))
        {
            if (_connectionQue.empty())
            {
                LOG("获取空闲连接超时了...获取连接失败!");
                    return nullptr;
            }
        }
        std::shared_ptr<Connection> p (_connectionQue.front(),
        [&](Connection *pcon){
            std::unique_lock<std::mutex> lock(_queueMutex);
            pcon->refreshAliveTime();
            _connectionQue.pop();
        });
        _connectionQue.pop();
    }
}

void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		// 通过sleep模拟定时效果
		std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));

		// 扫描整个队列，释放多余的连接
		std::unique_lock<std::mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			Connection *p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; 
			}
			else
			{
				break; 
			}
		}
	}
}