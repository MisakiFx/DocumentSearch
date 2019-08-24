#include "DataManager.h"
void SqliteManager::Open(const string& path)
{
	int ret = sqlite3_open(path.c_str(), &_db);
	//打开失败的话
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
	//要求数据库对象不为空
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
	//用智能锁来管理锁
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
	//分两步删除
	//1、先删除本路径下目标文件/文件夹
	char sql[256];
	sprintf(sql, "delete from %s where path = '%s' and name = '%s'", TB_NAME, path.c_str(), name.c_str());
	_dbmgr.ExecuteSql(sql);
	//2、如果删除对象是文件夹还要利用模糊匹配查找删除被删除文件夹下的所有文件包括文件夹
	string path_ = path;
	path_ += '\\';
	path_ += name;
	sprintf(sql, "delete from %s where path like '%%%s%%'", TB_NAME, path_.c_str());
	std::unique_lock<std::mutex> lock(_mtx);
	_dbmgr.ExecuteSql(sql);
}
void DataManager::Search(const string& key, vector<std::pair<string, string>>& docinfos)
{
	//用中文搜索导致乱码问题
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
		//为了解决sqlite中文编码不同问题我们将中文字符查询统一用拼音进行查询
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

	//首字母进行搜索(我的改进版,取消文字转首字母再进行搜索)
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
	//1.key是原串的字串
	//第一个版本，匹配不上返回原串
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
	//2.key是拼音全拼或者混杂原串或者原串中的汉字
	{
		string str_ap = ChineseConvertPinYinAllSpell(str);
		string key_ap = ChineseConvertPinYinAllSpell(key);
		size_t ht_index = 0;//ht_index用来遍历原字符串
		size_t ap_index = 0;//ap_index用来遍历原字符串的拼音
		size_t ht_start = 0, ht_len = 0;//ht_start用来记录高亮位置在原串中的下标,ht_len用来记录高亮部分在原串中的长度
		size_t ap_start = str_ap.find(key_ap);//在原串拼音中找到key，因为key也是一个全拼
		if (ap_start != string::npos)
		{
			size_t ap_end = ap_start + key_ap.size();//在拼音中高亮字符结束的下标
			//通过循环判断前缀在原串中结束的下标和高亮部分在原串中结束的下标
			while (ap_index < ap_end)
			{
				
				if (ap_index == ap_start)
				{
					//此时原串的下标已经遍历到了需要高亮的位置，因此记录下标
					ht_start = ht_index;
				}
				//如果是个ascii字符，则直接跳过
				if (str[ht_index] >= 0 && str[ht_index] <= 127)
				{
					++ht_index;
					++ap_index;
				}
				else//是汉字
				{
					char chinese[3] = { '\0' };
					chinese[0] = str[ht_index];
					chinese[1] = str[ht_index + 1];
					string ap_str = ChineseConvertPinYinAllSpell(chinese);
					ht_index += 2;//gbk汉字是两个字符直接跳过
					ap_index += ap_str.size();//拼音跳几个决定于这个汉字拼音有几个字符
				}
			}
			//循环结束ap_index == ap_end此时ht_index指向原串中需要高亮的结尾位置
			//因此可以计算出高亮区域长度ht_len
			ht_len = ht_index - ht_start;
			prefix = str.substr(0, ht_start);
			highlight = str.substr(ht_start, ht_len);
			suffix = str.substr(ht_start + ht_len, string::npos);
			return;
		}
	}
	//3.key是拼音首字母加混合原串
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
				//匹配上了，原串中开始高亮下标
				if (init_index == init_start)
				{
					ht_start = ht_index;
				}
				if (str[ht_index] >= 0 && str[ht_index] <= 127)//ascii字符
				{
					ht_index++;
					init_index++;
				}
				else//汉字
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
