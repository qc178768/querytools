// bercodec.h

#ifndef __bercodec_h__
#define __bercodec_h__

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include "asn1desc.h"
#include "asntree.h"
#include "string2.h"
#include "shlist.h"
#include <vector>

using namespace std;

#define ASN1_UTAG_BOOLEAN							1
#define ASN1_UTAG_INTEGER							2
#define ASN1_UTAG_BIT_STRING						3
#define ASN1_UTAG_OCTET_STRING						4
#define ASN1_UTAG_NULL								5
#define ASN1_UTAG_OBJECT_IDENTIFIER					6
#define ASN1_UTAG_ObjectDescriptor					7
#define ASN1_UTAG_EXTERNAL_INSTANCE_OF				8
#define ASN1_UTAG_REAL								9
#define ASN1_UTAG_ENUMERATED						10
#define ASN1_UTAG_EMBEDDED_PDV						11
#define ASN1_UTAG_UTF8String						12
#define ASN1_UTAG_RELATIVE_OLD						13

#define ASN1_UTAG_SEQUENCE_SEQUENCE_OF				16
#define ASN1_UTAG_SET_SET_OF						17
#define ASN1_UTAG_NumericString						18
#define ASN1_UTAG_PrintableString					19
#define ASN1_UTAG_TeletexString_T61String			20
#define ASN1_UTAG_VideotexString					21
#define ASN1_UTAG_IA5String							22
#define ASN1_UTAG_UTCTime							23
#define ASN1_UTAG_GeneralizedTime					24
#define ASN1_UTAG_GraphicString						25
#define ASN1_UTAG_VisibleString_ISO64String			26
#define ASN1_UTAG_GeneralString						27
#define ASN1_UTAG_UniversalString					28
#define ASN1_UTAG_CHARACTER_STRING					29
#define ASN1_UTAG_BMPString							30



// Macro for CASNName class
#define OID_PREFIX			"OID"
#define OID_PREFIX_SIZE		(sizeof(OID_PREFIX)-1)
#define SUM_PREFIX			"SUM"
#define SUM_PREFIX_SIZE		(sizeof(SUM_PREFIX)-1)

#define INC_STEP			4





class ASNCdr;
struct ASNNameHandle;

// Return:
//		<0 failed
//		else the handle of the coder
int ber_init(const char* fdesc, const char* fspec, const char* rootname = NULL);
void ber_free(size_t handle);
const ASNNameHandle* ber_getnamehandle(const char* name, size_t handle);
int ber_decode(const ASNNameHandle* hName, const Uchar* raw, Uint rawlen, string2& value, size_t handle);
int ber_encode(const ASNNameHandle* hName, const string2& value, Uchar* raw, Uint& rawlen, size_t handle);
int ber_seperator(const ASNCdr* pCdr, const ASNNameHandle*& hName, size_t handle);
int ber_show_tree(size_t handle);
int ber_show_name(size_t handle);
int ber_show_hash(size_t handle);
int ber_getshortname(const char* longname, string2& shortname, size_t handle);
int ber_getlongname(const char* shortname, string2& longname, size_t handle);

//add by shirly
int NewShowTree(std::vector<ShowNode>& node,const char* name,size_t handle);

const char* StringFind(char *src, const char *dst);


//------------------------------------------------------------------
// Decode & encode function type.
//------------------------------------------------------------------


typedef int (*ber_decoder)(const Uchar*, Uint, string2&);
typedef int (*ber_encoder)(const string2&, Uchar*, Uint&);

struct BERCoder
{
	char		name[256];		// BER Coder name
	ber_decoder	decode;			// Decode function
	ber_encoder	encode;		// Encode function
	char		desc[1024];		// Description of the codec
};


//------------------------------------------------------------------
// OID Name parser class
//------------------------------------------------------------------
class UIBuff
{
	Uint*		m_buff;
	Uint		m_buffsize;
	Uint		m_size;

protected:
	void expand(Uint inc) {
		if (m_size+inc>m_buffsize) {
			if (inc<INC_STEP) {
				inc = INC_STEP;
			}
			m_buffsize += inc;
			Uint* nbuff = new Uint[m_buffsize];
			if (m_buff) {
				for (register size_t i = 0; i<m_size; i++) {
					nbuff[i] = m_buff[i];
				}
				delete []m_buff;
			}
			m_buff = nbuff;
		}
	}

public:
	UIBuff(): m_buff(NULL), m_buffsize(0), m_size(0) {
		// Nothing to do here
	}

	UIBuff(const UIBuff& other): m_buff(NULL), m_buffsize(0), m_size(0) {
		*this = other;
	}

	UIBuff& operator =(const UIBuff& other) {
		if (this!=&other) {
			set(other.m_buff, other.m_size);
		}
		return *this;
	}

	~UIBuff() {
		if (m_buff) {
			delete []m_buff;
		}
	}

	void clear() {
		m_size = 0;
	}

	void set(const Uint* buff, Uint count) {
		clear();
		for (register size_t i = 0; i<count; i++) {
			append(buff[i]);
		}
	}

	void append(Uint tag) {
		expand(1);
		m_buff[m_size++] = tag;
		//printf("[ %s ] [ %s ] [ %d ] m_size = %d m_buff[%d] = %d\n",__FILE__,__func__,__LINE__,m_size,--m_size,m_buff[m_size]);
	}

	Uint size() const { return m_size; }

	const Uint* get() const { return m_buff; }

	Uint& operator [](Uint index) {
		return m_buff[index];
	}

	Uint get(Uint index) const {
		if (0<=index && index<m_size) {
			return m_buff[index];
		} else {
			return Uint(-1);
		}
	}
};


class ASNName
{
	string2		m_name;
	UIBuff		m_oids;

protected:
	bool set_str(string2& name, const Uint* iname, Uint isize) const;
	bool set_int(UIBuff& iname, const char* name) const;

public:
	ASNName();
	ASNName(const char* name);
	ASNName(const Uint* oids, Uint oidscount);
	ASNName(const ASNName& other);
	ASNName& operator =(const ASNName& other);
	~ASNName();

	bool set(const char* name);
	bool set(const Uint* oids, Uint oidscount);
	bool append(const Uint* oids, Uint oidscount);
	bool append(const ASNName& other);

	const char* str() const { return m_name.c_str(); }
	operator const char* () const { return m_name.c_str(); }
	Uint oids(Uint index) const { return m_oids.get(index); }
	const Uint* oids() const { return m_oids.get(); }
	Uint size() const { return m_oids.size(); }
	bool is_valid() const { return size()>0; }
};





struct ASNNameHandle
{
	ASNName			asnname;
	const BERCoder*	pbercoder;

public:
	ASNNameHandle();
	ASNNameHandle(const ASNNameHandle& other);
	ASNNameHandle(const ASNName& asnn, const BERCoder* pberc);
	~ASNNameHandle();
};

struct ASN1Coder
{
	char		asn1_name[256];		//	ASN.1 desc name
	char		coder_name[256];	//	Ber coder/decoder name, reference to BERCoder::name
};


class CASN1Coder
{
	typedef BERCoder*				PBERCoder;
	typedef shlist<PBERCoder>		CNameCoders;
	typedef shlist<ASNNameHandle>	CNameHandles;
	typedef std::vector<string2>	CSeperators;
	
	


	CSeperators		m_seperators;
	CNameHandles	m_namehandles;
	CNameHandles	m_fasthandles;
	CNameCoders		m_asn1_coders;
	CNameCoders		m_spec_coders;
	CASN1Desc		m_desc;

protected:
	static BERCoder		s_BERCoders[];
	static ASN1Coder	s_ASN1Coders[];

protected:
	int load_StandardBERCoder();
	int load_SpecificBERCoder(const char* fname);

	bool link_BERCoder(const ASN1_NODE* pNode, string2 full_name = "", string2 short_name = "OID");
	PBERCoder find_NamedBERCoder(const string2& full_name, const string2& type_name);
	PBERCoder find_SubNamedBERCoder(CNameCoders& coders, const string2& full_name);

	int find_ShortName(const ASN1_NODE* pNode, const char* longname, string2& shortname) const;
	int find_LongName(const ASN1_NODE* pNode, const size_t* iNames, int iNamesLen, string2& longname) const;

	const ASN1_NODE* get_ChildOf(const ASN1_NODE* pParent, const char* name, size_t len) const;
	const ASN1_NODE* get_ChildOf(const ASN1_NODE* pParent, size_t tag) const;

	void get_StringOid(const size_t* iNames, int iNamesLen, string2& sName) const;


	int add_BERCoder(const char* name, const char* type);
	int add_BERCoder(const char* name, PBERCoder pBERCoder);

	bool add_UTAG(const ASN1_NODE* pNode, string2& shortname, const char** pnext = NULL) const;
	bool skip_UTAG(const ASN1_NODE* pNode, const size_t*& iNames, int& iNamesLen) const;

	const ASN1_NODE* get_ActualType(const ASN1_NODE* pNode) const;


protected:
	static BERCoder* GetBERCoder(const char* name);

public:
	CASN1Coder();
	~CASN1Coder();

	//-------------------------------------------------------------------------
	//	Function:
	//		Initialize this object.
	//	Parameters:
	//		fdesc - INPUT, The ASN.1 description file name.
	//		fspec - INPUT, The specific file name, can be NULL.
	//	Return:
	//		0 - Success.
	//		else - Failed.
	int Init(const char* fdesc, const char* fspec, const char* rootname = NULL);

	//-------------------------------------------------------------------------
	//	Function:
	//		Free this object.
	void Free();

	const ASNNameHandle* get_namehandle(const char* name);

	//-------------------------------------------------------------------------
	//	Function:
	//		Decode the BER encoded fields value to string format.
	//	Parameters:
	//		name - INPUT, OID name of the field (format is 'OID.a.b.c...').
	//		raw - INPUT, The start address of BER encoded value.
	//		rawlen - INPUT, The BER encoded value size.
	//		value - OUTPUT, The string format value.
	//	Return:
	//		0 - Success.
	//		else - Failed.
	int decode(const ASNNameHandle* hName, const Uchar* raw, Uint rawlen, string2& value);

	//-------------------------------------------------------------------------
	//	Function:
	//		Encode the string format value to BER encoded value.
	//	Parameters:
	//		name - INPUT, OID name of the field (format is 'OID.a.b.c...').
	//		value - INPUT, The string format value.
	//		raw - OUTPUT, The start address of BER encoded value.
	//		rawlen - INPUT/OUTPUT, The BER encoded value size.
	//	Return:
	//		0 - Success.
	//		else - Failed.
	int encode(const ASNNameHandle* hName, const string2& value, Uchar* raw, Uint& rawlen);

	//-------------------------------------------------------------------------
	//	Function:
	//		Find the 'SEQUENCE OF' element in CDR, return the count of
	//		elements and the name of the element.
	//	Parameters:
	//		pCdr - INPUT, The CDR object.
	//		seperator - OUTPUT, The name of the 'SEQUENCE OF' element.
	//	Return:
	//		Count of the 'SEQUENCE OF' elements.

	//int get_seperator(const ASNCdr* pCdr, const ASNNameHandle*& hNameSep);


	// Other assistants
	int get_shortname(const char* longname, string2& shortname) const;
	int get_longname(const char* shortname, string2& longname) const;

	void Print(FILE* fp = stdout) const;
	void PrintTree(FILE* fp = stdout) const;
	void PrintHash(FILE* fp = stdout) const;
	// add by shirly
	void NewPrintTree(std::vector<ShowNode>& node,const char*name);
	void FirstToUpper(string2& trans_value);
};


class CASN1CoderList
{
	typedef CASN1Coder* 			PASN1Coder;
	typedef std::vector<PASN1Coder>	TASN1CoderList;
	TASN1CoderList	m_list;

protected:
	bool IsHandle(size_t handle) const { return (handle<m_list.size()) ? m_list[handle]!=NULL : false; }
	CASN1Coder* GetCoder(size_t handle) { return m_list[handle]; }
	const CASN1Coder* GetCoder(size_t handle) const { return m_list[handle]; }

public:
	CASN1CoderList();
	virtual ~CASN1CoderList();

	int AddCoder(const char* fdesc, const char* fspec, const char* rootname = NULL);
	void DelCoder(size_t handle);

	const ASNNameHandle* GetNameHandle(const char* name, size_t handle) {
		return GetCoder(handle)->get_namehandle(name);
	}

	int Decode(const ASNNameHandle* hName, const Uchar* raw, Uint rawlen, string2& value, size_t handle) {
		return GetCoder(handle)->decode(hName, raw, rawlen, value);
	}

	int Encode(const ASNNameHandle* hName, const string2& value, Uchar* raw, Uint& rawlen, size_t handle)	 {
		return GetCoder(handle)->encode(hName, value, raw, rawlen);
	}

// 	int GetSep(const ASNCdr* pCdr, const ASNNameHandle*& hName, size_t handle) {
// 		return GetCoder(handle)->get_seperator(pCdr, hName);
// 	}

	int GetShortName(const char* longname, string2& shortname, size_t handle) const;
	int GetLongName(const char* shortname, string2& longname, size_t handle) const;

	// Other assistants
	// add by shirly
	int Show(std::vector<ShowNode>& node,const char* name,size_t handle) ;
	int ShowTree(size_t handle) const;
	int ShowName(size_t handle) const;
	int ShowHash(size_t handle) const;
	void Clear();


};






//------------------------------------------------------------------
// Decode & encode functions
//------------------------------------------------------------------

int ber_decode_default(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_default(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_STRING(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_STRING(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_UINT(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_UINT(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_UINT4(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_UINT4(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_UINT3(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_UINT3(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_UINT2(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_UINT2(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_UINT1(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_UINT1(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_TimeStamp(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_TimeStamp(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_TimeStamp2(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_TimeStamp2(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_IPv4(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_IPv4(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_IPv6(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_IPv6(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_BCD(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_BCD(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_NBCD(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_NBCD(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_TBCD(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_TBCD(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_BCDDirNo(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_BCDDirNo(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_BCDTelNo(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_BCDTelNo(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_AddressString(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_AddressString(const string2& value, Uchar* raw, Uint& rawlen);

int ber_decode_TeleserviceCode(const Uchar* raw, Uint rawlen, string2& value);
int ber_encode_TeleserviceCode(const string2& value, Uchar* raw, Uint& rawlen);


#endif


