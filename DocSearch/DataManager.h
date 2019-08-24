/*
 *	数据处理模块
 */
#pragma once
#include "Common.h"
/*
 *	对sqlite数据库的一层封装，方便使用
 */
class SqliteManager
{
public:
	//构造
	SqliteManager()
		:_db(nullptr)
	{}
	//析构
	~SqliteManager()
	{
		Close();
	}
	//打开
	void Open(const string& path);
	//关闭
	void Close();
	//执行语句
	void ExecuteSql(const string& sql);
	//查询数据
	void GetTable(const string& sql, int& row, int& col, char**& ppRet);
	//禁用拷贝构造和operator=运算符重载
	SqliteManager(const SqliteManager&) = delete;
	SqliteManager& operator=(const SqliteManager&) = delete;
private:
	sqlite3* _db;//数据库对象
};
/*
 *	使用RAII智能管理数据库查询操作中需要手动释放内存
 */
class AutoGetTable
{
public:
	//构造函数自动调用查询数据函数
	AutoGetTable(SqliteManager& _db, const string& sql, int& row, int& col, char**& ppRet)
	{
		_db.GetTable(sql, row, col, ppRet);
		_ppRet = ppRet;
	}
	//析构函数自动调用释放内存
	~AutoGetTable()
	{
		TRACE_LOG("%p", _ppRet);
		sqlite3_free_table(_ppRet);
	}
	AutoGetTable(const AutoGetTable&) = delete;
	AutoGetTable operator=(const AutoGetTable&) = delete;
private:
	char** _ppRet;
};
/*
 *	数据库数据管理模块
 */
#define DB_NAME "doc.db"
#define TB_NAME "tb_doc"
//为了方便加锁，设计成单例模式
class DataManager
{
public:
	static DataManager* GetInstance()
	{
		static DataManager datamgr;
		//一定不能千万不能重复初始化，不停打开数据库会导致内存泄漏
	//	datamgr.Init();
		return &datamgr;
	}
	//初始化数据库，包括建表，打开数据库
	void Init();
	//在数据库中查找path下的所有子文档
	void GetDoc(const string& path, std::set<string>& dbset);
	//向数据库中插入文件路径及文件名
	void InsertDoc(const string& path, const string& name);
	//从数据库中删除指定文件及其相关信息
	void DeleteDoc(const string& path, const string& name);
	//根据关键字查找数据库中的文件信息
	void Search(const string& key, vector<std::pair<string, string>>& docinfos);
	//根据结果高亮
	void SplitHighlight(const string& str, const string& key, string& prefix, string& highlight, string& suffix);
private:
	DataManager()
	{

	}
	SqliteManager _dbmgr;
	std::mutex _mtx;
};
