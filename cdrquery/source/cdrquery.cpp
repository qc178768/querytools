
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "cdrquery.h"
#include <unistd.h> //chdir
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>

extern char FileSuffix[1024];
const char* StringFind(char *src, const char *dst);

//-------------------------------------------
#define ERR_EXIT(msg) \
	do \
	{  \
			fprintf(stderr, "[%s] [%d] %s", __FILE__, __LINE__,	msg);;  \
			exit(EXIT_FAILURE);   \
	}while(0)  
	
enum {
	DISP_BRIEF = 1,
	DISP_DETAIL ,
	DISP_QUERY_BRIEF ,
	DISP_QUERY_KEY,	    // 单索引文件查询(主叫,被叫,starttime(天))
	DISP_QUERY_TIME,	// 按时间段查询 精确到秒
};

void usage(char *prog)
{
	fprintf(stderr, 
			"Usage: %s [-m mode] [-f filepath] [-n recordno] [-d dirpath] [-i callingParty] [-I calledParty] [-t  startime  endtime]\n",
			prog);
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



void RecordBrief::ParseBCDTimeStamp(const ASN1OCTET* src,char* dest)
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

void RecordBrief::RecordBriefPrint()
{
	char StartTime[256];
	char EndTime[256];
	memset(StartTime,'\0',sizeof(StartTime));
	memset(EndTime,'\0',sizeof(EndTime));
	
	ParseBCDTimeStamp(startTime,StartTime);
	ParseBCDTimeStamp(endTime,EndTime);
	
	printf("%s,%d,%s,%s,%s,%s,%s,%s,%s,%s\n",\
	filename.c_str(),nIndex-1,recordType.c_str(),callingParty.c_str(),calledParty.c_str(),StartTime,EndTime,localRecordSeq.c_str(),recordSeq.c_str(), callduration.c_str());
	return ;
}

CdrQuery::CdrQuery() 
{
	InitArgc();
	InitAsn();
};

void CdrQuery::InitArgc()
{
	maxrecord = -1;
	opt = -1;
	mode = -1;
	recordno = -1;
	pageno = 0;				// 设置查询页(默认为0)
	curpagenum = -1;
	pagesize = 100; 		// 指定页大小(默认为100)
}

void CdrQuery::InitAsn()
{
	int ret = ber_init(ASN_FILE_NAME, NULL, "IMSRecord");  //运行程序同级目录下必须有IMSChargingDataTypes.asn文件
	//ber_show_tree(0);
	if (ret)
	{
		ERR_EXIT("ber_init error\n");
	}
}

int CdrQuery::GetFileCdrSum(const char* filename,int &maxrecord)
{
	std::string str = filename;
	std::string::size_type found = str.find_last_of('.');
	std::string suffix = str.substr(found);
	if(strcmp(suffix.c_str(),".dat"))
	{
		printf("[%s] [%d] error\n",__FILE__,__LINE__);
		exit(-1);
	}
	//step1 打开文件
	FILE* fp =fopen(filename,"rb");
	if(fp ==NULL)
	{
		printf("file open failed !");
		exit(-1);
	}
	
	uint32_t num = 0;
	//step2 读取文件中cdr记录的个数
	int ret = GetCdrNum(fp,num);
	maxrecord = num;

	if(ret)
	{
		return ret;
	}
	if(num  == 0)
	{
		fclose(fp);
		return FILE_NO_CDRRECORD;
	}
	
	return FILE_QUERY_SUCCESS;
}


int CdrQuery::FindRecordRange(int &startno, int& endno, int& rmax)
{
	int pagenum;
	if(pageno >= 0)
	{
		pagenum = (rmax / pagesize) + ((rmax%pagesize) ? 1:0); 
		if(pageno <= pagenum-1)
		{
			startno = pageno * pagesize;
			endno = (pageno == pagenum-1)? rmax : (startno+pagesize);
			curpagenum = endno - startno;  
			//printf("pageno=%d startno=%d, endno=%d,curpagenum=%d\n",pageno,startno,endno,curpagenum);
			return 0;
		}
		else{
			ERR_EXIT("query recordno no exist\n");
		}
	}
	else{
		ERR_EXIT("query recordno no exist\n");
	}
}


int CdrQuery::Query(const char* filename,int& startno, int& endno)
{
	std::string str = filename;
	std::string::size_type found = str.find_last_of('.');
	std::string suffix = str.substr(found);
	if(strcmp(suffix.c_str(),".dat"))
	{
		ERR_EXIT("file suffix is not .dat\n");
	}
	//step1 打开文件
	FILE* fp =fopen(filename,"rb");
	if(fp ==NULL)
	{
		ERR_EXIT("file open failed !\n");
	}
	//step2 读取文件头长度
	uint32_t fheadlen = 0;
	int ret = GetFileHeaderLength(fp,fheadlen);
	//printf("[%s] [%d] [cdrquerylength = %d ]\n",__FILE__,__LINE__,fheadlen);
	
	if(ret)
	{
		fclose(fp);
		return ret;
	}
	//step3 跳过文件头
	rewind(fp);
	fseek(fp,fheadlen,SEEK_SET);
	for(int i = 0; i < endno; i++)
	{
		
		//step4 读取cdr的长度
		int16_t cdrlength = 0;
		ret = GetCdrLength(fp,cdrlength);
		if(ret)
		{
			fclose(fp);
			return ret;
		}
		//step5 跳过cdr头部
		fseek(fp,3,SEEK_CUR);
		if(i >=startno && i<endno)
		{
			Decode(fp,cdrlength);			
		}
		else{
			fseek(fp,cdrlength,SEEK_CUR);
		}
	}
	fclose(fp);
	return FILE_QUERY_SUCCESS;
}


void CdrQuery::DispFileBrief(){
	FileListClear();
	GetFileCdrSum(filepath.c_str(),maxrecord);	// maxrecord 文件cdr总数
	int sno, eno;
	FindRecordRange(sno, eno, maxrecord);
	Query(filepath.c_str(),sno,eno);
	QueryShowBrief(sno);
}


void CdrQuery::DispFileDetail(){
	FileListClear();
	GetFileCdrSum(filepath.c_str(),maxrecord);
	int sno, eno;
	sno = recordno;
	eno = recordno+1;
	//printf("startno=%d, endno=%d\n",sno, eno);
	if(recordno >= 0 && recordno < maxrecord){
			Query(filepath.c_str(),sno, eno); // 
			ShowDetail(recordno);
	}
	else 
	{
		ERR_EXIT("query recordno no exist\n");
	}
	
}

void CdrQuery::DispFile(int mode)
{
	if(mode == DISP_BRIEF){
		DispFileBrief();
	}
	else if(mode == DISP_DETAIL){
		DispFileDetail();
	}
	else{
		ERR_EXIT("need to set file querymode!\n" 
					"-m 1 briefquery\n"
					"-m 2 detailquery\n");
	}
}


void GetTimeString(const std::string& str, std::string& timestr)
{
	char line[256] = {0};
	strcpy(line, str.c_str());
	char*temp = strtok(line,",");
	int i = 1;
	while(temp) 
	{
		if(i == 6)
		{
			//printf("temp=%s\n",temp);
			timestr = temp;
			break;
		}
		temp = strtok(NULL,",");
		++i;
	}
	
}


bool CmpTimeString(const std::string &First, const std::string &Second)
{
	std::string ftime, stime; 
	GetTimeString(First, ftime);  //得到starttimeString
	GetTimeString(Second, stime);
	if (ftime < stime) //由小到大排序
    {
       return true ;
    }
	return false ;
}





void CdrQuery:: GetSet(FILE* fp, std::vector<string>& callset) 
{
	char line[256] = {0};
//	char starttime[64] = {0};
//	char endtime[64] = {0};
	std::string str;
	while((fgets(line,sizeof(line),fp))!=NULL){
		
	//	char* find = strstr(line, ",,");  	//滤除starttime endtime为空的record信息
	//	if(find == NULL)
	
		{
			str = line;
			callset.push_back(str);
		}
	}		
}


void CdrQuery::GetTimeSet(FILE* fp, std::vector<string>& callset)  
{
	char line[256] = {0};
	char resline[256] = {0};
	char starttime[64] = {0};
	char endtime[64] = {0};
	std::string str;
	while((fgets(line,sizeof(line),fp))!=NULL){
		char* find = strstr(line, ",,");	// 存在说明endtime不存在.
		char  endtimeisempty = FALSE;
		strcpy(resline, line);
		char* temp = strtok(line,",");

		if(find)
		{
			endtimeisempty = TRUE;
		}
		int i = 1;
	    while(temp)  
	  	{
	  		if(i == 6)
	  		{
				strcpy(starttime, temp); // 在主被叫存在的情况下,默认开始时间是有的。。
			}
			if(i == 7 ) 
			{
				if(endtimeisempty == FALSE )
				{
					strcpy(endtime, temp);  
				}
				else  
				{
					strcpy(endtime, ""); 
				}
				break;
			}
			
			temp = strtok(NULL,",");
			++i;
	   	}
		if(i == 7)		// temp为NULL退出, i<7 情况一般不会发生
		{
			if( (strcmp(starttime, stime.c_str()) >= 0 ) && (strcmp(endtime, etime.c_str()) <= 0) )
			{
					str = resline;
					callset.push_back(str);
			}
		}
		
	}

}

void CdrQuery::NoQueryInfo()
{
	printf("0 0\n");
	const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
	printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
}

void CdrQuery::PrintHead(const int& setnums)
{
	printf("%d %d\n",curpagenum, setnums);
	const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
	printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
}

int  CdrQuery::ReadTimeCdrFromfile(const char * path)
{
	FILE *fp = fopen(path, "r");
	if(fp == NULL) // 文件不存在
	{
		return 0;
	}
	else
	{
		GetTimeSet(fp, recordList);  
		fclose(fp);
		return recordList.size();	
	}

}

void CdrQuery::ReadCdrFromfile(const char* path)  
{
	//char homedir[128]={0};
	
	//getcwd(homedir, sizeof(homedir)); //保存当前索引根目录
	
	FILE *fp = fopen(path, "r");
	if(fp == NULL) // 文件不存在
	{
		NoQueryInfo();
		return;
		/*
		printf("0 0\n");
		const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
		printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
		*/
	}
	else
	{
		/*
		int fd = fileno(fp);
		int ret;
		// 加读锁
		ret = lock_file_read(fd);
		if (ret == -1) {
			ERR_EXIT("lock_file_read\n");
		}
		fflush(fp);
		*/
		
		std::vector<string> resset;
		if( (!stime.empty()) && (!etime.empty()) ) 
		{
			GetTimeSet(fp, resset);    // 按时间段查询  单主叫 | 单被叫 | 某天
		}
		else{		
			GetSet(fp, resset);	  
		}

	
		int setnums = resset.size();
		if(setnums == 0) 
		{
			NoQueryInfo();
			return;
			/*
			printf("0 0\n");
			const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
			printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
			*/
		}

		int sline, eline;
		sort(resset.begin(),resset.end(),CmpTimeString);  
		FindRecordRange(sline, eline, setnums);
		PrintHead(setnums);
		/*
		printf("%d %d\n",curpagenum, setnums);
		const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
		printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
	    */
		std::vector<string>::iterator iter = resset.begin();
		int i =0;
		for(; iter!= resset.end() ; ++iter, ++i)
		{
			if( (i >= sline) && (i < eline ))
			{
				printf("%s", (*iter).c_str());
			}
		}
		resset.clear();

		/*
		ret = unlock_file(fd);
		if (ret == -1) {
			ERR_EXIT("unlock_file\n");
		}
		
		fflush(fp);
		*/
		fclose(fp);
	//	Chdir(homedir);  //改变到索引根目录
		
	}
}

void CdrQuery::GetGroupSet(FILE* fp, std::vector<string>& callset, const char* calling) 
{
	char line[256] = {0};
	char resline[256] = {0};
	char starttime[64] = {0};
	char endtime[64] = {0};
	std::string str;
	while((fgets(line,sizeof(line),fp))!=NULL){
		char* find = strstr(line, ",,");	
		char  endtimeisempty = FALSE;
		strcpy(resline, line);
		char*temp = strtok(line,",");
		
		if(find)
		{
			endtimeisempty = TRUE;
		}
		
		int i = 1;
    	while(temp && i<4)  // 定位到主叫字符串           
  		{
       	 	temp = strtok(NULL,",");
			++i;
   		}
		if((i == 4)&&(strcmp(temp+strlen("tel:"), calling) == 0))
		{
			if((!stime.empty()) && (!etime.empty()))  // -i and -I and -t
			{
				 while(temp)            
	  			 {
	  				if(i == 6)
	  				{
						strcpy(starttime, temp);
					}
					if(i == 7)
					{
						if(endtimeisempty == FALSE )
						{
							strcpy(endtime, temp);  
						}
						else  
						{
							strcpy(endtime, ""); 
						}
						break;
					}
	       	 		temp = strtok(NULL,",");
					++i;
	   			}
				if( (strcmp(starttime, stime.c_str()) >= 0 ) && (strcmp(endtime, etime.c_str()) <= 0) )
				{
					str = resline;
					callset.push_back(str);
				}
			}
			else{		// -i and -I
					str = resline;
					callset.push_back(str);
			}
		}

	}

}
 
void CdrQuery::ReadCdrFromfile(const char* path, const char* calling)  //在被叫文件找对应主叫cdr记录 | -t
{
	//char homedir[128]={0};
	
	//getcwd(homedir, sizeof(homedir)); //保存当前索引根目录
	
	FILE *fp = fopen(path, "r");
	if(fp == NULL) // 文件不存在
	{
		NoQueryInfo();
		return;
		/*
		printf("0 0\n");
		const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
		printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
		*/
	}
	else
	{
		/*
		int fd = fileno(fp);
		int ret;
		// 加读锁
		ret = lock_file_read(fd);
		if (ret == -1) {
			ERR_EXIT("lock_file_read\n");
		}
		fflush(fp);
		*/
		
		std::vector<string> callset;
		GetGroupSet(fp, callset, calling);  


		int setnums = callset.size();
		if(setnums == 0) 
		{
			//ERR_EXIT("query key wrong!\n");
			NoQueryInfo();
			return;
			/*
			printf("0 0\n");
			const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
			printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
			return;
			*/
		}

		int sline, eline;
		sort(callset.begin(),callset.end(),CmpTimeString);  
		FindRecordRange(sline, eline, setnums);
		PrintHead(setnums);
		/*
		printf("%d %d\n",curpagenum, setnums);
		const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
		printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
		*/

		std::vector<string>::iterator iter = callset.begin();
		int i =0;
		for(; iter!= callset.end() ; ++iter, ++i)
		{
			if( (i >= sline) && (i < eline ))
			{
				printf("%s", (*iter).c_str());
			}
		}
		callset.clear();

		/*
		ret = unlock_file(fd);
		if (ret == -1) {
			ERR_EXIT("unlock_file\n");
		}
		
		fflush(fp);
		*/
		fclose(fp);
	//	Chdir(homedir);  //改变到索引根目录
	}
}


void CdrQuery::DispQueryBrief(unsigned int qmode)   // -i -I -t -d dir
{
	if(qmode == DISP_QUERY_KEY )
	{
		if(q_callingParty.empty()&& (!q_calledParty.empty()))
		{
			
			std::string queryfile = AddPathPrefix(dirpath, CALLED_PATH_NAME);
			queryfile = AddPathPrefix(queryfile, q_calledParty);
			ReadCdrFromfile(queryfile.c_str());
		}
		else if(!q_callingParty.empty()&& (q_calledParty.empty()))
		{
			std::string queryfile = AddPathPrefix(dirpath, CALLING_PATH_NAME);
			queryfile = AddPathPrefix(queryfile, q_callingParty);
			ReadCdrFromfile(queryfile.c_str());
		}
		else if((!q_callingParty.empty())&& (!q_calledParty.empty())) // 被叫索引文件里查找对应主叫
		{
			std::string queryfile = AddPathPrefix(dirpath, CALLED_PATH_NAME);
			queryfile = AddPathPrefix(queryfile, q_calledParty);
			ReadCdrFromfile(queryfile.c_str(), q_callingParty.c_str());
		}
		else
		{
			ERR_EXIT("need to set callingno or calledno \n");
		}
	}
	else if(qmode == DISP_QUERY_TIME)
	{
		if (!stime.empty() && !etime.empty() )
		{
			string startdate = stime.substr(0, TIME_LENGTH);
			string enddate = etime.substr(0, TIME_LENGTH);
			if( (startdate < enddate) || (stime <= etime) )
			{ 
				int startyear,startmonth,startday;
				int endyear,endmonth,endday;
				int month, day;
				char date[MAX_PATH]; // 索引date文件
				sscanf(startdate.c_str(),"%d-%d-%d",&startyear,&startmonth,&startday);
				sscanf(enddate.c_str(),"%d-%d-%d",&endyear,&endmonth,&endday);
				int recordnum = 0; 

				if(startyear == endyear)
				{
					int startyearday = day_of_year(startyear, startmonth, startday);
					int endyearday = day_of_year(endyear,endmonth,endday);
					int i = 0;
					for(i=startyearday; i<=endyearday; ++i)
					{
						month_day(startyear, i, &month, &day);
						sprintf(date,"%04d-%02d-%02d",startyear,month,day);
						std::string queryfile = AddPathPrefix(dirpath, TIME_PATH_NAME);
						queryfile = AddPathPrefix(queryfile, date);
						ReadTimeCdrFromfile(queryfile.c_str());
					}
					recordnum = recordList.size();
					if(recordnum == 0) 
					{
						NoQueryInfo();
						return;
					}

					int sline, eline;
					sort(recordList.begin(),recordList.end(),CmpTimeString);  	
					FindRecordRange(sline, eline, recordnum);
					PrintHead(recordnum);
					
					/*
					printf("%d %d\n",curpagenum, setnums);
					const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
					printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
					*/
					
					std::vector<string>::iterator iter = recordList.begin();
					i =0;
					for(; iter!= recordList.end() ; ++iter, ++i)
					{
						if( (i >= sline) && (i < eline ))
						{
							printf("%s", (*iter).c_str());
						}
					}
					recordList.clear();
						
						
				}
				else // startyear < endyear
				{
					NoQueryInfo();
					return;
					
				}
			}
						
				/*
				if( startdate == enddate)
				{
					std::string queryfile = AddPathPrefix(dirpath, TIME_PATH_NAME);
					queryfile = AddPathPrefix(queryfile, startdate);
					ReadCdrFromfile(queryfile.c_str());
				}
				
				else if(startdate < enddate) // 同一个月
				{
				  
					//printf("DispQueryBrief == 950 \n");
					std::string queryfile;
					char str[3] = {0};
					

					startdate = startdate.substr(DAY_TIME_LENGTH-1);
					enddate = enddate.substr(DAY_TIME_LENGTH-1);

					//int days = atoi(endday.c_str()) - atoi(startday.c_str()) + 1;
					//int startdays = atoi(startday.c_str());
					
					int i = 0;
					for(i = 0; i < days; i++)
					{	
						
						memset(str, '\0', sizeof(str));
						queryfile = AddPathPrefix(dirpath, TIME_PATH_NAME);
						sprintf(str, "%02d", startdays);
						queryfile = AddPathPrefix(queryfile, stime.substr(0, (DAY_TIME_LENGTH) - 1) + str);
						ReadCdrFromfile(queryfile.c_str());
					//	printf("file name %s\n", queryfile.c_str());
						startdays++;
						
					}
					
				
				  }
				  */
				
			else{
				NoQueryInfo();
				return ;

			}
			
		}
		else
		{
			ERR_EXIT("need to set -t starttime and endtime\n");
		}
	}

	
	/*
	ReadDir(dirpath,true);
	std::vector<std::string>::iterator it = fileList.begin();
	for(; it != fileList.end(); it++)
	{
		FileListClear();
		Query(it->c_str(),maxrecord);
		filepath = *it;	
		GetQueryList(qmode);  // 依次查询目录下文件,querylistBrief添加符合qmode的cdr记录
	}
	int sno, eno;
	int querysize = querylistBrief.size();
	FindRecordRange(sno, eno, querysize);
	std::list<RecordBrief>::iterator iter;
	iter = querylistBrief.begin();
	for(int i = 0; i < eno; i++, ++iter)
	{
		if(0 == i)
		{
			printf("%d %d\n",curpagenum,querysize);
			const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
			printf("%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
		}
		if(i >=sno && i<eno)
		{
			iter->RecordBriefPrint();;			
		}
	}
	*/
}

void CdrQuery::CmdOpt(int argc,char ** argv)
{
	while ((opt = getopt(argc, argv, "m:f:n:d:i:I:p:s:t:")) > 0) 
	{
		switch (opt) 
		{ 
			case 'm':						// 查询模式 
				mode = atoi(optarg);	
				break;
			case 'f':						// 查文件 
				filepath = optarg;
				break;
			case 'n':						// 查文件中指定cdr记录
				recordno = atoi(optarg);
				break;
			case 'p':						// 设置查询页(默认为0)
				pageno = atoi(optarg);
				break;
			case 's':						// 指定页大小(默认为100)
				pagesize = atoi(optarg);
				break;
			case 'd':						// 查目录
				dirpath=optarg;
				//dirpath.append("/");
				break;
			case 'i':						// 设置主叫号
				q_callingParty = optarg;
				//q_callingParty.insert(0,"tel:");
				break;
			case 'I':						// 设置被叫号	
				q_calledParty = optarg;
				//q_calledParty.insert(0,"tel:");
				break;
			case 't':						// 设置查询时间     传参 "starttime" "endtime"
				stime = optarg;
				etime = argv[optind];
				//printf("stime=%s,etime=%s\n",stime.empty()?"NULL":stime.c_str(),etime.empty()?"NULL" :etime.c_str());
				break;
			default: 
				usage(argv[0]);
				exit(EXIT_SUCCESS);
		}
	}

}


void CdrQuery::atBegin()  //实时更新数据
{

/*
    if (fork() == 0){  
		char argv[128] = {0};
		strcpy(argv, " -d ");
		strcat(argv,dirpath.c_str());
        //child process  
        char * execvp_str[] = {argv ,NULL};  
        if (execvp("prodat",execvp_str) <0 ){  
            ERR_EXIT("error on exec"); 
        }  
    }else{  
        //parent process  
        wait(NULL);  
        printf("execvp done\n\n");  
    }  
*/	
	char argv[128] = {0};
	strcpy(argv, "/opt/utoss/querytools/createindex/bin/createindex -d ");  //根据实际情况配置createindex路径
	strcat(argv,dirpath.c_str());

	system(argv);
  	//printf("update data success!\n");
}


void CdrQuery::DirorFile()
{
	if(filepath.empty() == true && dirpath.empty() == false)  // 查询目录
	{
		
		if(mode == DISP_QUERY_BRIEF)
		{
			atBegin();
			
			
			std::string lockfilepath = AddPathPrefix(dirpath, LOCK_FILE_NAME);  //lockfilepath文件路径
	
			FILE *fp = fopen(lockfilepath.c_str(), "w");	

		//	------------------------------------------------
			int fd = fileno(fp);
			int ret;
			// 加写锁
			ret = lock_file_write(fd);
			//printf("pid=%d\n",getpid());
			if (ret == -1) {
				ERR_EXIT("lock_file_write\n");
			}
			
	
			if (q_calledParty.empty() && q_callingParty.empty()) // -t starttime endtime
			{
				DispQueryBrief(DISP_QUERY_TIME);
			}
			else
			{
				DispQueryBrief(DISP_QUERY_KEY);  
			}
			

			ret = unlock_file(fd);
			if (ret == -1) {
				ERR_EXIT("unlock_file\n");
			}
	
		//	------------------------------------------------
			
			/*
			if(	(q_calledParty.empty() == false) || (q_callingParty.empty() == false))
			{
		   		 DispQueryBrief(DISP_QUERY_CALL);
			}
			else if((stime.empty() == false) || (stime.empty() == false))
			{
				 DispQueryBrief(DISP_QUERY_TIME);
			}
			else
			{
				ERR_EXIT("need to set query mode -t or -i or -I\n");
			}
			*/
			
		}
		else
		{
			ERR_EXIT("need to set dir querymode!\n"
				 "-m 3\n");
		}
		
	}
	else if(filepath.empty() == false && dirpath.empty() == true ) //查询文件
	{
		DispFile(mode);		
	}
	else{
		ERR_EXIT("need to set -f queryfile or -d querydir!\n");
	}
	
}
CdrQuery::~CdrQuery()
{


}

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
	 	return -1;
	 }
     while((ent=readdir(pDir))!=NULL)  
     {  
    // 	puts(ent->d_name);
     	if((ent->d_type & DT_DIR) && brecursive)  
        {  
			if(strncmp(ent->d_name, ".", 1) == 0)
				continue;
			else
			{
				std::string path = AddPathPrefix(dir,ent->d_name);
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
//				printf("%s	",ent->d_name);      
	   			std::string tmp = AddPathPrefix(dir,ent->d_name);
				fileList.push_back(tmp);
//				printf("tmp = %s\n",tmp.c_str());
			}
		}
	 }
	 closedir(pDir);
	 /*
	 std::vector<std::string>::iterator it = fileList.begin();
	 for(; it != fileList.end(); it++)
	 {
		printf("%s\n",it->c_str());	 
	 }
	 */
	return 0;
}

/*
int CdrQuery::Query(const char* filename,int &maxrecord)
{
	std::string str = filename;
	std::string::size_type found = str.find_last_of('.');
	std::string suffix = str.substr(found);
	if(strcmp(suffix.c_str(),".dat"))
	{
		ERR_EXIT("file suffix is not .dat\n");
	}
	//step1 打开文件
	FILE* fp =fopen(filename,"rb");
	if(fp ==NULL)
	{
		ERR_EXIT("file open failed !\n");
	}
	//step2 读取文件头长度
	uint32_t fheadlen = 0;
	int ret = GetFileHeaderLength(fp,fheadlen);
	//printf("[%s] [%d] [cdrquerylength = %d ]\n",__FILE__,__LINE__,fheadlen);
	
	if(ret)
	{
		return ret;
	}
	//step3 读取文件中cdr记录的个数
	uint32_t num = 0;

	ret = GetCdrNum(fp,num);
	maxrecord = num;

	if(ret)
	{
		return ret;
	}
	if(num  == 0)
	{
		fclose(fp);
		return FILE_NO_CDRRECORD;
	}
	//step4 跳过文件头
	rewind(fp);
	fseek(fp,fheadlen,SEEK_SET);
	for(int i = 0; i < num; i++)
	{
		
		//step5 读取cdr的长度
		int16_t cdrlength = 0;
		ret = GetCdrLength(fp,cdrlength);
		if(ret)
		{
			fclose(fp);
			return ret;
		}
		//step6 跳过cdr头部
		fseek(fp,3,SEEK_CUR);
		Decode(fp,cdrlength);
		
//		printf("[ %s ] [ %s ] [ %d ] [ %s ]  cdrnum = %d\n",__FILE__,__func__,__LINE__,str.c_str(),num);
		
		/*
		std::string str = filename;
		std::string::size_type found = str.find_last_of('/');
		if (found != std::string::npos)
		{
			str = str.substr(found+1,str.length()-found-1);
		}	
		CdrfileList.push_back(str);
		
		
	}
	fclose(fp);
	return FILE_QUERY_SUCCESS;
}

*/

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


void CdrQuery::GetBriefList(std::list<RecordBrief>& list, int &startno)
{
	std::vector<CDR*>& cdrlist = cdrList;
	const CASN1Desc& asnDesc = m_asnDesc;	
	int i = 1;
	const char * cfilename = filepath.c_str();
	for(; i <= cdrlist.size() ; i++)
	{
		RecordBrief tmp;
		char  dest[256] = {0};
		tmp.Setfilename(cfilename);
		tmp.SetIndex(i+startno);
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


void CdrQuery::QueryShowBrief(int & startno)
{
	std::list<RecordBrief> brieflist;
	GetBriefList(brieflist,startno);
	std::list<RecordBrief>::iterator it = brieflist.begin();
	//printf("brieflistsize=%d\n", brieflist.size());
	//printf("curpagenum=%d maxrecord=%d\n",curpagenum,maxrecord);
	printf("%d %d\n",curpagenum,maxrecord);
	const char* head[]={"FilePath","RecordNo","RecordType","CallingParty","CalledParty","StarTime","EndTime","localRecordSeq","recordSeq","callduration"};
	printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",head[0],head[1],head[2],head[3],head[4],head[5],head[6],head[7],head[8],head[9]);
	for(; it!= brieflist.end() ; it++)
	{
		it->RecordBriefPrint();
			
	}
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

void CdrQuery::ShowDetail(int index)
{
	std::vector<CDR*>& cdrlist = cdrList;	//cdrList只有一个CDR记录

	nodeList.clear();
	CDR* pCDR = cdrlist[0];  
	CASNTree &treeData = pCDR->data;
	const ASNBinTreeNode* rootNode = treeData.GetElem(NULL, 0);
//	printf("\n----***treeData.Print()***begin----\n\n");
//	treeData.Print();
//	printf("\n----***treeData.Print()***end----\n\n");
	std::string name;
	//printf("\n----ber_show_tree***begin----\n");
	//ber_show_tree(0);
	if(rootNode->tag == 63)
	{
		name = "sCSCFRecord";
		printf("%s\n",name.c_str());
	}
	if(rootNode->tag == 69)
	{
		name = "aSRecord";
		printf("%s\n",name.c_str());
	}
	if(rootNode->tag == 67)
	{
		name = "mGCFRecord";
		printf("%s\n",name.c_str());
	}
	ASNName parentName;
	FillTree(rootNode,parentName,0);
	NewShowTree(nodeList,name.c_str(),0); 
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
	//	printf("GetCdrNum()::read file header failed");
		fclose(fp);
		return FILE_HEADE_ERR;
	}
	unsigned int size = fread(&cdrnum,1,4,fp);
//	printf("CdrQuery::GetCdrNum size of = %d\n",size);
	if(size != 4)
	{
//		printf("GetCdrNum()::read file failed !");
		fclose(fp);
		return FILE_HEADE_ERR;
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
		printf("GetFileHeaderLength()::read file header failed");
		fclose(fp);
		return FILE_HEADE_ERR;
	}
	unsigned int size = fread(&fileheadlen,1,4,fp);
//	printf("CdrQuery::GetFileHeaderLength size of = %d\n",size);
	if(size != 4)
	{
	//	printf("GetFileHeaderLength()::read file failed !");
		fclose(fp);
		return FILE_HEADE_ERR;
	}
	len=ntohl(fileheadlen);
	if(len == fileheadlen)
	{
	//	printf("ntoh flase,file error,now return\n");
		fclose(fp);
		return FILE_HEADE_ERR;
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
		printf("GetCdrHeaderLength()::read file cdr header failed !");
		fclose(fp);
		return FILE_CDR_HEADE_ERR;
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
//	fileList.clear();
//	CdrfileList.clear();
}




