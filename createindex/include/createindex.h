#ifndef __CDRQUERY__H
#define __CDRQUERY__H
#include <stdio.h>
#include <list>
#include <string>
#include <arpa/inet.h>
#include <vector>

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>      


#include "asn1type.h"
#include "sysutil.h"

#include "asntree.h"
#include "bercodec.h"
#include "string2.h"
#include "asn1desc.h"


#define SMALLBUFFSIZE 256
#define BUFFSIZE 1024
#define FREESIZE 5000  // cdrList释放临界值
#define CALL_PREFIX_LENGTH  (sizeof("tel:")-1)
#define TIME_LENGTH (sizeof("2018-10-10") -1)
#define CALLING_PATH_NAME "calling_dir"      //宏定义的索引目录名
#define CALLED_PATH_NAME  "called_dir"
#define TIME_PATH_NAME    "time_dir"
#define LOG_PATH_NAME     "log_dir"
#define INDEX_FILE_NAME   "processed.r"
#define LOCK_FILE_NAME  "lock.h"

#define FORMATDATESIZE (sizeof("2018-12-25 17:55:20")-1) // "2018-12-25 17:55:20"



#define ERR_EXIT(msg) \
	{  \
			fprintf(stderr, "[%s] [%d] %s", __FILE__, __LINE__,	msg);;  \
			exit(EXIT_FAILURE);   \
	} 

#define ERR_MSG(msg) \
	{  \
			fprintf(stderr, "[%s] [%d] %s", __FILE__, __LINE__,	msg);;  \
	} 



//#define BCD(v) (((unsigned char)((v) / 10) << 4) & 0xF0) | ((unsigned char)((v) % 10) & 0x0F)
#define DEBCD(v) (((int)((v/16))*10) + ((int)(v%16)))

const char* const ASN_FILE_NAME		= "/opt/utoss/querytools/createindex/bin/IMSChargingDataTypes.asn";

unsigned char Mkdir(const char *path, mode_t mode);
void Chdir(const char *path);
unsigned char MkResultDirs();


enum IndexDirFlag{  //1 calling目录不存在 2 called目录不存在  4 time目录不存在
	CALLINGDIR_FLAG=1,  
	CALLEDDIR_FLAG=2,
	TIMEDIR_FLAG=4,
};


enum QueryErr
{
	FILE_QUERY_SUCCESS,
	FILE_OPEN_FAIL,
	FILE_HEADE_ERR,
	FILE_CDR_HEADE_ERR,
	FILE_READ_FAIL,
	FILE_NO_CDRRECORD,
	NOFILE_INDIR,
};

class RecordBrief
{

private:
	int         nIndex;
	std::string recordType;
	std::string callingParty;
	std::string calledParty;
	std::string localRecordSeq;
	std::string recordSeq;
	ASN1OCTET   startTime[9];
	ASN1OCTET   endTime[9];
	std::string filename;
	std::string callduration;

public:
	RecordBrief();
	~RecordBrief();
public:

	void 	SetIndex(int index);
	void 	Setfilename(const char* filename);
	void 	SetRecordType(const std::string& type);
	time_t  StringToDatetime(string str);
	std::string  DatetimeToString(time_t time);
	void 	SetCallingParty(char* calling);
	void 	SetCalledParty(char* called);
	void 	SetStartTime(char* starttime);
	void 	SetEndTime(char* endtime);
	
	void 	SetCallDuration(const std::string& duration);
	std::string GetCallDuration() const;
	
	void    WriteIndexFile(const char* path) const;
	void 	WriteFile(const char* querykey) const;
	void 	GetInfoFile(const char* querykey, std::string& str , std::string& filename) const;
	
	ASN1OCTET*  GetStartTime() const;
	ASN1OCTET*  GetEndTime()   const;
	void 	SetLocalRecordSeq(char* lseq);
	void 	SetRecordSeq(char* rseq);
	std::string GetCallingParty() const;
	std::string GetCalledParty() const;
	void    ParseBCDTimeStamp(const ASN1OCTET* src,char* dest) const;
};

struct CDR 
{
	uint8_t rel;
	uint8_t ver;
	uint8_t fmt;
	uint8_t ts;
	uint8_t rel_ext;
	CASNTree data;
};


class CdrQuery
{

public:
	CdrQuery();  // InitArgc InitAsn
	~CdrQuery();
	void 	CmdOpt(int argc,char ** argv);
	void 	ProDir(char ** argv);  //对指定目录生成索引及其辅助文件(*.log process.r)

private:
	void    InitArgc();	
	void 	InitAsn();	


	void 	DecodeDir();
	int     ReadDir(std::string dir,bool  brecursive);
	void 	GetRealFileList(std::vector<std::string> &needprocesslist);
	int  	Query(const char* filename,int& maxrecord);
	void 	CreateRecordIndex(unsigned char flag); 
	void    CreateRecordIndex();
	void    CreateCallingIndex(const std::list<RecordBrief>::const_iterator &it);
	void 	CreateCalledIndex(const std::list<RecordBrief>::const_iterator &it);
	void 	CreateTimeIndex(const std::list<RecordBrief>::const_iterator &it);
	void 	WritetoProcessedFile(const char *filename, const char& c);
	void    WritetoLog(const char* logname, const char* filename, const int &cdrnum); 
	unsigned int 	ProcessRecordIndex(unsigned char flag, const char* logpath);

	
	//void    ReadFromIndexFile(char* str);
	//void 	WriteToIndexFile(const char* str);
	
	std::string AddPathPrefix(const std::string& path,const std::string& filename);
	std::vector<CDR*>& GetCDRList();
	
	void 	FileListClear();
	void 	crd_list_Clear();
	int  	GetCdrNum(FILE* fp,uint32_t& len);
	int  	GetFileHeaderLength(FILE* fp,uint32_t& len);
	int  	GetCdrLength(FILE* fp,int16_t& len);
	void 	GetBriefList(std::list<RecordBrief>& list, const int shift);
	ASN1OCTET* 	Decode(FILE* fp,int cdrlen);
	
	void FillTree(const ASNBinTreeNode* pInsertNode, const ASNName& parentName, int seq);
	const CASN1Desc& GetASN1Desc() const;
	BERCoder* GetBERCoder(const char* name) const;
//	bool BrotherStr(string tocompare1, string tocompare2);
	
private:
	CASN1Desc			m_asnDesc;
	shlist<BERCoder*>	m_asn1_coders;
	//Parsing the AVPs of the CDR file
	std::vector<ShowNode>  nodeList;
	std::vector<CDR*>	cdrList;
	std::vector<std::string> fileList;
	std::vector<std::string> realfilelist;


	std::list<RecordBrief> recordList;
	std::string filepath, dirpath;

	/*
	std::string stime;
	std::string etime;
	*/
	
	int cdrnum, opt; //cdrnum 单个.dat文件cdr总数

};

#endif

