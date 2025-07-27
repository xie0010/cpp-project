#include <stdio.h>      
#include <sqlite3.h>   
#include <string.h>     
#include <stdlib.h> 

int callback(void* arg, int cols, char** msgText, char** headText){
    if(*((int *)arg) == 0){
        for(int i = 0; i < cols; ++i){
            printf("%-10s\t",headText[i]);
        }
        printf("\n");
        printf("----------------------------------------\n");
        *((int *)arg) = 1;
    }

    for(int i = 0; i < cols; ++i){
        printf("%-10s\t", msgText[i] ? msgText[i] : "NULL");
    }
    printf("\n");

    return 0;
}

void do_add(sqlite3* ppDb){
    int add_num = 0;
    char add_name[20] = "";
    double add_score = 0;

    printf("请输入学生的学号：");
    scanf("%d", &add_num);

    printf("请输入学生的姓名：");
    scanf("%s", add_name);

    printf("请输入学生的成绩：");
    scanf("%lf", &add_score);
    
    getchar();
    
    char sql[128] = "";
    sprintf(sql,"INSERT INTO STUDENT VALUES(%d, '%s', %.2lf);", add_num, add_name, add_score);

    char *errMsg = NULL;
    if(sqlite3_exec(ppDb, sql, NULL, NULL, &errMsg) != SQLITE_OK){
        printf("添加信息失败，失败原因：%s\n", errMsg);
        sqlite3_free(errMsg);
        return;
    }
    printf("添加成功\n");
}

void do_show(sqlite3* ppDb){
    char sql[] = "SELECT * FROM STUDENT;";
    char *errmsg = NULL;
    int flag = 0;

    if(sqlite3_exec(ppDb, sql, callback, &flag, &errmsg) != SQLITE_OK){
        printf("查询失败，失败原因：%s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }

    if(flag == 0){
        printf("暂无学生信息\n");
    }
}

void do_delete(sqlite3* ppDb){
    int del_numb = 0;

    printf("请输入要删除的学生学号：");
    scanf("%d", &del_numb);
    getchar();

    char sql[128] = "";

    sprintf(sql,"DELETE FROM STUDENT WHERE numb = %d", del_numb);
    char *errmsg = NULL;

    if(sqlite3_exec(ppDb, sql, NULL, NULL, &errmsg) != SQLITE_OK){
        printf("删除失败，失败原因：%s\n", errmsg);
        sqlite3_free(errmsg);
        return;
    }
    if(sqlite3_changes(ppDb) > 0){
        printf("删除成功\n");
    }else{
        printf("未找到学号为%d的学生\n", del_numb);
    }
}

void do_update(sqlite3* ppDb){
    int update_num = 0;
    char update_name[20] = "";
    double update_score = 0;

    printf("请输入要修改的学生的学号：");
    scanf("%d", &update_num);

    printf("请输入要修改的学生的姓名：");
    scanf("%s", update_name);

    printf("请输入要修改的学生的成绩：");
    scanf("%lf", &update_score);
    
    getchar();

    char sql[128] = "";
    sprintf(sql,"UPDATE STUDENT SET name = '%s', score = %.2lf WHERE numb = %d;", 
    update_name, update_score, update_num);

    char *errmsg = NULL;

    if(sqlite3_exec(ppDb, sql, NULL, NULL, &errmsg) != SQLITE_OK){
        printf("修改失败，失败原因：%s\n", errmsg);  
        sqlite3_free(errmsg); 
        return;
    }

     if(sqlite3_changes(ppDb) > 0) {
        printf("修改成功\n");                                    
    } else {
        printf("未找到学号为 %d 的学生\n", update_num);         
    }
}

void do_search(sqlite3* ppDb) {
    int search_numb = 0;  
    
    printf("请输入要查找的学生学号：");
    scanf("%d", &search_numb);  
    getchar();                  
    
    char sql[128] = "";         
    sprintf(sql, "SELECT * FROM STUDENT WHERE numb = %d;", search_numb);
    
    char *errmsg = NULL;        
    int flag = 0;              
    
    if(sqlite3_exec(ppDb, sql, callback, &flag, &errmsg) != SQLITE_OK) {
        printf("查找失败，失败原因：%s\n", errmsg);  
        sqlite3_free(errmsg);                        
        return;                                      
    }
    
    if(flag == 0) {
        printf("未找到学号为 %d 的学生\n", search_numb);
    }
}

void init_database(sqlite3* ppDb){
    char *sql = "CREATE TABLE IF NOT EXISTS STUDENT("
                "numb INT PRIMARY KEY NOT NULL,"
                "name TEXT NOT NULL,"
                "score REAL NOT NULL);";

    char * errmsg = NULL;

    if(sqlite3_exec(ppDb, sql, NULL, NULL, &errmsg) != SQLITE_OK){
        printf("创建失败：%s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

void show_menu() {
    printf("\n========== 学生管理系统 ==========\n");  
    printf("1. 添加学生信息\n");                        
    printf("2. 显示所有学生信息\n");                    
    printf("3. 删除学生信息\n");                        
    printf("4. 修改学生信息\n");                        
    printf("5. 查找学生信息\n");                        
    printf("6. 退出系统\n");                            
    printf("==================================\n");    
    printf("请选择操作：");                             
}

int main(){
    sqlite3 *ppDb = NULL;
    int choice = 0;

    if(sqlite3_open("student.db", &ppDb) != SQLITE_OK) {
        printf("打开数据库失败: %s\n", sqlite3_errmsg(ppDb));
        return -1;
    }   

    printf("数据库连接成功!\n");

    init_database(ppDb);

    while(1){
        show_menu();
        scanf("%d", &choice);
        getchar();

        switch(choice){
            case 1:
                do_add(ppDb);
                break;
            case 2:
                do_show(ppDb);
                break;
            case 3:
                do_delete(ppDb);
                break;
            case 4:
                do_update(ppDb);
                break;
            case 5:
                do_search(ppDb);
                break;
            case 6:
                printf("感谢使用学生管理系统！\n");
                sqlite3_close(ppDb);
                return 0;
            default:
                printf("无效选择\n");
                break;
        }

        printf("按回车键继续...\n");
        getchar();
    }

    return 0;
}