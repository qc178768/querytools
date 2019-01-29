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


#include "asn1type.h"
#include "sysutil.h"

#include "asntree.h"
#include "bercodec.h"
#include "string2.h"
#include "asn1desc.h"


#define SMALLBUFFSIZE 256
#define BUFFSIZE 1024
#define MAX_PATH 128
#define CALLING_PATH_NAME "calling_dir"      //宏定义的索引目录名
#define CALLED_PATH_NAME  "called_dir"
#define TIME_PATH_NAME    "time_dir"

#define TIME_LENGTH  (sizeof("2018-10-10") -1)   
#define DAY_TIME_LENGTH (sizeof ("2018-10-") -1)


#define FORMATDATESIZE 10 // "2018-12-25"
#define LOCK_FILE_NAME  "lock.h"  //锁文件 控制更新dat和查询dat竟态情况



//#define BCD(v) (((unsigned char)((v) / 10) << 4) & 0xF0) | ((unsigned char)((v) % 10) & 0x0F)
#define DEBCD(v) (((int)((v/16))*10) + ((int)(v%16)))

const char* const ASN_FILE_NAME		= "/opt/utoss/querytools/createindex/bin/IMSChargingDataTypes.asn";



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
	friend class CdrQuery;
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
	void 	RecordBriefPrint();
	void 	SetIndex(int index);
	void 	Setfilename(const char* filename);
	void 	SetRecordType(const std::string& type);
	time_t  StringToDatetime(string str);
	string  DatetimeToString(time_t time);
	void 	SetCallingParty(char* calling);
	void 	SetCalledParty(char* called);
	void 	SetStartTime(char* starttime);
	void 	SetEndTime(char* endtime);
	
	void 	SetCallDuration(const std::string& duration);
	std::string GetCallDuration() const;
	
	ASN1OCTET*  GetStartTime() const;
	ASN1OCTET*  GetEndTime()   const;
	void 	SetLocalRecordSeq(char* lseq);
	void 	SetRecordSeq(char* rseq);
	std::string GetCallingParty() const;
	std::string GetCalledParty() const;
private:
	void    ParseBCDTimeStamp(const ASN1OCTET* src,char* dest);
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
	CdrQuery();
	~CdrQuery();
	void 	CmdOpt(int argc,char ** argv);
	void  	DirorFile();
private:
	int 	Query(const char* filename,int& startno, int& endno);
	void 	QueryShowBrief(int & startno);
	int     ReadDir(std::string dir,bool  brecursive);

	void 	DispQueryBrief(unsigned int qmode);
	void 	InitArgc();
	void 	DispFileBrief();
	void 	DispFileDetail();
	void 	DispFile(int mode);
	void 	InitAsn();
	void 	ShowDetail(int index);
	void 	FileListClear();
	std::vector<std::string> fileList;

private:
	void 	NoQueryInfo();
	void    ReadCdrFromfile(const char * path);  
	void    ReadCdrFromfile(const char*  path, const char* calling);  
	int     ReadTimeCdrFromfile(const char * path);  
	void 	PrintHead(const int& setnums);
	
	void 	crd_list_Clear();
	int  	GetCdrNum(FILE* fp,uint32_t& len);
	int  	GetFileHeaderLength(FILE* fp,uint32_t& len);
	int  	GetCdrLength(FILE* fp,int16_t& len);
	
	void 	GetBriefList(std::list<RecordBrief>& list, int &startno);

	void    GetSet(FILE* fp, std::vector<string>& set);  // 单一条件查询(主叫或被叫或某天)
	void	GetTimeSet(FILE* fp, std::vector<string>& set);  //单一条件查询(主叫或者被叫或某天)+(按时间段查询)
	void 	GetGroupSet(FILE* fp, std::vector<string>& set, const char* calling); // (主被叫组合) | (按时间段查询主被叫组合)
		
	ASN1OCTET* 	Decode(FILE* fp,int cdrlen);

	std::string AddPathPrefix(const std::string& path,const std::string& filename);
	//const char* StringFind(char *src, const char *dst);
	void atBegin(); 
	public:
	std::vector<CDR*>& GetCDRList();
	void FillTree(const ASNBinTreeNode* pInsertNode, const ASNName& parentName, int seq);
	const CASN1Desc& GetASN1Desc() const;
	BERCoder* GetBERCoder(const char* name) const;
	
	bool BrotherStr(string tocompare1, string tocompare2);
	int  FindRecordRange(int &startno, int& endno, int &rmax);
	int  GetFileCdrSum(const char* filename,int &maxrecord);
private:
	CASN1Desc			m_asnDesc;
	shlist<BERCoder*>	m_asn1_coders;
	std::vector<CDR*>	cdrList;
	int maxrecord;
	int opt;
	int mode;
	int recordno;
	int pageno;
	int curpagenum;
	int pagesize;
	std::string stime;
	std::string etime;
	
	std::string q_callingParty,q_calledParty;
	std::string filepath,dirpath;
	std::vector<std::string> recordList;   //按时间段查询
	//Parsing the AVPs of the CDR file
	std::vector<ShowNode>  nodeList;
};

#endif

