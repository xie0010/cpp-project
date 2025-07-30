#include <sqlite3.h>
#include <string>
#include <mutex>
#include <memory>
using namespace std;
class DatabaseManager
{
private:
    sqlite3 *usr_db_;
    sqlite3 *dict_db_;
    mutex usr_mutex_;
    mutex dict_mutex_;

    bool initializeUserDB();
    bool initializeDictDB();
    bool dictToDatabase(); 
    bool executeSQL(sqlite3 *db, const string &sql, char ** errmsg = NULL);
    
public:
    DatabaseManager(const string &usr_db_path, const string &dict_db_path);;
    ~DatabaseManager();

    bool initalizeDatabase();

    bool registerUser(const string &name, const string &password);
    bool loginUser(const string &name, const string &password, bool &is_online);
    bool logoutUser(const string &name);

    bool querryWord(const string &word, string &meaning);
    bool recordHistory(const string &name, const string &word, const string &meanig, const string &time);
    bool getHistory(const string name, string &history);
};

