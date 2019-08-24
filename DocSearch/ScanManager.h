/*
 *	扫描模块
 *	最后用线程实现
 */
#pragma once
#include "Common.h"
#include "DataManager.h"


//将扫描模块写为单例模式，因为全局只能有一个扫描模块，设计为单例模式
class ScanManager
{
public:
	void Scan(const string& path);
	//利用线程不断执行扫描
	void StartScan()
	{
		while (1)
		{
			Scan("D:\\Studyfile\\资料\\比特\\C语言课件V3");
			//Scan("D:\\Studyfile");
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		//Scan("D:\\Studyfile\\资料\\比特\\C语言课件V3");
	}
	static ScanManager* CreateInstance()
	{
		static ScanManager scanmgr;
		//成员函数的指针得再取一次地址，传入我们要调它的对象的指针
		static std::thread thd(&StartScan, &scanmgr);
		//thd.detach();
		//scanmgr.Scan("D:\\Studyfile\\资料\\比特\\C语言课件V3");
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

