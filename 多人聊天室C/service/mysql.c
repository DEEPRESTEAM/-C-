#include "my_sqlite.h"
char * msg = NULL;

//数据库初始化
sqlite3 * initsqlLite()
{
    // 首先打开数据库，没有就创建一个
    //  指向数据库的指针
    sqlite3 *db = NULL;
    int data = -1;
    char * sql;
    char * msg;
    // 打开指定的数据库文件，如果不存在将创建一个同名的数据库文件
    data = sqlite3_open("test.db", &db);
    return db;
}

//创建各个表
void creatTable(sqlite3 * db , char * sql){
    if (db == NULL || sql == NULL)
    {
        printf("db is null or sql is null");
        return;
    }
    
    int ret = 0;
    ret = sqlite3_exec(db , sql , 0 , 0 , &msg);
    if (ret != SQLITE_OK)
    {
        printf("create table error %s\n", msg);
        sqlite3_free(msg); // 释放掉azResult的内存空间
    }
}




//数据库增加数据
int addData(sqlite3 * db , char * sql){
    if (db == NULL || sql == NULL)
    {
        ERRLOG("db or sql error");
    }
    int ret = 0;
    ret = sqlite3_exec(db, sql, 0, 0, &msg);
    if (ret != SQLITE_OK)
    {
        printf("find error %s\n", msg);
        sqlite3_free(msg); // 释放掉azResult的内存空间
        return -1;
    }
    return 0;
}

//数据库修改数据
void changeData(sqlite3 * db , char * sql){
    if (db == NULL || sql == NULL)
    {
        ERRLOG("db or sql error");
    }
    int ret = 0;
    ret = sqlite3_exec(db , sql , 0 , 0  , &msg);
    if (ret != SQLITE_OK)
    {
        printf("updata error %s\n", msg);
        sqlite3_free(msg); // 释放掉azResult的内存空间
    }
    
}

//数据库删除数据
int deleteData(sqlite3 * db , char * sql){
    if (db == NULL || sql == NULL)
    {
        ERRLOG("db or sql error");
    }
    int ret = 0;
    ret = sqlite3_exec(db, sql, 0, 0, &msg);
    if (ret != SQLITE_OK)
    {
        printf("updata error %s\n", msg);
        sqlite3_free(msg); // 释放掉azResult的内存空间
        return -1;
    }
    return 0;
}

//数据库查找数据
dbData_t findData(sqlite3 * db , char * sql){
    if (db == NULL || sql == NULL)
    {
        ERRLOG("db or sql error");
    }
    dbData_t data;
    int ret = 0;
    ret = sqlite3_get_table(db, sql, &data.result, &data.row, &data.col, &msg);
    printf("%d\n" , ret);
    if (ret == SQLITE_OK)
    {   
        return data;
    }else{
        printf("find error %s\n" , msg);
        sqlite3_free(msg); // 释放掉azResult的内存空间
    }
}

//处理查询数据后获取所对应的字段属性在数字组中的位置
int getFinResult(int row , int colmn  , int postion){
    return (row * colmn + postion - 1);
}