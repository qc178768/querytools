#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "../include/createindex.h"
#include <unistd.h> //chdir
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>


extern char FileSuffix[BUFFSIZE];
const char* StringFind(char *src, const char *dst);

//-------------------------------------------
// 获取processed.r文件里的数据转换为 string
std::string GetProcessedString() 
{
	FILE *fp = fopen(INDEX_FILE_NAME, "r");
	std::string  temp;
	if(fp == NULL)
	{
		return temp;
	}
	else
	{
		char buf[BUFFSIZE];
		
		while (!feof(fp))
		{
			memset(buf, '\0', BUFFSIZE);
			fgets(buf, BUFFSIZE, fp);
			temp += buf;
		}
		fclose(fp);
		return temp;
	}
}

// 以一定条件分割 string 装入数组
std::vector<std::string> GetProcessedList(std::string temp, const char ch)
{
	std::vector<std::string> get_file;
	size_t find = 0;
	size_t length = 0;
	length = temp.length();
	while (length > 0)
	{
		find = temp.find_first_of(ch);
		if(find != std::string::npos)
		{
			get_file.push_back(temp.substr(0, find));
			length -= (find+1);
			if(length > 0)
			{
				temp = temp.substr(find+1, length);
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return get_file;
}

void usage(const char *prog)
{
	fprintf(stderr, 
			"Usage: %s -d dirpath\n",
			prog);
	exit(EXIT_SUCCESS);
}

unsigned char Mkdir(const char *path, mode_t mode)
{
	unsigned char  flag= 0;
	if (access(path, F_OK) == -1) 
	{
		if(strcmp(path, CALLING_PATH_NAME) == 0)
		{
			flag = CALLINGDIR_FLAG;
		}
		else if(strcmp(path, CALLED_PATH_NAME ) == 0)
		{
			flag = CALLEDDIR_FLAG;
		}
		else if(strcmp(path, TIME_PATH_NAME ) == 0)
		{
			flag = TIMEDIR_FLAG;
		}
		int ret = mkdir(path, mode);
		if (ret != 0)
		{
			ERR_EXIT("mkdir failed\n");		
		}
	}
	return flag;
}


void Chdir(const char *path)
{
	int ret = 0;
	ret = chdir(path);
	if (ret != 0)
	{
		ERR_EXIT("chdir failed\n");		
	}
}

unsigned char MkIndexDirs()   //在查询目录下(如果不存在)建立索引目录
{
	unsigned char flag= 0;    
	flag |= Mkdir(CALLING_PATH_NAME, 0777);
	flag |= Mkdir(CALLED_PATH_NAME, 0777);
	flag |= Mkdir(TIME_PATH_NAME,0777);
	Mkdir(LOG_PATH_NAME,0777);
	return flag;
	
}

int is_ProcessDir(const char* dirname)
{
	if(strcmp(dirname, CALLING_PATH_NAME) == 0 || strcmp(dirname, CALLED_PATH_NAME) == 0 
		|| strcmp(dirname, LOG_PATH_NAME) == 0  || strcmp(dirname, TIME_PATH_NAME))
	{
		return FALSE;
	}
	return TRUE;
}


//-------------------------------------------


RecordBrief::RecordBrief()
{
	memset(startTime,'\0',sizeof(startTime));
	memset(endTime,'\0',sizeof(endTime));

}

RecordBrief::~RecordBrief()
{
	
}

void RecordBrief::SetIndex(int index)
{
	this->nIndex = index;
	return ;
	
}
void RecordBrief::Setfilename(const char* filename)
{
	if(filename!= NULL)
	{
		this->filename = filename;
	}
	else
	{
		printf("filename is null");
	}
	return;
}

void RecordBrief::SetRecordType(const std::string& type)
{
	if(!type.empty())
		this->recordType= type;
	return ;
}	

void RecordBrief::SetCallingParty(char* calling)
{
	if(calling != NULL)
		this->callingParty = calling;
	return ;
}

void RecordBrief::SetCalledParty(char * called)
{
	if(called != NULL)
		this->calledParty = called;
	return ;
}


void RecordBrief::SetStartTime(char* starttime)
{
	if(starttime != NULL)
	{
		memcpy(startTime,starttime,sizeof(startTime));
	
	}
	return ;
}

void RecordBrief::SetEndTime(char* endtime)
{
	if(endtime != NULL)
	{
		memcpy(endTime,endtime,sizeof(endTime));
	}
	return ;
}


void RecordBrief::SetCallDuration(const std::string& duration)
{
	if(!duration.empty())
		this->callduration= duration;
	return ;
	
}

std::string	RecordBrief::GetCallDuration() const
{
	return callduration;
}

void RecordBrief::SetLocalRecordSeq(char* lseq)
{
	if(lseq != NULL)
		this->localRecordSeq= lseq;
	return ;
}

void RecordBrief::SetRecordSeq(char* rseq)
{
	if(rseq != NULL)
		this->recordSeq= rseq;
	return ;
}

ASN1OCTET* RecordBrief::GetStartTime() const
{
	return (ASN1OCTET*)startTime;
}
ASN1OCTET* RecordBrief::GetEndTime()const
{
	return (ASN1OCTET*)endTime;
}

std::string RecordBrief::GetCallingParty() const
{
	return callingParty;
}
std::string RecordBrief::GetCalledParty() const
{
	return calledParty;
}

time_t RecordBrief::StringToDatetime(string str)
{
    char *cha = (char*)str.data();             
    tm tm_;                                    
    int year, month, day, hour, minute, second;
    sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    tm_.tm_year = year - 1900;                 
    tm_.tm_mon = month - 1;                    
    tm_.tm_mday = day;                        
    tm_.tm_hour = hour;                        
    tm_.tm_min = minute;                       
    tm_.tm_sec = second;                      
    tm_.tm_isdst = 0;                         
    time_t t_ = mktime(&tm_);                  
    return t_;                                 
}

string RecordBrief::DatetimeToString(time_t time)
{
    tm *tm_ = localtime(&time);                
    int year, month, day, hour, minute, second;
    year = tm_->tm_year + 1900;                
    month = tm_->tm_mon + 1;                   
    day = tm_->tm_mday;                        
    hour = tm_->tm_hour;                      
    minute = tm_->tm_min;                      
    second = tm_->tm_sec;                     
    char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];
    sprintf(yearStr, "%d", year);              
    sprintf(monthStr, "%d", month);           
    sprintf(dayStr, "%d", day);                
    sprintf(hourStr, "%d", hour);              
    sprintf(minuteStr, "%d", minute);          
    if (minuteStr[1] == '\0')                 
    {
        minuteStr[2] = '\0';
        minuteStr[1] = minuteStr[0];
        minuteStr[0] = '0';
    }
    sprintf(secondStr, "%d", second);         
    if (secondStr[1] == '\0')                  
    {
        secondStr[2] = '\0';
        secondStr[1] = secondStr[0];
        secondStr[0] = '0';
    }
    char s[20];                               
    sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);
    string str(s);                             
    return str;                                
}



void RecordBrief::ParseBCDTimeStamp(const ASN1OCTET* src,char* dest) const
{
	struct tm timetmp;
	if(src[0] == '\0')
	{
		return;
	}
	timetmp.tm_year = DEBCD(src[0]);
	timetmp.tm_mon  = DEBCD(src[1]);
	timetmp.tm_mday = DEBCD(src[2]);
	timetmp.tm_hour = DEBCD(src[3]);
	timetmp.tm_min  = DEBCD(src[4]);
	timetmp.tm_sec  = DEBCD(src[5]);
	
	sprintf(dest,"%04d-%02d-%02d %02d:%02d:%02d",timetmp.tm_year+2000,timetmp.tm_mon,timetmp.tm_mday, \
		  timetmp.tm_hour,timetmp.tm_min,timetmp.tm_sec);
	return ;
}

void RecordBrief::GetInfoFile(const char* path, std::string& indexstr , std::string& filename) const	
{
	FILE *fp = fopen(path, "r");
	if(fp == NULL)
	{
		return;
	}

	/*
	int fd = fileno(fp);
	int ret;
	// 加读锁
	ret = lock_file_read(fd);
	if (ret == -1) {
		ERR_EXIT("lock_file_write\n");
	}
	fflush(fp);
	*/
	
	
	char line[SMALLBUFFSIZE]={0};
	int i = 1;
	char*temp;
	while(!feof(fp))  
	{
		fgets(line,sizeof(line),fp); 
		if(feof(fp))	//line保存最后一行信息
		{
			temp = strtok(line,",");    //strtok会修改原line的内容','->'\0'
			if(temp && (i==1))			// 定位到FilePath
			{
			   filename = temp;	
			}
	    	while(temp && (i<2))  
	  		{
	       	 	temp = strtok(NULL,",");
				++i;
	   		}
			if( temp && (i == 2))	// 定位到RecordNo
			{
				indexstr = temp;	  
				break;
			}
		}   
				
	}

	/*

	fflush(fp);
	ret = unlock_file(fd);
	if (ret == -1) {
		ERR_EXIT("unlock_file\n");

		
	}
	*/

	fclose(fp);
}

void RecordBrief::WriteFile(const char* path) const
{
	
	char StartTime[256];
	char EndTime[256];
	memset(StartTime,'\0',sizeof(StartTime));
	memset(EndTime,'\0',sizeof(EndTime));
	
	ParseBCDTimeStamp(startTime,StartTime);
	ParseBCDTimeStamp(endTime,EndTime);
	FILE *fp = fopen(path, "a+");

	/*
	int fd = fileno(fp);
	int ret;
	// 加写锁
	ret = lock_file_write(fd);
	if (ret == -1) {
		ERR_EXIT("lock_file_write\n");
	}
	
	fflush(fp); 
	*/
	
	
	fprintf(fp,"%s,%d,%s,%s,%s,%s,%s,%s,%s,%s\n",\
			filename.c_str(),nIndex-1,recordType.c_str(),callingParty.c_str(),calledParty.c_str(),
			StartTime,EndTime,localRecordSeq.c_str(),recordSeq.c_str(), callduration.c_str());
			

/*	printf("%s,%d,%s,%s,%s,%s,%s,%s,%s,%s\n",\
			filename.c_str(),nIndex-1,recordType.c_str(),callingParty.c_str(),calledParty.c_str(),
			StartTime,EndTime,localRecordSeq.c_str(),recordSeq.c_str(), callduration.c_str()); */

	/*
	fflush(fp);  //文件缓存->内核缓存(write) 防止竟态

	
	ret = unlock_file(fd);
	if (ret == -1) {
		ERR_EXIT("unlock_file\n");
	}
	*/
	
	fclose(fp);
}


void RecordBrief::WriteIndexFile(const char* path) const
{
	char homedir[128]={0};
	
	/*
	FILE *fp = fopen(querykey, "r");
	if(fp == NULL) // 文件不存在
	{
		fp = fopen(querykey, "a+");
		const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
		fprintf(fp,"%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
	}
	
	else
	{
		fp = fopen(querykey, "a+");
	}
	*/

// 	建立索引文件时程序意外挂掉,一个.dat文件部分cdr记录已写入相应文件,程序重新运行时去重处理。
	static int safeflag = FALSE;  // safeflag必须为静态变量
	if(safeflag== FALSE)
	{
		std::string lastrecordindex;   // path文件最后一行对应的recordIndex。
		std::string lastlinefilename;
		GetInfoFile(path, lastrecordindex, lastlinefilename);
 
		// lastrecordindex.empty() 表示path文件不存在     或者 .dat文件不同         
		if(lastrecordindex.empty() || (lastlinefilename != filename) || (nIndex - 1 > atoi(lastrecordindex.c_str())))                                             
		{
			safeflag = TRUE;
			WriteFile(path);
		}
		
	}
	else
	{
		WriteFile(path);
	}
	
}


CdrQuery::CdrQuery() 
{
	InitArgc();
	InitAsn();	
};


void CdrQuery::InitArgc()
{
	cdrnum = -1;
	opt = -1;
}


// 在fileList(记录查询目录下所有.dat文件)中,删除在processedlist中存在的.dat文件。
void CdrQuery::GetRealFileList(std::vector<std::string>& needprocesslist)
{
	std::string  temp = GetProcessedString();
	if(!temp.empty())
	{
		std::vector<std::string> processedlist = GetProcessedList(temp, '\n');	
		std::vector<std::string>::iterator that = processedlist.begin();

		
		int i = 0;
		int j = 0;

		for (; that != processedlist.end(); that++)
		{
			for(j = 0; j < needprocesslist.size(); )
			{
				if ( processedlist[i] == needprocesslist[j] )
				{
					needprocesslist.erase(needprocesslist.begin() + j);
					break;
				}
				else
				{
					++j;
				}

			}
			i++;
		}
		
		
	}

}

// 已处理文件记录
void CdrQuery::WritetoProcessedFile(const char *filename, const char& c)
{
	FILE *fp = fopen(INDEX_FILE_NAME, "a+");
	
//	printf("write file name to file = %s\n", filename);
	fprintf(fp, "%s%c", filename, c);
	fclose(fp);

}



void CdrQuery::InitAsn()
{ 
	int ret = ber_init(ASN_FILE_NAME, NULL, "IMSRecord");  ///  /opt/utoss/cdrquery/query/IMSChargingDataTypes.asn
	//ber_show_tree(0);
	if (ret)
	{
		ERR_EXIT("ber_init error\n");
	}
}

void CdrQuery::CreateCallingIndex(const std::list<RecordBrief>::const_iterator &it)
{

	std::string calling_num = it->GetCallingParty();
	
	if( strcmp( calling_num.c_str(),"calling is NULL" ) == 0 )
	{
		std::string callingpath = AddPathPrefix(dirpath, CALLING_PATH_NAME);  //callingpath保存主叫文件绝对路径
		callingpath = AddPathPrefix(callingpath, "emptycalling");
		it->WriteIndexFile(callingpath.c_str());
	}
	else if( calling_num.size() > CALL_PREFIX_LENGTH)
	{
		calling_num = calling_num.substr(CALL_PREFIX_LENGTH); 
		
		std::string callingpath = AddPathPrefix(dirpath, CALLING_PATH_NAME);  //callingpath保存主叫文件绝对路径
		callingpath = AddPathPrefix(callingpath, calling_num);
		it->WriteIndexFile(callingpath.c_str());
	}
	else
	{
		//ERR_MSG("calling length is short\n"); 
	}
	
}


void CdrQuery::CreateCalledIndex(const std::list<RecordBrief>::const_iterator &it)
{

	std::string called_num = it->GetCalledParty();

	if( strcmp( called_num.c_str(),"called is NULL" ) == 0)
	{
		std::string calledpath = AddPathPrefix(dirpath, CALLED_PATH_NAME);  //calledpath保存被叫文件绝对路径
		calledpath = AddPathPrefix(calledpath, "emptycalled");
		it->WriteIndexFile(calledpath.c_str());

	}
	else if( called_num.size() > CALL_PREFIX_LENGTH)
	{
		called_num = called_num.substr(CALL_PREFIX_LENGTH);

		std::string calledpath = AddPathPrefix(dirpath, CALLED_PATH_NAME);  //calledpath保存被叫文件绝对路径
		calledpath = AddPathPrefix(calledpath, called_num);
		it->WriteIndexFile(calledpath.c_str());

	}
	else
	{
		//ERR_MSG("called length is short\n");
	}

}



void CdrQuery::CreateTimeIndex(const std::list<RecordBrief>::const_iterator &it)
{
		
	char StartTime[256];
	memset(StartTime,'\0',sizeof(StartTime));
	it->ParseBCDTimeStamp(it->GetStartTime(),StartTime);
	std::string starttime = StartTime;

	if( starttime.size() > TIME_LENGTH)
	{
		starttime = starttime.substr(0, TIME_LENGTH);

		std::string timepath = AddPathPrefix(dirpath, TIME_PATH_NAME);  //calledpath保存被叫文件绝对路径
		timepath = AddPathPrefix(timepath, starttime);
		it->WriteIndexFile(timepath.c_str());

	}
	else if (starttime.size() == 0)
	{
		std::string timepath = AddPathPrefix(dirpath, TIME_PATH_NAME);  //calledpath保存被叫文件绝对路径
		timepath = AddPathPrefix(timepath, "emptystarttime");
		it->WriteIndexFile(timepath.c_str());
		
	}
	else
	{
		//ERR_MSG("time length is short\n");
	}

}




void CdrQuery::CreateRecordIndex()
{
	//std::list<RecordBrief> brieflist;

	std::list<RecordBrief>::iterator it = recordList.begin(); 
	for(; it!= recordList.end() ; it++)
	{

		CreateCallingIndex(it);
		CreateCalledIndex(it);
		CreateTimeIndex(it);
	
	}
	recordList.clear();  //释放
}


void CdrQuery::	WritetoLog(const char* logname, const char* filename, const int &cdrnum)  
{	
	FILE *fp = fopen(logname, "a+");
	fprintf(fp, "%s %d\n", filename, cdrnum);
	fclose(fp);
	
}

/*
void CdrQuery::ReadFromIndexFile(char* str)
{
	FILE* fp;
	
	fp = fopen(INDEX_FILE_NAME, "a+");
	
	fgets(str, SMALLBUFFSIZE, fp); //首次创建str为'\0'
	fclose(fp);
}

	
void CdrQuery::WriteToIndexFile(const char* str)
{
	FILE* fp;
	fp = fopen(INDEX_FILE_NAME, "w");
	fprintf(fp, "%s", str);
	fclose(fp);
}
*/


bool CmpString(const std::string &First, const std::string &Second)
{
    if (First < Second) //由小到大排序
    {
       return true ;
    }
	return false ;
}



unsigned int CdrQuery::ProcessRecordIndex(unsigned char flag, const char* logpath)
{
	unsigned int totalrecord = 0;
	if(flag == 0 || flag == 7)
	{		
		std::vector<std::string>::iterator it = realfilelist.begin();
  
		for(; it != realfilelist.end(); ++it)
		{
			filepath = *it;	
			Query(it->c_str(),cdrnum);
			//printf("filepath=%s\n", filepath.c_str());
			
			CreateRecordIndex();
		
			WritetoProcessedFile(filepath.c_str(),'\n');
			WritetoLog(logpath, filepath.c_str(), cdrnum); 
			totalrecord += cdrnum;
		}
	}
		
	else{
		std::vector<std::string>::iterator it = fileList.begin();   
  
		for(; it != fileList.end(); ++it)
		{
			filepath = *it;	
			Query(it->c_str(),cdrnum);
			//printf("filepath=%s\n", filepath.c_str());
			
			CreateRecordIndex(flag);  //不存在的目录处理
		
		//	WritetoProcessedFile(filepath.c_str(),'\n');
		//	WritetoLog(logpath, filepath.c_str(), cdrnum); 
		//	totalrecord += cdrnum;
		}
		
		it = realfilelist.begin(); 
  
		for(; it != realfilelist.end(); ++it)
		{
			filepath = *it;	
			Query(it->c_str(),cdrnum);   
			//printf("filepath=%s\n", filepath.c_str());
			
			CreateRecordIndex(7-flag);    //存在目录的处理
		
			WritetoProcessedFile(filepath.c_str(),'\n');
			WritetoLog(logpath, filepath.c_str(), cdrnum); 
			totalrecord += cdrnum;
		}
		
		
	}

	return totalrecord;
}

	
void CdrQuery::CreateRecordIndex(unsigned char flag)  
{

	std::list<RecordBrief>::iterator it = recordList.begin(); 
	for(; it!= recordList.end() ; it++)
	{
		if(flag & CALLINGDIR_FLAG)
		{
			CreateCallingIndex(it);
		}
		if(flag & CALLEDDIR_FLAG)
		{
			CreateCalledIndex(it);
		}
		if(flag & TIMEDIR_FLAG)
		{
			CreateTimeIndex(it);
		}
		
	}
	recordList.clear();  //释放
}


// 解析目录下的.dat文件,提取简短cdrrecord信息,并基于查询条件在指定目录下建立索引
void CdrQuery::DecodeDir()  
{
	Chdir(dirpath.c_str()); 
	ReadDir(dirpath,true);
	unsigned char flag = MkIndexDirs(); 

	sort(fileList.begin(),fileList.end(),CmpString);  
	realfilelist=fileList;

	FILE *fp = fopen(LOCK_FILE_NAME, "w");
	
	int fd = fileno(fp);
	int ret;
	// 加写锁
	ret = lock_file_write(fd);
	//printf("pid=%d\n",getpid());
	
	if (ret == -1) {
		ERR_EXIT("lock_file_write\n");
	}
	
	GetRealFileList(realfilelist);   //得到需要处理的.dat列表

	//-------------------------------------------------------------------------------------

	

	/*
	it = fileList.begin();
	for(; it != fileList.end(); it++)
	{
		printf("filepath=%s\n", it->c_str());
	}
	*/
	//char datfilename[SMALLBUFFSIZE] = {0};  //processed.r保存已处理过的.dat名字。
	char logname[SMALLBUFFSIZE];
	time_t now;
	struct tm *curTime;
	now = time(NULL);
	curTime = localtime(&now);
	sprintf(logname,"%04d%02d%02d%02d%02d%02d.log",curTime->tm_year+1900,
	curTime->tm_mon+1,curTime->tm_mday,curTime->tm_hour,curTime->tm_min,
	curTime->tm_sec);
	std::string logpath = AddPathPrefix(dirpath, LOG_PATH_NAME);  //logpath保存log文件绝对路径
	logpath = AddPathPrefix(logpath, logname);

	int totalrecord = 0;
	//ReadFromIndexFile(datfilename); 

	totalrecord=ProcessRecordIndex(flag, logpath.c_str());
	

	if(totalrecord > 0)
	{
		FILE *fp = fopen(logpath.c_str(), "a+");
		fprintf(fp, "totalrecord=%d\n", totalrecord);
		fclose(fp);
	}

//-------------------------------------------------------------------------------------
	
	ret = unlock_file(fd);
	if (ret == -1) {
		ERR_EXIT("unlock_file\n");
	}

	fileList.clear(); 
	realfilelist.clear();
	//unlink(LOCK_FILE_NAME);

	
//	displayProcessTimes("CreateRecordIndex end:\n");
	//--it; //指向最新的.dat文件
	//fprintf(fp, "%s", (*it).c_str());
}

void CdrQuery::CmdOpt(int argc,char ** argv)
{
	while ((opt = getopt(argc, argv, "d:")) > 0) 
	{
		switch (opt) 
		{ 
			case 'd':					
				dirpath = optarg;   //-d 指定的dirpath必须为绝对路径
				break;
			default:
				usage(argv[0]);
				break;
		}
	}

}


void CdrQuery::ProDir(char ** argv)
{
	if(dirpath.empty() == false)  // 查询目录
	{
		DecodeDir();	
	}
	else{
		usage(argv[0]);
	}
	
}
CdrQuery::~CdrQuery()
{


}

/*
bool CdrQuery::BrotherStr(string tocompare1, string tocompare2) 
{ 
    if(tocompare1.length() != tocompare2.length()) 
    { 
        return false; 
    } 
    const char *cmp1ptr = tocompare1.c_str(); 
    const char *cmp2ptr = tocompare2.c_str(); 
    int count[2][126]; 
    memset(count, 0, sizeof(int) * 2 * 126); 
    while(*cmp1ptr != '\0') 
    { 
        count[0][*cmp1ptr++]++; 
        count[1][*cmp2ptr++]++; 
    } 
    for(int cmp = 0; cmp < 126; ++cmp) 
    { 
        if(count[0][cmp] != count[1][cmp]) 
        { 
            return false; 
        } 
    } 
    return true; 
}
*/



std::vector<CDR*>& CdrQuery::GetCDRList()
{
	return cdrList;
}

const CASN1Desc& CdrQuery::GetASN1Desc() const
{
	return m_asnDesc;
}

BERCoder* CdrQuery::GetBERCoder(const char* name) const
{
	return m_asn1_coders.get(name);
}

int CdrQuery::ReadDir(std::string dir,bool  brecursive)
{
	 std::string::size_type found = 0;
	 DIR              *pDir ; 
     struct dirent    *ent  ;
     pDir=opendir(dir.c_str());  
	 if(pDir == NULL)
	 {
	 	ERR_EXIT("opendir failed!\n");
	 }
     
    for (;;) 
	{
        errno = 0;              /* To distinguish error from end-of-directory */  
		ent  = readdir(pDir);
		if (ent == NULL)
			   break;
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		{
		//		printf("ent->d_name=%s\n", ent->d_name);
				continue;
		}
     	else if((ent->d_type & DT_DIR) && brecursive)  
        {  
        	if( is_ProcessDir(ent->d_name) )
        	{
				std::string path = AddPathPrefix(dir,ent->d_name);
			//	printf("path=%s\n", path.c_str());
				ReadDir(path,brecursive);
        	}
	    }  
	  	else
	   	{	
	   		std::string str = ent->d_name;
			
			found = str.find_last_of('.');
			if (found == std::string::npos)
				continue;
			str = str.substr(found);
			if(!strcmp(str.c_str(),".dat"))
			{
			//	printf("%s	",ent->d_name);      
	   			std::string tmp = AddPathPrefix(dir,ent->d_name);
				fileList.push_back(tmp);
			//	printf("tmp = %s\n",tmp.c_str());
			}
		}
	 }

	 if (errno != 0)
        ERR_EXIT("readdir\n");
	 
	 if(closedir(pDir) == -1)
	 {
	 	ERR_EXIT("closedir\n");
	 }

	 /*
	 std::vector<std::string>::iterator it = fileList.begin();
	 for(; it != fileList.end(); it++)
	 {
		printf("%s\n",it->c_str());	 
	 }
	 */

	return 0;
}

int CdrQuery::Query(const char* filename,int &cdrnum)
{
	/*
	std::string str = filename;

	
	std::string::size_type found = str.find_last_of('.');
	std::string suffix = str.substr(found);
	if(strcmp(suffix.c_str(),".dat"))
	{
		ERR_EXIT("file suffix is not .dat\n");
	}
	*/
	//step1 打开文件
	FILE* fp =fopen(filename,"rb");
	if(fp ==NULL)
	{
		ERR_EXIT("openfile failed!\n");
	}
	//step2 读取文件头长度
	uint32_t fheadlen = 0;
	int ret = GetFileHeaderLength(fp,fheadlen);
	//printf("[%s] [%d] [cdrquerylength = %d ]\n",__FILE__,__LINE__,fheadlen);
	
	//step3 读取文件中cdr记录的个数
	uint32_t num = 0;

	ret = GetCdrNum(fp,num);
	cdrnum = num;

	if(num  == 0)
	{
		fclose(fp);
		ERR_MSG("file no cdrrecord!\n");
	}
	//step4 跳过文件头
	rewind(fp);
	fseek(fp,fheadlen,SEEK_SET);
	int numdecoded = 0;           // 记录已解码的cdr个数
	for(int i = 0; i < num; i++)
	{
		
		//step5 读取cdr的长度
		int16_t cdrlength = 0;
		ret = GetCdrLength(fp,cdrlength);

		//step6 跳过cdr头部
		fseek(fp,3,SEEK_CUR);
		Decode(fp,cdrlength);
		++numdecoded;
		if(numdecoded%FREESIZE == 0)
		{
			//释放cdrlist空间
			GetBriefList(recordList, numdecoded - FREESIZE); //(numdecoded - FREESIZE):record记录起始位置 从0开始
			FileListClear();  
		}

//		printf("[ %s ] [ %s ] [ %d ] [ %s ]  cdrnum = %d\n",__FILE__,__func__,__LINE__,str.c_str(),num);
		
		/*
		std::string str = filename;
		std::string::size_type found = str.find_last_of('/');
		if (found != std::string::npos)
		{
			str = str.substr(found+1,str.length()-found-1);
		}	
		CdrfileList.push_back(str);
		*/
		
	}
	if(numdecoded == num && (numdecoded%FREESIZE != 0))
	{
		//释放cdrlist空间
		GetBriefList(recordList, (numdecoded - numdecoded%FREESIZE));
		FileListClear();  
	}
	fclose(fp);
	return FILE_QUERY_SUCCESS;
}


char* hex2str(const char *source, char *dest,int sourceLen)
{
	short i;
	unsigned char highByte, lowByte;
	for (i = 0; i < sourceLen; i += 2)
	{
		highByte = toupper(source[i]);
		lowByte  = toupper(source[i + 1]);

		if (highByte > 0x39)
		{
			highByte -= 0x37;
		}
		else
		{
			highByte -= 0x30;
		}
		if (lowByte > 0x39)
        {
			lowByte -= 0x37;
        }
		else
		{
			lowByte -= 0x30;
		}
		dest[i / 2] = (highByte << 4) | lowByte;
    }
    return  dest;
}


void CdrQuery::GetBriefList(std::list<RecordBrief>& list, const int shift)
{
	std::vector<CDR*>& cdrlist = cdrList;
	const CASN1Desc& asnDesc = m_asnDesc;	
	int i = 1;
	const char * cfilename = filepath.c_str();
	int size = cdrlist.size();
	for(; i <= size ; i++)
	{
		RecordBrief tmp;
		char  dest[256] = {0};
		tmp.Setfilename(cfilename);
		tmp.SetIndex(i+shift);
		//printf("start=%d end=%d\n", 1+shift, shift+size);
		
	//	printf("[ %s ] [ %s ] [ %d ] [ %s ]  \n",__FILE__,__func__,__LINE__,CdrfileList.at(i - 1).c_str());

		CDR* pCDR = cdrlist[i-1];
		CASNTree &asnData = pCDR->data;
		size_t cdrType = asnData.GetTree()->tag;
		char oidBuff[128]={0};
	//	printf("[ %s ] [ %s ] [ %d ] [ %p ]  \n",__FILE__,__func__,__LINE__,&cdrlist);
		sprintf(oidBuff, "OID.%d.0", cdrType);
		const ASNNameHandle* recordTypeNH = ber_getnamehandle(oidBuff, 0);
//		printf("[%s] [%d] ----->oids==%d\n",__FILE__,__LINE__,*(recordTypeNH->asnname).oids());
		assert(recordTypeNH);
		const ASNBinTreeNode* pNode = asnData.GetNode((Uint*)recordTypeNH->asnname.oids(),recordTypeNH->asnname.size());
		if (!pNode)
		{
			return;
		}
		string2 txt;
		ber_decode(recordTypeNH, pNode->value, pNode->length, txt, 0);
		tmp.SetRecordType(txt.c_str());
//		printf("[ %s ] [ %s ] [ %d ] [ %s ]  \n",__FILE__,__func__,__LINE__,txt.c_str());	
		
		
		sprintf(oidBuff, "OID.%d.6.0", cdrType);
		const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
		assert(callingNH);
		pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.6.1", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(callingNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.6.2", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(callingNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.6.3", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(callingNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{	
			strcpy(dest,"calling is NULL");
		}
		else
		{
			ber_decode(recordTypeNH, pNode->value, pNode->length, txt, 0);
			hex2str(txt.c_str(), dest,txt.size());
		}
		tmp.SetCallingParty(dest);
	//	printf("[ %s ] [ %s ] [ %d ] [ %s ]  \n",__FILE__,__func__,__LINE__,txt.c_str());
		
		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.7.0", cdrType);
		const ASNNameHandle* calledNH = ber_getnamehandle(oidBuff, 0);
		assert(calledNH);
		pNode = asnData.GetNode((Uint*)calledNH->asnname.oids(),calledNH->asnname.size());
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.7.1", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(calledNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.7.2", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(calledNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{
			sprintf(oidBuff, "OID.%d.7.3", cdrType);
			const ASNNameHandle* callingNH = ber_getnamehandle(oidBuff, 0);
			assert(calledNH);
			pNode = asnData.GetNode((Uint*)callingNH->asnname.oids(),callingNH->asnname.size());
		}
		if (!pNode)
		{
			strcpy(dest,"called is NULL");
		}
		else
		{
			ber_decode(recordTypeNH, pNode->value, pNode->length, txt, 0);	
			hex2str(txt.c_str(), dest,txt.size());
		}
		tmp.SetCalledParty(dest);
		//printf("[ %s ] [ %s ] [ %d ] [ %s ]  \n",__FILE__,__func__,__LINE__,txt.c_str());

		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.10", cdrType);
		const ASNNameHandle* startTimeNH = ber_getnamehandle(oidBuff, 0);
		assert(startTimeNH);
		pNode = asnData.GetNode((Uint*)startTimeNH->asnname.oids(),startTimeNH->asnname.size());
		if (!pNode)
		{
			memset(dest,0,sizeof(dest));
			//return;
		}
		else
		{
			ber_decode(recordTypeNH, pNode->value, pNode->length, txt, 0);
			hex2str(txt.c_str(), dest,txt.size());
		}
//		printf("dest = %s\n",dest);
		tmp.SetStartTime(dest);

		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.11", cdrType);
		const ASNNameHandle* endimeNH = ber_getnamehandle(oidBuff, 0);
		assert(endimeNH);
		pNode = asnData.GetNode((Uint*)endimeNH->asnname.oids(),endimeNH->asnname.size());
		if (!pNode)
		{
			memset(dest,0,sizeof(dest));
			//return;
		}
		else
		{
			ber_decode(recordTypeNH, pNode->value, pNode->length, txt, 0);
			hex2str(txt.c_str(), dest,txt.size());
		}
		tmp.SetEndTime(dest);

		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.15", cdrType);
		const ASNNameHandle* localRecordSeqNH = ber_getnamehandle(oidBuff, 0);
		assert(localRecordSeqNH);
		pNode = asnData.GetNode((Uint*)localRecordSeqNH->asnname.oids(),localRecordSeqNH->asnname.size());
		if (!pNode)
		{
			strcpy(dest,"null");
			//return;
		}
		else
		{
			ber_decode(localRecordSeqNH, pNode->value, pNode->length, txt, 0);
			sprintf(dest,"%s",txt.c_str());
		}
		tmp.SetLocalRecordSeq(dest);

		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.16", cdrType);
		const ASNNameHandle* recordSeqNH = ber_getnamehandle(oidBuff, 0);
		assert(recordSeqNH);
		pNode = asnData.GetNode((Uint*)recordSeqNH->asnname.oids(),recordSeqNH->asnname.size());
		if (!pNode)
		{
			strcpy(dest,"null");
			//return;
		}
		else
		{
			ber_decode(recordSeqNH, pNode->value, pNode->length, txt, 0);
			sprintf(dest,"%s",txt.c_str());
		}
		tmp.SetRecordSeq(dest);


		// 添加 time duration
		memset(dest,0,sizeof(dest));
		sprintf(oidBuff, "OID.%d.250", cdrType);
		const ASNNameHandle* callDuration = ber_getnamehandle(oidBuff, 0);
		assert(recordSeqNH);
		pNode = asnData.GetNode((Uint*)callDuration->asnname.oids(),callDuration->asnname.size());
		if (!pNode)
		{
			strcpy(dest,"null");
			//return;
		}
		else
		{
			ber_decode(callDuration, pNode->value, pNode->length, txt, 0);
			sprintf(dest,"%s",txt.c_str());
		}
		tmp.SetCallDuration(dest);

		list.push_back(tmp);
		
	}

	return ;
}


void CdrQuery::FillTree(const ASNBinTreeNode* pInsertNode, const ASNName& parentName, int seq)
{
	if (!pInsertNode)
	{
		return ;
	}
	ShowNode tmp;
	ASNShowNode asn_tmp;
	string2 longname;
	ASNName self(parentName);
	int count = 0;
	if (TAGTYPE_GENERIC != pInsertNode->type)
	{
		Uint tag= pInsertNode->tag;
//		printf("[ %s ] [ %s ] [ %d ] tag = %d\n",__FILE__,__func__,__LINE__,tag);
		self.append(&tag, 1);
//		printf("[ %s ] [ %s ] [ %d ] self.c_str() = %s\n",__FILE__,__func__,__LINE__,self.str());
	}

	char* csLongName = NULL;
	if (ber_getlongname(self.str(), longname, 0))
	{
//		printf("[%s] [%d] longname.c_str() = %s\n",__FILE__,__LINE__,longname.c_str());
		if(csLongName != NULL)
		{
			sprintf(csLongName,"%d",pInsertNode->tag);
		}
	}
	else
	{
		csLongName = longname;
	}	

	if(csLongName != NULL)
	{
		if (TAGTYPE_GENERIC == pInsertNode->type && seq >= 0)
		{
			sprintf(csLongName+strlen(csLongName),"[%d]",seq);
/*
			std::vector<ASNShowNode>::iterator it = AnodeList.begin();
			for(;it!=AnodeList.end();it++)
			{
				
					if(strcmp(it->node_name,csLongName) == 0)
					{
						printf("521----%s-",it->node_name);
						sprintf(csLongName+strlen(csLongName),"[%d]",seq);
						strncpy(asn_tmp.node_name,csLongName,strlen(csLongName));
						//it++;
						//printf("525----%s-",it->node_name);
						//AnodeList.insert(it,asn_tmp);
					}
				
			}
		*/}

		if (!pInsertNode->construct) {
			const ASNNameHandle* hName = ber_getnamehandle(self.str(), 0);
			string2 svalue;
			ber_decode(hName, pInsertNode->value, pInsertNode->length, svalue, 0);
			//printf("%s = %s\n",csLongName,svalue.c_str());
			strcpy(tmp.node_name,csLongName);
			memcpy(tmp.node_value,svalue.c_str(),svalue.size());
		}
		else
		{	
			//printf("%s\n",csLongName);
			strcpy(tmp.node_name,csLongName);
		}
		nodeList.push_back(tmp);
	}
	for (const ASNBinTreeNode* next = pInsertNode->children; next; next=next->brothers)
	{
		int seq = 0;
		for (const ASNBinTreeNode* pSeqOf = next; pSeqOf; pSeqOf = pSeqOf->sequenceof) {

			FillTree(pSeqOf, self, seq);
			seq++;
		}

	}

}

std::string CdrQuery::AddPathPrefix(const std::string& path,const std::string& filename) 
{
	std::string retStr;
	retStr = path;
	std::string::size_type found = path.rfind("/");
	if(found == std::string::npos || found != (path.length()-1) )    
	{
		retStr += "/";				
	}
	retStr += filename;
	return retStr;
}


const char* StringFind(char *src, const char *dst)
{
	int i, j;
	for (i=0; src[i]!='\0'; i++)
	{
		if(src[i]!=dst[0])
		continue;
		j = 0;
		while(dst[j]!='\0' && src[i+j]!='\0')
	{
		j++;
		if(dst[j]!=src[i+j])
		break;
	}
	if(dst[j]=='\0')
		return dst;
	}
	return NULL;
}



int  CdrQuery::GetCdrNum(FILE* fp,uint32_t& len)
{
	len = 0;
	uint32_t cdrnum;
	memset(&cdrnum,'\0',sizeof(cdrnum));
	rewind(fp);
	if(fseek(fp,18,SEEK_SET))
	{
		fclose(fp);
		ERR_EXIT("fseek faiure!\n");
	}
	unsigned int size = fread(&cdrnum,1,4,fp);
	
//	printf("CdrQuery::GetCdrNum size of = %d\n",size);
	if(size != 4)
	{
		fclose(fp);
		ERR_EXIT("fread failed!\n");
	}
	len = ntohl(cdrnum);
//	printf("CdrQuery::GetCdrNum len = %d\n",len);
	return 0;
}


int  CdrQuery::GetFileHeaderLength(FILE* fp,uint32_t& len)
{
	uint32_t fileheadlen;
	memset(&fileheadlen,'\0',sizeof(fileheadlen));
	rewind(fp);
	if(fseek(fp,4,SEEK_SET))
	{
		fclose(fp);
		ERR_EXIT("fseek faiure!\n");
	}
	unsigned int size = fread(&fileheadlen,1,4,fp);
//	printf("CdrQuery::GetFileHeaderLength size of = %d\n",size);
	if(size != 4)
	{
		fclose(fp);
		ERR_EXIT("fread failed!\n");
	}
	len=ntohl(fileheadlen);
	if(len == fileheadlen)
	{
		fclose(fp);
		ERR_EXIT("ntohl failed!\n");
	}
//	printf("CdrQuery::GetFileHeaderLength len = %d\n",len);
	return 0;
}


int  CdrQuery::GetCdrLength(FILE* fp,int16_t& len)
{
	int16_t cdrlen;
	memset(&cdrlen,'\0',sizeof(cdrlen));
	unsigned int size = fread(&cdrlen,1,2,fp);
	/*
	char tmp[3];
	memset(tmp,'\0',sizeof(tmp));
	memcpy(tmp,&cdrlen,2);
	printf("CdrQuery::GetCdrLength str = %0x %0x size of = %d\n",tmp[0],tmp[1],size);
	*/
	if(size != 2)
	{
		fclose(fp);
		ERR_EXIT("fread failed!\n");
	}
	
	len=ntohs(cdrlen);
//	printf("CdrQuery::GetCdrLength len = %d\n",len);
	return 0;
}



ASN1OCTET* CdrQuery::Decode(FILE* fp,int cdrlen)
{
	int len = cdrlen;
	int size = 0;
	ASN1OCTET   *buff = new ASN1OCTET[len];
	memset(buff,'\0',len);
	CDR* pCDR = new CDR;
	size = fread(buff, 1, len, fp);
	pCDR->data.Parse(buff, len); 
	cdrList.push_back(pCDR);
	
//	std::vector<CDR*>::iterator it = cdrList.begin();
	if(buff != NULL)
	{
		delete []buff;
		buff = NULL;
	}
}


void CdrQuery::crd_list_Clear()
{
	for (size_t i = 0; i<cdrList.size(); i++) {
		delete(cdrList[i]);
	}
	cdrList.clear();
	
}

void CdrQuery::FileListClear()
{
	crd_list_Clear();

}




