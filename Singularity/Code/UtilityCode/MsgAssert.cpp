//	----==== MSGASSERT.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			6/06
//	Description:	A function to replace the standard assert macro
//----------------------------------------------------------------------------------------

#include <fstream>
#include <stdlib.h>

void _msgAssert(bool condition, const char *msg)
{
	if (condition) {
		return;
	} else {
		std::ofstream outFile;

		outFile.open("msgAssert.txt", std::ios_base::trunc | std::ios_base::out);
		if (outFile.is_open()) {
			outFile << msg;
			outFile.close();
		}

		exit(1);
	}
}