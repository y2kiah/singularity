//	----==== SYSTEM.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	Stores the user's system information for reference by the engine
//----------------------------------------------------------------------------------------



#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "system.h"


/*------------------
---- STRUCTURES ----
------------------*/

////////// class SystemInfo //////////


void SystemInfo::querySystemInformation(void)
{
	MEMORYSTATUS	memStatus;
	OSVERSIONINFO	osVersionInfo;
	
	// read memory status
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatus(&memStatus);

	// read OS version
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	GetVersionEx(&osVersionInfo);

	// fill in data members
	totalMemory = memStatus.dwTotalPhys + memStatus.dwTotalPageFile;
	totalPhys = memStatus.dwTotalPhys;
	availMemory = memStatus.dwAvailPhys + memStatus.dwAvailPageFile;
	availPhys = memStatus.dwAvailPhys;


}