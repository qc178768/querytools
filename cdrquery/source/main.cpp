#include "../include/cdrquery.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string>


int main(int argc,char* argv[])
{
	CdrQuery g_queryobj;
	g_queryobj.CmdOpt(argc,argv);
	g_queryobj.DirorFile();

	return 0;
}

