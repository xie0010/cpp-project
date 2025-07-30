#include "database_manager.hpp"
#include <fstream>
#include <ctime>
#include <stdexcept>
#include <iostream>
#define DICT_PATH "./dict.txt"

DatabaseManager::DatabaseManager(const string &usr_db_path, const string &dict_db_path)
{
    if(sqlite3_open(usr_db_path.c_str(), &usr_db_) != SQLITE_OK){
        throw runtime_error("用户数据库打开失败：" + string(sqlite3_errmsg(usr_db_)));
    }
    if(sqlite3_open(dict_db_path.c_str(), &dict_db_) != SQLITE_OK){
        throw runtime_error("单词数据库打开失败：" + string(sqlite3_errmsg(dict_db_)));
    }
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(usr_db_);
    sqlite3_close(dict_db_);
}

bool DatabaseManager::initalizeDatabase(){
    return initializeUserDB() && initializeDictDB();
}

bool DatabaseManager::initializeUserDB(){
    lock_guard<mutex> lock(usr_mutex_);

    const char* sql = "creat table if not exists usr("
                    "name text primary key,"
                    "passway int,"
                    "stage int);";

    if(!executeSQL(usr_db_, sql)){
        cerr << "用户数据初始化失败" << endl;
        return false;
    }

    sql = "update usr set stage=0;";
    if(!executeSQL(usr_db_, sql)){
        cerr << "用户状态重置失败" << endl;
        return false;
    }

    return true;
}

bool DatabaseManager::initializeDictDB(){
    lock_guard<mutex> lock(dict_mutex_);
    const char *sql = "create table if not exists dict(word text, mena text);";
    if(!executeSQL(dict_db_, sql)){
        cerr << "单词表创建失败" << endl;
        return false;
    }

    sql = "select count(*) from dict;";
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
        return false;
    }

    bool need_import = false;

    if(sqlite3_step(stmt) == SQLITE_ROW){
        if(sqlite3_column_int(stmt, 0) == 0){
            need_import = true;
        }
    }

    sqlite3_finalize(stmt);
    return need_import ? dictToDatabase() : true;
}

bool DatabaseManager::dictToDatabase(){
    ifstream file(DICT_PATH);
    if(!file){
        cerr << "无法打开词典文件：" << DICT_PATH << endl;
        return false;
    }

    string line;
    while(getline(file, line)){
        if(line.empty())continue;

        size_t pos = line.find(' ');
        if(pos == string::npos){
            cerr << "无效的词典条目（缺少空格分隔）" << line << endl;
            continue;
        }

        string word = line.substr(0, pos);
        string mean = line.substr(pos+1);

        const char* sql = "insert into dict values(?,?)";
        sqlite3_stmt *stmt = NULL;
        if(sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
            cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
            file.close();
            return false;
        }

        sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 1, mean.c_str(), -1, SQLITE_STATIC);

        if(sqlite3_step(stmt) != SQLITE_DONE){
            cerr << "数据插入失败" << sqlite3_errmsg(dict_db_) << endl;
            sqlite3_finalize(stmt);
            file.close();
            return false;
        }

        sqlite3_finalize(stmt);
    }

    file.close();

    cout << "单词库导入成功" << endl;
    return true;
}

bool DatabaseManager::executeSQL(sqlite3 *db, const string &sql, char **errmsg){
    if(sqlite3_exec(db, sql.c_str(), NULL, NULL, errmsg) != SQLITE_OK){
        cerr << "sql执行失败：" << *errmsg << endl;
        sqlite3_free(errmsg);
        return false;
    }

    return true;
}

bool DatabaseManager::registerUser(const string &name, const string &password){
    lock_guard<mutex> lock(usr_mutex_);
    const char *sql = "insert into usr values(?, ?, 0);";
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << endl;
        return false;
    } 

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if(result == SQLITE_CONSTRAINT){
        cerr << "该用户名已经存在：" << name << endl;
        return false;
    }
    return result == SQLITE_DONE;
}

bool DatabaseManager::loginUser(const string &name, const string &password, bool &is_online){
    lock_guard<mutex> lock(usr_mutex_);

    const char *sql = "select stage from usr where name=? and passwd=?";

    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);

    if(result == SQLITE_ROW){
        is_online = sqlite3_column_int(stmt, 0) == 1;

        if(!is_online){
            sqlite3_finalize(stmt);

            const char* update_sql = "update usr set stage=1 where name=?;";
            if(sqlite3_prepare_v2(usr_db_,update_sql, -1, &stmt, NULL) != SQLITE_OK){
                cerr << "sql准备失败：" << sqlite3_errmsg(usr_db_) << endl;
                return false;
            }
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
            result = sqlite3_step(stmt);
        }
    }

    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool DatabaseManager::logoutUser(const string &name){
    lock_guard<mutex> lock(usr_mutex_);
    const char *sql = "update usr set stage=0 where name=?;";
    
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << sqlite3_errmsg(dict_db_) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);

    return result = SQLITE_DONE;
}

bool DatabaseManager::querryWord(const string &word, string &meaning){
    const char *sql = "select mean from dict where word=?;";
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(dict_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_STATIC);

    bool found = false;

    if(sqlite3_step(stmt) == SQLITE_ROW){
        const unsigned char *result = sqlite3_column_text(stmt, 0);
        if(result){
            meaning = reinterpret_cast<const char*>(result);
            found = true;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::recordHistory(const string &name, const string &word, const string &meaning, const string &time){
    lock_guard<mutex> lock(usr_mutex_);

    const char *sql = "insert into history values(?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if(sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << sqlite3_errmsg(usr_db_) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, word.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, time.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE;
}

bool DatabaseManager::getHistory(const string name, string &history){
    lock_guard<mutex> lock(usr_mutex_);
    const char* sql = "select word,mean,time,from history where name=? order by time DESC;";
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(usr_db_, sql, -1, &stmt, NULL) != SQLITE_OK){
        cerr << "sql准备失败" << sqlite3_errmsg(usr_db_) <<endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    history.clear();
    while(sqlite3_step(stmt) == SQLITE_ROW){
        const unsigned char* word = sqlite3_column_text(stmt, 0);
        const unsigned char* mean = sqlite3_column_text(stmt, 1);
        const unsigned char* time = sqlite3_column_text(stmt, 2);
        
        if(word && mean && time){
            history += string(reinterpret_cast<const char*>(word)) + "\t";
            history += string(reinterpret_cast<const char*>(mean)) + "\t";
            history += string(reinterpret_cast<const char*>(time)) + "\t";
        }
    }

    sqlite3_finalize(stmt);
    return true;
}
