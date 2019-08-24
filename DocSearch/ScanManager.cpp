#include "ScanManager.h"
void ScanManager::Scan(const string& path)
{
	//比对文件系统和数据库
	vector<string> localdirs;
	vector<string> localfiles;
	//拿到本地数据
	DirectoryList(path, localdirs, localfiles);
	std::set<string> localset;
	localset.insert(localfiles.begin(), localfiles.end());
	localset.insert(localdirs.begin(), localdirs.end());
	//拿到数据库信息
	std::set<string> dbset;
	DataManager::GetInstance()->GetDoc(path, dbset);

	//比对算法判断谁有谁没有
	//本地有数据库没有，数据库新增，
	//本地没有数据库有，数据库删除
	//本地先结束，删除数据库后面所有信息
	//数据库先结束，数据库新增本地后面所有信息
	auto localit = localset.begin();
	auto dbit = dbset.begin();
	//遍历
	while (localit != localset.end() && dbit != dbset.end())
	{
		//
		if (*localit < *dbit)
		{
			//数据库新增数据
			DataManager::GetInstance()->InsertDoc(path, *localit);
			++localit;
		}
		else if (*dbit < *localit)
		{
			//数据库删除数据
			DataManager::GetInstance()->DeleteDoc(path, *dbit);
			++dbit;
		}
		else
		{
			++dbit;
			++localit;
		}
	}
	//判断结束
	while (localit != localset.end())
	{
		//新增数据
		DataManager::GetInstance()->InsertDoc(path, *localit);
		++localit;
	}
	while (dbit != dbset.end())
	{
		//删除数据
		DataManager::GetInstance()->DeleteDoc(path, *dbit);
		++dbit;
	}
	//递归遍历当前目录下所有目录
	for (const auto& subdirs : localdirs)
	{
		string subpath = path;
		subpath += '\\';
		subpath += subdirs;
		Scan(subpath);
	}
}
