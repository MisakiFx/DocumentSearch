/*
 *	ɨ��ģ��
 *	������߳�ʵ��
 */
#pragma once
#include "Common.h"
#include "DataManager.h"


//��ɨ��ģ��дΪ����ģʽ����Ϊȫ��ֻ����һ��ɨ��ģ�飬���Ϊ����ģʽ
class ScanManager
{
public:
	void Scan(const string& path);
	//�����̲߳���ִ��ɨ��
	void StartScan()
	{
		while (1)
		{
			Scan("D:\\Studyfile\\����\\����\\C���Կμ�V3");
			//Scan("D:\\Studyfile");
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		//Scan("D:\\Studyfile\\����\\����\\C���Կμ�V3");
	}
	static ScanManager* CreateInstance()
	{
		static ScanManager scanmgr;
		//��Ա������ָ�����ȡһ�ε�ַ����������Ҫ�����Ķ����ָ��
		static std::thread thd(&StartScan, &scanmgr);
		//thd.detach();
		//scanmgr.Scan("D:\\Studyfile\\����\\����\\C���Կμ�V3");
		return &scanmgr;
	}
private:
	ScanManager()
	{
		//_datamgr.Init();
	}
	ScanManager(const ScanManager&)
	{

	}
	//DataManager _datamgr;
}; 

