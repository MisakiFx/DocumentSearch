#include "DataManager.h"
#include "ScanManager.h"
#include "Common.h"
//内存泄漏排查,在扫描模块中出现内存泄漏
//确定问题发生在数据库模块中和本地扫描模块中
//全部屏蔽没有再发生内存泄漏
void TestDirectoryList()
{
	while (1)
	{
		vector<string> dirs;
		vector<string> files;
		DirectoryList("D:\\Studyfile\\资料", dirs, files);
		for (const auto& e : dirs)
		{
			cout << e << endl;
		}
		for (const auto& e : files)
		{
			cout << e << endl;
		}
	}
}
void TestSqlite()
{
	SqliteManager sq;
	sq.Open("test.db");
	string createtb_sql = "create table tb_doc(id integer primary key autoincrement, doc_path text, doc_name text)";
	string insert_sql1 = "insert into tb_doc(doc_path, doc_name) values('D:stl', 'vector.h')";
	string insert_sql2 = "insert into tb_doc(doc_path, doc_name) values('D:stl', 'list.h')";
	string insert_sql3 = "insert into tb_doc(doc_path, doc_name) values('D:stl', 'map.h')";
	//sq.ExecuteSql(insert_sql1);
	//sq.ExecuteSql(insert_sql2);
	//sq.ExecuteSql(insert_sql3);
	string query_sql = "select * from tb_doc where doc_path = 'D:stl'";
	int row, col;
	char** ppRet;
	sq.GetTable(query_sql, row, col, ppRet);
	for (int i = 1; i <= row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			cout << ppRet[i * col + j] << " ";
		}
		cout << endl;
	}
	sqlite3_free_table(ppRet);
	AutoGetTable agt(sq, query_sql, row, col, ppRet);
	for (int i = 1; i <= row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			cout << ppRet[i * col + j] << " ";
		}
		cout << endl;
	}
}
void TestScanManager()
{
	//ScanManager	scanmgr;
	//scanmgr.Scan("D:\\Studyfile\\资料\\比特\\C语言课件V3");
}

void TestSearch()
{
	//搜索要搞成线程的
	//ScanManager	scanmgr;
	//scanmgr.Scan("D:\\Studyfile");
	ScanManager::CreateInstance();
	//DataManager datamgr;
	DataManager::GetInstance()->Init();
	string key;
	cout << "==============================================================================================================================" << endl;
	cout << "请输入要搜索的关键字:";
	while (std::cin >> key)
	{
		vector<std::pair<string, string>> docinfos;
		DataManager::GetInstance()->Search(key, docinfos);
		printf("%-50s %-50s\n", "名称", "路径");
		for (const auto& e : docinfos)
		{
			//cout << e.first << "  " << e.second << endl;
			//printf("%-50s %-50s\n", e.first.c_str(), e.second.c_str());
			//输出文件名，需要高亮分割处理
			string prefix, highlight, suffix;
			const string& name = e.first;
			const string& path = e.second;
			DataManager::GetInstance()->SplitHighlight(name, key, prefix, highlight, suffix);
			cout << prefix;
			ColorPrintf(highlight.c_str());
			cout << suffix;
			//补齐格式空格
			for (size_t i = name.size(); i <= 50; i++)
			{
				cout << " ";
			}
			printf("%-50s\n", path.c_str());//输出路径
		}
		cout << "==============================================================================================================================" << endl;
		cout << "请输入要搜索的关键字:";
	}
}
void TestPinyin()
{
	string allspell = ChineseConvertPinYinAllSpell("拼音比特科技 pinyin test 来试试");
	string initials = ChineseConvertPinYinInitials("拼音比特科技 pinyin test 来试试");
	cout << allspell << endl;
	cout << initials << endl;
}
void TestHighlight()
{
	//1.kay是什么，高亮key
	{
		//ColorPrintf("我绿了\n");
		string key = "古天乐";
		string str = "我古天乐绿了";
		size_t pos = str.find(key);
		string prefix, suffix;
		if (pos != string::npos)
		{
			prefix = str.substr(0, pos);
			suffix = str.substr(pos + key.size(), string::npos);
		}
		cout << prefix;
		ColorPrintf(key.c_str());
		cout << suffix << endl;
	}

	//2.kay是拼音，高亮对应的汉字
	{
		//ColorPrintf("我绿了\n");
		string key = "gutianle";
		string str = "我古天乐绿了";
		string prefix, suffix;
		string pinyin = ChineseConvertPinYinAllSpell(str);
	}

	//3.key是首字母，高亮对应汉字
	{
		//ColorPrintf("我绿了\n");
		string key = "gtl";
		string str = "我古天乐绿了";
		string prefix, suffix;
	}
}
int main()
{
	//TestDirectoryList();
	//TestSqlite();
	//TestScanManager();
	TestSearch();
	//ColourPrintf("Misaki");
	//TestPinyin();
	//TestHighlight();
}