#ifndef _MY_SQLITE_H
#define _MY_SQLITE_H

#include<head.h>
#include<sqlite3.h>


//用于存储查找的内容
typedef struct dbData{
    int row;
    int col;
    char **result;
}dbData_t;


//数据库初始化
sqlite3 *initsqlLite();

// 创建各个表
void creatTable(sqlite3 *db, char *sql);

// 数据库增加数据 
int addData(sqlite3 *db, char *sql);

// 数据库修改数据
void changeData(sqlite3 *db, char *sql);

// 数据库删除数据
int deleteData(sqlite3 *db, char *sql);

// 数据库查找数据
dbData_t findData(sqlite3 *db, char *sql);

// 处理查询数据后获取所对应的字段属性在数字组中的位置
int getFinResult(int row, int colmn, int postion);

#endif