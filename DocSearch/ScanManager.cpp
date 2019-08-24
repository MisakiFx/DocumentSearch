#include "ScanManager.h"
void ScanManager::Scan(const string& path)
{
	//�ȶ��ļ�ϵͳ�����ݿ�
	vector<string> localdirs;
	vector<string> localfiles;
	//�õ���������
	DirectoryList(path, localdirs, localfiles);
	std::set<string> localset;
	localset.insert(localfiles.begin(), localfiles.end());
	localset.insert(localdirs.begin(), localdirs.end());
	//�õ����ݿ���Ϣ
	std::set<string> dbset;
	DataManager::GetInstance()->GetDoc(path, dbset);

	//�ȶ��㷨�ж�˭��˭û��
	//���������ݿ�û�У����ݿ�������
	//����û�����ݿ��У����ݿ�ɾ��
	//�����Ƚ�����ɾ�����ݿ����������Ϣ
	//���ݿ��Ƚ��������ݿ��������غ���������Ϣ
	auto localit = localset.begin();
	auto dbit = dbset.begin();
	//����
	while (localit != localset.end() && dbit != dbset.end())
	{
		//
		if (*localit < *dbit)
		{
			//���ݿ���������
			DataManager::GetInstance()->InsertDoc(path, *localit);
			++localit;
		}
		else if (*dbit < *localit)
		{
			//���ݿ�ɾ������
			DataManager::GetInstance()->DeleteDoc(path, *dbit);
			++dbit;
		}
		else
		{
			++dbit;
			++localit;
		}
	}
	//�жϽ���
	while (localit != localset.end())
	{
		//��������
		DataManager::GetInstance()->InsertDoc(path, *localit);
		++localit;
	}
	while (dbit != dbset.end())
	{
		//ɾ������
		DataManager::GetInstance()->DeleteDoc(path, *dbit);
		++dbit;
	}
	//�ݹ������ǰĿ¼������Ŀ¼
	for (const auto& subdirs : localdirs)
	{
		string subpath = path;
		subpath += '\\';
		subpath += subdirs;
		Scan(subpath);
	}
}
