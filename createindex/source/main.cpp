#include "../include/createindex.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string>


int main(int argc,char* argv[])
{
	//displayProcessTimes("main start:\n");
	CdrQuery g_queryobj;
	g_queryobj.CmdOpt(argc,argv);
	g_queryobj.ProDir(argv);

	return 0;
}

