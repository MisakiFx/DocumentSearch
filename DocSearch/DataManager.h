/*
 *	���ݴ���ģ��
 */
#pragma once
#include "Common.h"
/*
 *	��sqlite���ݿ��һ���װ������ʹ��
 */
class SqliteManager
{
public:
	//����
	SqliteManager()
		:_db(nullptr)
	{}
	//����
	~SqliteManager()
	{
		Close();
	}
	//��
	void Open(const string& path);
	//�ر�
	void Close();
	//ִ�����
	void ExecuteSql(const string& sql);
	//��ѯ����
	void GetTable(const string& sql, int& row, int& col, char**& ppRet);
	//���ÿ��������operator=���������
	SqliteManager(const SqliteManager&) = delete;
	SqliteManager& operator=(const SqliteManager&) = delete;
private:
	sqlite3* _db;//���ݿ����
};
/*
 *	ʹ��RAII���ܹ������ݿ��ѯ��������Ҫ�ֶ��ͷ��ڴ�
 */
class AutoGetTable
{
public:
	//���캯���Զ����ò�ѯ���ݺ���
	AutoGetTable(SqliteManager& _db, const string& sql, int& row, int& col, char**& ppRet)
	{
		_db.GetTable(sql, row, col, ppRet);
		_ppRet = ppRet;
	}
	//���������Զ������ͷ��ڴ�
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
 *	���ݿ����ݹ���ģ��
 */
#define DB_NAME "doc.db"
#define TB_NAME "tb_doc"
//Ϊ�˷����������Ƴɵ���ģʽ
class DataManager
{
public:
	static DataManager* GetInstance()
	{
		static DataManager datamgr;
		//һ������ǧ�����ظ���ʼ������ͣ�����ݿ�ᵼ���ڴ�й©
	//	datamgr.Init();
		return &datamgr;
	}
	//��ʼ�����ݿ⣬�������������ݿ�
	void Init();
	//�����ݿ��в���path�µ��������ĵ�
	void GetDoc(const string& path, std::set<string>& dbset);
	//�����ݿ��в����ļ�·�����ļ���
	void InsertDoc(const string& path, const string& name);
	//�����ݿ���ɾ��ָ���ļ����������Ϣ
	void DeleteDoc(const string& path, const string& name);
	//���ݹؼ��ֲ������ݿ��е��ļ���Ϣ
	void Search(const string& key, vector<std::pair<string, string>>& docinfos);
	//���ݽ������
	void SplitHighlight(const string& str, const string& key, string& prefix, string& highlight, string& suffix);
private:
	DataManager()
	{

	}
	SqliteManager _dbmgr;
	std::mutex _mtx;
};
