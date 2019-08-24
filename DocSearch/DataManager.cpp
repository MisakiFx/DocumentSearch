#include "DataManager.h"
void SqliteManager::Open(const string& path)
{
	int ret = sqlite3_open(path.c_str(), &_db);
	//��ʧ�ܵĻ�
	if (ret != SQLITE_OK)
	{
		ERROR_LOG("sqlite3_open\n");
	}
	else
	{
		TRACE_LOG("sqlite3_open success\n");
	}
}
void SqliteManager::Close()
{
	assert(_db);
	int ret = sqlite3_close(_db);
	if (ret != SQLITE_OK)
	{
		ERROR_LOG("sqlite3_close\n");
	}
	else
	{
		TRACE_LOG("sqlite3_close success\n");
	}
}
void SqliteManager::ExecuteSql(const string& sql)
{
	assert(_db);
	char* errmsg;
	int ret = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errmsg);
	if (ret != SQLITE_OK)
	{
		ERROR_LOG("sqlite3_exec(%s) errmsg:%s\n", sql.c_str(), errmsg);
		sqlite3_free(errmsg);
	}
	else
	{
		TRACE_LOG("sqlite3_exec(%s) success\n", sql.c_str());
	}
}
void SqliteManager::GetTable(const string& sql, int& row, int& col, char**& ppRet)
{
	//Ҫ�����ݿ����Ϊ��
	assert(_db);
	char* errmsg;
	int ret = sqlite3_get_table(_db, sql.c_str(), &ppRet, &row, &col, &errmsg);
	if (ret != SQLITE_OK)
	{
		ERROR_LOG("sqlite3_get_table(%s) errmsg:%s\n", sql.c_str(), errmsg);
		sqlite3_free(errmsg);
	}
	else
	{
		TRACE_LOG("sqlite3_get_table(%s) success\n", sql.c_str());
	}
}
void DataManager::Init()
{
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.Open(DB_NAME);
	char sql[256];
	sprintf(sql, "create table if not exists %s (id integer primary key, path text, name text, name_pinyin text, name_initials text)", TB_NAME);
	_dbmgr.ExecuteSql(sql);
}
void DataManager::GetDoc(const string& path, std::set<string>& dbset)
{
	char sql[256];
	sprintf(sql, "select name from %s where path = '%s'", TB_NAME, path.c_str());
	int row = 0, col = 0;
	char** ppRet;
	//����������������
	std::unique_lock<std::mutex> lock(_mtx);
	AutoGetTable agt(_dbmgr, sql, row, col, ppRet);
	lock.unlock();
	for (int i = 1; i <= row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			dbset.insert(ppRet[i * col + j]);
		}
	}
}
void DataManager::InsertDoc(const string& path, const string& name)
{
	char sql[256];
	string pinyin = ChineseConvertPinYinAllSpell(name);
	string initials = ChineseConvertPinYinInitials(name);
	sprintf(sql, "insert into %s(path, name, name_pinyin, name_initials) values('%s', '%s', '%s', '%s')", TB_NAME, path.c_str(), name.c_str(), pinyin.c_str(), initials.c_str());
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(sql);
}
void DataManager::DeleteDoc(const string& path, const string& name)
{
	//������ɾ��
	//1����ɾ����·����Ŀ���ļ�/�ļ���
	char sql[256];
	sprintf(sql, "delete from %s where path = '%s' and name = '%s'", TB_NAME, path.c_str(), name.c_str());
	_dbmgr.ExecuteSql(sql);
	//2�����ɾ���������ļ��л�Ҫ����ģ��ƥ�����ɾ����ɾ���ļ����µ������ļ������ļ���
	string path_ = path;
	path_ += '\\';
	path_ += name;
	sprintf(sql, "delete from %s where path like '%%%s%%'", TB_NAME, path_.c_str());
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(sql);
}
void DataManager::Search(const string& key, vector<std::pair<string, string>>& docinfos)
{
	//����������������������
	//char sql[256];
	//sprintf(sql, "select name, path from %s where name like '%%%s%%'", TB_NAME, key.c_str());
	//int row, col;
	//char** ppRet;
	//AutoGetTable agt(_dbmgr, sql, row, col, ppRet);
	//for (int i = 1; i <= row; i++)
	//{
	//	docinfos.push_back(std::make_pair(ppRet[i * col + 0], ppRet[i * col + 1]));
	//}

	{
		//Ϊ�˽��sqlite���ı��벻ͬ�������ǽ������ַ���ѯͳһ��ƴ�����в�ѯ
		char sql[256];
		string pinyin = ChineseConvertPinYinAllSpell(key);
		string initials = ChineseConvertPinYinInitials(key);
		sprintf(sql, "select name, path from %s where name_pinyin like '%%%s%%' or name_initials like '%%%s%%'", TB_NAME, pinyin.c_str(), initials.c_str());
		int row, col;
		char** ppRet;
		std::unique_lock<std::mutex> lock(_mtx);
		AutoGetTable agt(_dbmgr, sql, row, col, ppRet);
		lock.unlock();
		for (int i = 1; i <= row; i++)
		{
			docinfos.push_back(std::make_pair(ppRet[i * col + 0], ppRet[i * col + 1]));
		}
	}

	//����ĸ��������(�ҵĸĽ���,ȡ������ת����ĸ�ٽ�������)
	//{
	//	char sql[256];
	//	string pinyin = ChineseConvertPinYinAllSpell(key);
	//	//string initials = ChineseConvertPinYinInitials(key);
	//	sprintf(sql, "select name, path from %s where name_pinyin like '%%%s%%' or name_initials like '%%%s%%'", TB_NAME, pinyin.c_str(), key.c_str());
	//	int row, col;
	//	char** ppRet;
	//	AutoGetTable agt(_dbmgr, sql, row, col, ppRet);
	//	for (int i = 1; i <= row; i++)
	//	{
	//		docinfos.push_back(std::make_pair(ppRet[i * col + 0], ppRet[i * col + 1]));
	//	}
	//}
}

void DataManager::SplitHighlight(const string& str, const string& key, string& prefix, string& highlight, string& suffix)
{
	//1.key��ԭ�����ִ�
	//��һ���汾��ƥ�䲻�Ϸ���ԭ��
	//{
	//	size_t ht_index = str.find(key);
	//	if (ht_index == string::npos)
	//	{
	//		prefix = str;
	//		TRACE_LOG("Highlight no match\n");
	//		return;
	//	}
	//	prefix = str.substr(0, ht_index);
	//	highlight = key;
	//	suffix = str.substr(ht_index + key.size(), string::npos);
	//}
	{
		size_t ht_start = str.find(key);
		if (ht_start != string::npos)
		{
			prefix = str.substr(0, ht_start);
			highlight = key;
			suffix = str.substr(ht_start + key.size(), string::npos);
			return;
		}
	}
	//2.key��ƴ��ȫƴ���߻���ԭ������ԭ���еĺ���
	{
		string str_ap = ChineseConvertPinYinAllSpell(str);
		string key_ap = ChineseConvertPinYinAllSpell(key);
		size_t ht_index = 0;//ht_index��������ԭ�ַ���
		size_t ap_index = 0;//ap_index��������ԭ�ַ�����ƴ��
		size_t ht_start = 0, ht_len = 0;//ht_start������¼����λ����ԭ���е��±�,ht_len������¼����������ԭ���еĳ���
		size_t ap_start = str_ap.find(key_ap);//��ԭ��ƴ�����ҵ�key����ΪkeyҲ��һ��ȫƴ
		if (ap_start != string::npos)
		{
			size_t ap_end = ap_start + key_ap.size();//��ƴ���и����ַ��������±�
			//ͨ��ѭ���ж�ǰ׺��ԭ���н������±�͸���������ԭ���н������±�
			while (ap_index < ap_end)
			{
				
				if (ap_index == ap_start)
				{
					//��ʱԭ�����±��Ѿ�����������Ҫ������λ�ã���˼�¼�±�
					ht_start = ht_index;
				}
				//����Ǹ�ascii�ַ�����ֱ������
				if (str[ht_index] >= 0 && str[ht_index] <= 127)
				{
					++ht_index;
					++ap_index;
				}
				else//�Ǻ���
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[ht_index];
					chinese[1] = str[ht_index + 1];
					string ap_str = ChineseConvertPinYinAllSpell(chinese);
					ht_index += 2;//gbk�����������ַ�ֱ������
					ap_index += ap_str.size();//ƴ���������������������ƴ���м����ַ�
				}
			}
			//ѭ������ap_index == ap_end��ʱht_indexָ��ԭ������Ҫ�����Ľ�βλ��
			//��˿��Լ�����������򳤶�ht_len
			ht_len = ht_index - ht_start;
			prefix = str.substr(0, ht_start);
			highlight = str.substr(ht_start, ht_len);
			suffix = str.substr(ht_start + ht_len, string::npos);
			return;
		}
	}
	//3.key��ƴ������ĸ�ӻ��ԭ��
	{
		string init_str = ChineseConvertPinYinInitials(str);
		string init_key = ChineseConvertPinYinInitials(key);
		size_t init_start = init_str.find(init_key);
		if (init_start != string::npos)
		{
			size_t init_end = init_start + init_key.size();
			size_t init_index = 0, ht_index = 0;
			size_t ht_start = 0, ht_len = 0;
			while (init_index < init_end)
			{
				//ƥ�����ˣ�ԭ���п�ʼ�����±�
				if (init_index == init_start)
				{
					ht_start = ht_index;
				}
				if (str[ht_index] >= 0 && str[ht_index] <= 127)//ascii�ַ�
				{
					ht_index++;
					init_index++;
				}
				else//����
				{
					ht_index += 2;
					init_index++;
				}
			}
			ht_len = ht_index - ht_start;
			prefix = str.substr(0, ht_start);
			highlight = str.substr(ht_start, ht_len);
			suffix = str.substr(ht_start + ht_len, string::npos);
			return;
		}
	}
	TRACE_LOG("split highlight no match. str:%s, key:%s\n", str.c_str(), key.c_str());
	prefix = str;
}
