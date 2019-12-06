//	----==== SYSTEM.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	Stores the user's system information for reference by the engine
//----------------------------------------------------------------------------------------



#ifndef SYSTEM_H
#define SYSTEM_H

#include "typedefs.h"
#include "singleton.h"

/*---------------
---- DEFINES ----
---------------*/

#define sInfo		SystemInfo::getInstance()	// used to access the SystemInfo instance globally
#define system		SystemInfo::getInstance()


/*------------------
---- STRUCTURES ----
------------------*/

enum WINDOWS_VERSION {
	UNKNOWN	= 0,
	WINDOWS_95,
	WINDOWS_95_SR2,
	WINDOWS_98,
	WINDOWS_98_SR2,
	WINDOWS_ME,
	WINDOWS_NT,
	WINDOWS_2K,
	WINDOWS_XP,
	WINDOWS_FUTURE
};


struct OS_VERSION {
	uint32	majorVersion;
	uint32	minorVersion;
	uint32	build;
};


enum CPU_FLAGS {
	SUPPORTS_MMX	= 23,	// processor supports MMX extensions
	SUPPORTS_SSE	= 25,	// processor supports Intel SSE extensions
	SUPPORTS_SSE2	= 26,	// processor supports Intel SSE2 extensions
};


class SystemInfo : public Singleton<SystemInfo> {
	private:

		///// Attributes
		u32Flags		cpuFlags;
		int				platform;
		OS_VERSION		osVersion;
		uint32			totalMemory;
		uint32			totalPhys;
		uint32			availMemory;
		uint32			availPhys;
		uint32			processorSpeed;

		///// Mutators
		void			querySystemInformation();
		bool			verifyCPUID();
		void			verifySSE();
		void			verifySSE2();
		void			readCPUFlags();
		void			computeProcessorSpeed();

	public:

		///// Accessors
		u32Flags			getCPUFlags();
		int					getPlatform();
		const OS_VERSION &	getOSVersion();
		uint32				getTotalMemory();
		uint32				getPhysicalMemory();
		uint32				getAvailableMemory();

		void				readCPUCounter(uint64 * pCounter);

		///// Constructors / Destructor
		explicit SystemInfo() : Singleton<SystemInfo>(*this) {}
	    ~SystemInfo() {}
};

#endif