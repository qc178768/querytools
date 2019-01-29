// bercodec.cpp
// History:
//	2004-12-10 Mike Liu	Do NOT check the time-zone to avoid bad
//				source CDR TimeStamp encoding.
//	2005-05-16 Dennis Li	Modify add_UTAG and skip_UTAG for node which is the child of 'sequence(set) of' type;
//----------------------------------------------------------------------------
//#include "stdafx.h"
#include "bercodec.h"
#include "tools.h"
#include "tokpsr.h"
#include <errno.h>
#include <vector>


using namespace std;


#define HASH_MAX		8191
#define MODULE_NAME		"ASN.1 Coder"
#define ASN1_SPEC_SEPERATOR	"seperator"
#define TBCD_DICT		"0123456789*#abc"
#define BCD_DICT		"0123456789ABCDEF"



/*****************************************************************************
Function:
	Siwtch high quart and low quart of a octet.
	For example: quart_switch(0xAB) returns 0xBA.
*****************************************************************************/
inline Uchar quart_switch(Uchar c) {
	return (((c&0xf0)>>4)|((c&0x0f)<<4));
}

/*****************************************************************************
Function:
	Convert a BCD encoded unsigned char to dec value.
*****************************************************************************/
inline Uchar bcd_to_dec(Uchar c) {
	register Uchar temp = ((c&0xf0)>>4);
	return ((temp<<3)+(temp<<1))+(c&0x0f);
}

/*****************************************************************************
Function:
	Check if value of a BCD encoded unsigned char between
	the ragne.
*****************************************************************************/
inline bool bcd_between(Uchar c, int low, int high)
{
	Uchar dc = bcd_to_dec(c);
	return (dc>=low && dc<=high);
}

/*****************************************************************************
Function:
	Check if a time stamp encoded is valid.
History:
	2004-12-10 Mike Liu	Ignore the time-zone part checking, to enhance
				bad encoded time!
*****************************************************************************/
inline bool check_TimeStamp(const Uchar* raw)
{
	return 	bcd_between(raw[0], 0, 38) &&	// Year
		bcd_between(raw[1], 1, 12) &&	// Month
		bcd_between(raw[2], 1, 31) &&	// Day
		bcd_between(raw[3], 0, 23) &&	// HH
		bcd_between(raw[4], 0, 59) &&	// MM
		bcd_between(raw[5], 0, 59)	// &&	// SS
//		bcd_between(raw[7], 0, 23) &&	//
//		bcd_between(raw[8], 0, 59)
		;
}

inline Uchar tbcd_quart(const char* dict, char c) {
	register const char* p = dict;
	while (*p ) {
		if (*p==c) {
			break;
		}
		p++;
	}
	return p-dict;
}

inline Uint yy2yyyy(Uchar yy) {
	return ((yy>40) ? (1900+yy) : (2000+yy));
}


inline Uint yyyy2yy(Uint yyyy) {
	return (yyyy>=2000 ? (yyyy-2000) : (yyyy-1900));
}

inline char bcd_hi(Uchar c, const char* dict = BCD_DICT) {
	return dict[(c&0xf0)>>4];
}

inline char bcd_lo(Uchar c, const char* dict = BCD_DICT) {
	return dict[c&0x0f];
}


inline Uchar mkbcd(const char* p) {
	return ((*p-'0')<<4)+(*(p+1)-'0');
}

const char* gASN1SpecToken[] =
{
	"::=",
	NULL
};


CASN1CoderList	gASN1Coders;





//------------------------------------------------------------------
//
// Implement of exported global functions
//
//------------------------------------------------------------------

int ber_init(const char* fdesc, const char* fspec, const char* rootname/*=NULL*/)
{
	return gASN1Coders.AddCoder(fdesc, fspec, rootname);
}

void ber_free(size_t handle)
{
	gASN1Coders.DelCoder(handle);
}

const ASNNameHandle* ber_getnamehandle(const char* name, size_t handle)
{
	return gASN1Coders.GetNameHandle(name, handle);
}

int ber_decode(const ASNNameHandle* hName, const Uchar* raw, Uint rawlen, string2& value, size_t handle)
{
	value.clear();
	return gASN1Coders.Decode(hName, raw, rawlen, value, handle);
}

int ber_encode(const ASNNameHandle* hName, const string2& value, Uchar* raw, Uint& rawlen, size_t handle)
{
	return gASN1Coders.Encode(hName, value, raw, rawlen, handle);
}

int ber_seperator(const ASNCdr* pCdr, const ASNNameHandle*& hName, size_t handle)
{
	//return gASN1Coders.GetSep(pCdr, hName, handle);
	return 0;
}

int ber_show_tree(size_t handle)
{
	return gASN1Coders.ShowTree(handle);
}

int ber_show_name(size_t handle)
{
	return gASN1Coders.ShowName(handle);
}

int ber_show_hash(size_t handle)
{
	return gASN1Coders.ShowHash(handle);
}

int ber_getshortname(const char* longname, string2& shortname, size_t handle)
{
	return gASN1Coders.GetShortName(longname, shortname, handle);
}

int ber_getlongname(const char* shortname, string2& longname, size_t handle)
{
	return gASN1Coders.GetLongName(shortname, longname, handle);
}

// add by shirly
int NewShowTree(std::vector<ShowNode>& sublist,const char* name,size_t handle)
{
	return gASN1Coders.Show(sublist,name,handle);
}



//----------------------------------------------------------------------------
//
// Implementation for transfers
//
//----------------------------------------------------------------------------

BERCoder CASN1Coder::s_BERCoders[] =
{
//	Format name			Parser function			Builder function		Comments
//------------------------------------------------------------------------------------------------------------------------------------
	{"default",			ber_decode_default,		ber_encode_default,		"Same as RAW."},
	{"RAW",				ber_decode_default,		ber_encode_default,		"Hex data."},
	{"STRING",			ber_decode_STRING,		ber_encode_STRING,		"Plain text."},
	{"UINT",			ber_decode_UINT,		ber_encode_UINT,		"Unsigned int (1,2,3 or 4 bytes)."},
	{"UINT4",			ber_decode_UINT4,		ber_encode_UINT4,		"Unsigned int (4 bytes)."},
	{"UINT2",			ber_decode_UINT2,		ber_encode_UINT2,		"Unsigned int (3 bytes)."},
	{"UINT3",			ber_decode_UINT3,		ber_encode_UINT3,		"Unsigned int (2 bytes)."},
	{"UINT1",			ber_decode_UINT1,		ber_encode_UINT2,		"Unsigned int (1 bytes)."},
	{"TimeStamp",			ber_decode_TimeStamp,		ber_encode_TimeStamp,		"TimeStamp of UTCTime."},
	{"TimeStamp2",			ber_decode_TimeStamp2,		ber_encode_TimeStamp2,		"TimeStamp of UTCTime reverse order."},
	{"IPv4",			ber_decode_IPv4,		ber_encode_IPv4,		"IPv4 address."},
	{"IPv6",			ber_decode_IPv6,		ber_encode_IPv6,		"IPv6 address."},
	{"BCD",				ber_decode_BCD,			ber_encode_BCD,			"BCD encoded/decode data."},
	{"NBCD",			ber_decode_NBCD,		ber_encode_NBCD,		"Negative BCD encoded/decode data."},
	{"TBCD",			ber_decode_TBCD,		ber_encode_TBCD,		"TBCD encoded/decode data."},
	{"TelNo",			ber_decode_BCDTelNo,		ber_encode_BCDTelNo,		"BCDDirectoryNumber encoded with octet 1, 2."},
	{"BCDDirectoryNumber",		ber_decode_BCDDirNo,		ber_encode_BCDDirNo,		"BCDDirectoryNumber encoded without octet 1, 2."},
	{"AddressString",		ber_decode_AddressString,	ber_encode_AddressString,	"AddressString format."},
	{"TeleserviceCode",		ber_decode_TeleserviceCode,	ber_encode_TeleserviceCode,	"<Group>|<Service>."}
};

BERCoder* CASN1Coder::GetBERCoder(const char* name)
{
	for (int i = 0; i<dim(s_BERCoders); i++) {
		if (strcmp(s_BERCoders[i].name, name)==0) {
			return s_BERCoders+i;
		}
	}
	return NULL;
}

ASN1Coder CASN1Coder::s_ASN1Coders[] =
{
//	Name in ASN.1 Desc		Coder/Decoder (Ref to table gBERCoders)
//----------------------------------------------------------------------------
	{"BOOLEAN",					"UINT"},
	{"INTEGER",					"UINT"},
	{"ENUMERATED",					"UINT"},
	{"UTF8String",				"STRING"},
	{"GraphicString",				"STRING"},
	{"IA5String",					"STRING"},
	{"OCTET STRING",				"STRING"},
	{"TimeStamp",					"TimeStamp"},
	{"IPBinaryAddress.iPBinV4Address",		"IPv4"},
	{"IPBinaryAddress.iPBinV6Address",		"IPv6"},
	{"TBCD-STRING",					"TBCD"},
	{"BCDDirectoryNumber",				"BCDDirectoryNumber"},
	{"AddressString",				"AddressString"}
};




//----------------------------------------------------------------------------
// Converter functions
//----------------------------------------------------------------------------



int ber_decode_default(const Uchar* raw, Uint rawlen, string2& value)
{

	char temp[4];
	temp[2] = 0;
	for (register size_t i = 0; i<rawlen; i++) {
		temp[0] = bcd_hi(raw[i]);
		temp[1] = bcd_lo(raw[i]);
		value += temp;
	}
	return 0;
}

int ber_encode_default(const string2& value, Uchar* raw, Uint& rawlen)
{
	register int i = 0;
	char* last = 0;
	const char* p = value.c_str();
	char cTemp[3];
	cTemp[2] = 0;
	while (*p) {
		if (i>=rawlen) {
			return -1;
		}
		cTemp[0] = *p;
		p++;
		cTemp[1] = *p;
		if (*p) {
			p++;
		}
		raw[i++] = strtoul(cTemp, &last, 16);
	}
	rawlen = i;
	return 0;
}

int ber_decode_STRING(const Uchar* raw, Uint rawlen, string2& value)
{
	for (register size_t i = 0; i<rawlen; i++) {
		value += char(raw[i]);
	}
	return 0;
}

int ber_encode_STRING(const string2& value, Uchar* raw, Uint& rawlen)
{
	register int i = 0;
	const char* p = value.c_str();
	while (*p) {
		if (i>=rawlen) {
			return -1;
		}
		raw[i++] = (Uchar)*p++;
	}
	rawlen = i;
	return 0;
}

int ber_decode_UINT(const Uchar* raw, Uint rawlen, string2& value)
{
	switch (rawlen) {
	case 1:
		return ber_decode_UINT1(raw, rawlen, value);
	case 2:
		return ber_decode_UINT2(raw, rawlen, value);
	case 3:
		return ber_decode_UINT3(raw, rawlen, value);
	case 4:
		return ber_decode_UINT4(raw, rawlen, value);
	default:
		return ber_decode_default(raw, rawlen, value);
	}
}

int ber_encode_UINT(const string2& value, Uchar* raw, Uint& rawlen)
{
	Uint ui = atoi(value.c_str());
	if (ui<=0xff) {		// 1 byte
		return ber_encode_UINT1(value, raw, rawlen);
	} else if (ui<=0xffff) {	// 2 bytes
		return ber_encode_UINT2(value, raw, rawlen);
	} else if (ui<=0xffffff) {	// 3 bytes
		return ber_encode_UINT3(value, raw, rawlen);
	} else {	// 4 bytes
		return ber_encode_UINT4(value, raw, rawlen);
	}
}

int ber_decode_UINT4(const Uchar* raw, Uint rawlen, string2& value)
{
	unsigned int n = 0;
	if (rawlen!=sizeof(n)) {
		return -1;
	}
	memcpy(&n, raw, rawlen);
	char temp[32];
	myutostr(temp, ntohl(n));
	value = temp;
	return 0;
}

int ber_encode_UINT4(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<4) {
		return -1;
	}
	rawlen = 4;
	Uint ui = atoi(value.c_str());
	ui = htonl(ui);
	memcpy(raw, &ui, 4);
	return 0;
}

int ber_decode_UINT3(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=3) {
		return -1;
	}
	Uchar buff[4];
	memset(buff, 0, sizeof(buff));
	memcpy(buff+1, raw, rawlen);
	return ber_decode_UINT4(buff, sizeof(buff), value);
}

int ber_encode_UINT3(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<3) {
		return -1;
	}
	rawlen = 3;
	Uint ui = atoi(value.c_str());
	ui = htonl(ui);
	memcpy(raw, ((char*)&ui)+1, 3);
	return 0;
}

int ber_decode_UINT2(const Uchar* raw, Uint rawlen, string2& value)
{
	unsigned short n = 0;
	if (rawlen!=sizeof(n)) {
		return -1;
	}
	memcpy(&n, raw, rawlen);
	char temp[32];
	myutostr(temp, (Uint)ntohs(n));
	value = temp;
	return 0;
}

int ber_encode_UINT2(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<2) {
		return -1;
	}
	rawlen = 2;
	Uint ui = atoi(value.c_str());
	unsigned short uh = (unsigned short)ui;
	uh = htons(uh);
	memcpy(raw, &uh, 2);
	return 0;
}

int ber_decode_UINT1(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=1) {
		return -1;
	}
	char temp[32];
	myutostr(temp, (Uint)raw[0]);
	value = temp;
	return 0;
}

int ber_encode_UINT1(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<1) {
		return -1;
	}
	rawlen = 1;
	*raw = (Uchar)atoi(value.c_str());
	return 0;
}

/*************************************************************************
TimeStamp					::= OCTET STRING (SIZE(9))
	--
	-- The contents of this field are a compact form of the UTCTime format
	-- containing local time plus an offset to universal time. Binary coded
	-- decimal encoding is employed for the digits to reduce the storage and
	-- transmission overhead
	-- e.g. YYMMDDhhmmssShhmm
	-- where
	-- YY 	= 	Year 00 to 99		BCD encoded
	-- MM 	= 	Month 01 to 12 		BCD encoded
	-- DD	=	Day 01 to 31		BCD encoded
	-- hh	=	hour 00 to 23		BCD encoded
	-- mm	=	minute 00 to 59		BCD encoded
	-- ss	=	second 00 to 59		BCD encoded
	-- S	=	Sign 0 = "+", "-"	ASCII encoded
	-- hh	=	hour 00 to 23		BCD encoded
	-- mm	=	minute 00 to 59		BCD encoded
	--
*************************************************************************/
int ber_decode_TimeStamp(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=9) {
		return -1;
	}
	if (check_TimeStamp(raw)) {
		char cTemp[32];
		char* p = cTemp;
		if (raw[0]>=40) {
			*p++ = '1';
			*p++ = '9';
		} else {
			*p++ = '2';
			*p++ = '0';
		}
		// yy
		*p++ = bcd_hi(raw[0]);
		*p++ = bcd_lo(raw[0]);
		// mm
		*p++ = bcd_hi(raw[1]);
		*p++ = bcd_lo(raw[1]);
		// dd
		*p++ = bcd_hi(raw[2]);
		*p++ = bcd_lo(raw[2]);
		// HH
		*p++ = bcd_hi(raw[3]);
		*p++ = bcd_lo(raw[3]);
		// MM
		*p++ = bcd_hi(raw[4]);
		*p++ = bcd_lo(raw[4]);
		// SS
		*p++ = bcd_hi(raw[5]);
		*p++ = bcd_lo(raw[5]);
		// +/-
		if (raw[6]!='+'&&raw[6]!='-') {
			return -1;
		}
		*p++ = (char)raw[6];
		// HH
		*p++ = bcd_hi(raw[7]);
		*p++ = bcd_lo(raw[7]);
		// MM
		*p++ = bcd_hi(raw[8]);
		*p++ = bcd_lo(raw[8]);

		*p = 0;

		value = cTemp;

		return 0;
	} else {
		return ber_decode_TimeStamp2(raw, rawlen, value);
	}
}

int ber_encode_TimeStamp(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<9) {
		return -1;
	}
	if (value.size()<19) {	// yyyymmddHHMMSS+/-HHMM
		return -1;
	}
	rawlen = 9;
	const char* p = value.c_str()+2;	// Skip century
	// yy
	raw[0] = mkbcd(p); p+=2;
	// mm
	raw[1] = mkbcd(p); p+=2;
	// dd
	raw[2] = mkbcd(p); p+=2;
	// HH
	raw[3] = mkbcd(p); p+=2;
	// MM
	raw[4] = mkbcd(p); p+=2;
	// SS
	raw[5] = mkbcd(p); p+=2;
	// +/1
	if (*p!='+'&&*p!='-') {
		return -1;
	}
	raw[6] = *p; p++;
	// HH
	raw[7] = mkbcd(p); p+=2;
	// MM
	raw[8] = mkbcd(p);

	return 0;
}


/*****************************************************************************
TimeStamp2
	-- TimeStamp with reverse quart order, that is, the high digit in
	-- the 4321 bits and low digit in 8765 bits.
*****************************************************************************/
int ber_decode_TimeStamp2(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=9) {
		return -1;
	}
	char cTemp[32];
	char* p = cTemp;
	if (quart_switch(raw[0])>=40) {
		*p++ = '1';
		*p++ = '9';
	} else {
		*p++ = '2';
		*p++ = '0';
	}
	// yy
	*p++ = bcd_lo(raw[0]);
	*p++ = bcd_hi(raw[0]);
	// mm
	*p++ = bcd_lo(raw[1]);
	*p++ = bcd_hi(raw[1]);
	// dd
	*p++ = bcd_lo(raw[2]);
	*p++ = bcd_hi(raw[2]);
	// HH
	*p++ = bcd_lo(raw[3]);
	*p++ = bcd_hi(raw[3]);
	// MM
	*p++ = bcd_lo(raw[4]);
	*p++ = bcd_hi(raw[4]);
	// SS
	*p++ = bcd_lo(raw[5]);
	*p++ = bcd_hi(raw[5]);
	// +/-
	if (raw[6]!='+'&&raw[6]!='-') {
		return -1;
	}
	*p++ = (char)raw[6];
	// HH
	*p++ = bcd_lo(raw[7]);
	*p++ = bcd_hi(raw[7]);
	// MM
	*p++ = bcd_lo(raw[8]);
	*p++ = bcd_hi(raw[8]);

	*p = 0;

	value = cTemp;
	return 0;
}

int ber_encode_TimeStamp2(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (ber_encode_TimeStamp(value, raw, rawlen)!=0) {
		return -1;
	}
	raw[0] = quart_switch(raw[0]);
	raw[1] = quart_switch(raw[1]);
	raw[2] = quart_switch(raw[2]);
	raw[3] = quart_switch(raw[3]);
	raw[4] = quart_switch(raw[4]);
	raw[5] = quart_switch(raw[5]);
	raw[7] = quart_switch(raw[7]);
	raw[8] = quart_switch(raw[8]);
	return 0;
}

int ber_decode_IPv4(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=4) {
		return -1;
	}
	char temp[128];
	sprintf(temp, "%lu.%lu.%lu.%lu",
		(Uint)raw[0],
		(Uint)raw[1],
		(Uint)raw[2],
		(Uint)raw[3]);
	value = temp;
	return 0;
}

int ber_encode_IPv4(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<4) {
		return -1;
	}
	const char* p = value.c_str();
	char* lasts = NULL;

	raw[0] = strtoul(p, &lasts, 10);
	if (*lasts!='.') {
		return -1;
	} else {
		p = lasts+1;
	}

	raw[1] = strtoul(p, &lasts, 10);
	if (*lasts!='.') {
		return -1;
	} else {
		p = lasts+1;
	}

	raw[2] = strtoul(p, &lasts, 10);
	if (*lasts!='.') {
		return -1;
	} else {
		p = lasts+1;
	}

	raw[3] = strtoul(p, &lasts, 10);
	rawlen = 4;
	return 0;
}

int ber_decode_IPv6(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen!=16) {
		return -1;
	}
	char cTemp[256];
	unsigned short hu = 0;
	for (register const Uchar* p = raw; p<raw+rawlen; p += sizeof(hu)) {
		memcpy(&hu, p, sizeof(hu));
		sprintf(cTemp, "%hx", ntohs(hu));
		if (value.size()>0) {
			value += ':';
		}
		value += cTemp;
	}
	return 0;
}

int ber_encode_IPv6(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<16) {
		return -1;
	}
	const char* p = value.c_str();
	unsigned short hu = 0;
	char* last = NULL;
	Uchar* q = raw;
	rawlen = 0;
	while (*p) {
		hu = strtoul(p, &last, 16);
		hu = htons(hu);
		memcpy(q, &hu, sizeof(hu));
		q += sizeof(hu);
		rawlen += sizeof(hu);
		if (*last==':') {
			last++;
		}
		p = last;
	}
	if (rawlen!=16) {
		return -1;
	}
	return 0;
}

int ber_decode_BCD(const Uchar* raw, Uint rawlen, string2& value)
{
	for (Uint i = 0; i<rawlen; i++) {
		value += bcd_hi(raw[i]);
		value += bcd_lo(raw[i]);
	}
	return 0;
}

int ber_encode_BCD(const string2& value, Uchar* raw, Uint& rawlen)
{
	const char* p = value.c_str();
	int i = 0;
	for (;;) {
		if (i>=rawlen) {
			return -1;
		}
		if (*p==0) {
			break;
		}

		raw[i] = 0;

		// High quart
		if ('0'<=*p && *p<='9') {
			raw[i] |= ((*p-'0')<<4);
		} else if ('A'<=*p && *p<='F') {
			raw[i] |= (((*p-'A')+10)<<4);
		} else if ('a'<=*p && *p<='f') {
			raw[i] |= (((*p-'a')+10)<<4);
		}
		p++;
		if (*p==0) {
			i++;
			break;
		}
		// Low quart
		if ('0'<=*p && *p<='9') {
			raw[i] |= (*p-'0');
		} else if ('A'<=*p && *p<='F') {
			raw[i] |= ((*p-'A')+10);
		} else if ('a'<=*p && *p<='f') {
			raw[i] |= ((*p-'a')+10);
		}
		p++;
		if (*p==0) {
			i++;
			break;
		}
		if (*p==' ') {
			p++;
		}
		i++;
	}
	rawlen = i;
	return 0;
}

int ber_decode_NBCD(const Uchar* raw, Uint rawlen, string2& value)
{
	for (Uint i = 0; i<rawlen; i++) {
		value += bcd_lo(raw[i]);
		value += bcd_hi(raw[i]);
	}
	return 0;
}

int ber_encode_NBCD(const string2& value, Uchar* raw, Uint& rawlen)
{
	const char* p = value.c_str();
	int i = 0;
	for (;;) {
		if (i>=rawlen) {
			return -1;
		}
		if (*p==0) {
			break;
		}

		raw[i] = 0;

		// High quart
		if ('0'<=*p && *p<='9') {
			raw[i] |= 0x0f & (*p-'0');
		} else if ('A'<=*p && *p<='F') {
			raw[i] |= 0x0f & ((*p-'A')+10);
		} else if ('a'<=*p && *p<='f') {
			raw[i] |= 0x0f & ((*p-'a')+10);
		}
		p++;
		if (*p==0) {
			i++;
			break;
		}
		// Low quart
		if ('0'<=*p && *p<='9') {
			raw[i] |= 0xf0 & ((*p-'0')<<4);
		} else if ('A'<=*p && *p<='F') {
			raw[i] |= 0xf0 & (((*p-'A')+10)<<4);
		} else if ('a'<=*p && *p<='f') {
			raw[i] |= 0xf0 & (((*p-'a')+10)<<4);
		}
		p++;
		if (*p==0) {
			i++;
			break;
		}
		if (*p==' ') {
			p++;
		}
		i++;
	}
	rawlen = i;
	return 0;
}

/*************************************************************************

TBCD-STRING::= OCTET STRING
	-- This type (Telephony Binary Coded Decimal String) is used to
	-- represent several digits from 0 through 9, *, #, a, b, c, two
	-- digits per octet, each digit encoded 0000 to 1001 (0 to 9),
	-- 1010 (*), 1011 (#), 1100 (a), 1101 (b) or 1110 (c); 1111 used
	-- as filler when there is an odd number of digits.

	-- bits 8765 of octet n encoding digit 2n
	-- bits 4321 of octet n encoding digit 2(n-1) +1

*************************************************************************/

int ber_decode_TBCD(const Uchar* raw, Uint rawlen, string2& value)
{
	for (register int i = 0; i<rawlen; i++) {
		char lc = bcd_lo(raw[i], TBCD_DICT);
		if (lc==0) {
			break;
		}
		value += lc;

		char hc = bcd_hi(raw[i], TBCD_DICT);
		if (hc==0) {
			break;
		}
		value += hc;
	}
	return 0;
}

int ber_encode_TBCD(const string2& value, Uchar* raw, Uint& rawlen)
{
	const char* p = value.c_str();
	int i = 0;
	while (*p) {
		static const char* dict = TBCD_DICT;
		if (i>=rawlen) {
			return -1;
		}
		raw[i] = 0;
		Uchar l = tbcd_quart(dict, *p);
		if (l==0x0f) {
			break;
		}
		raw[i] = (raw[i] | l);
		p++;

		if (*p==0) {
			raw[i] = (raw[i] | 0xf0);
		} else {
			Uchar h = tbcd_quart(dict, *p);
			if (h==0x0f) {
				break;
			}
			raw[i] = (raw[i] | (h<<4));
		}
		p++;
		i++;
	}
	rawlen = i;
	return 0;
}

/*************************************************************************
BCDDirectoryNumber		::= OCTET STRING
	-- This type contains the binary coded decimal representation of
	-- a directory number e.g. calling/called/connected/translated number.
	-- The encoding of the octet string is in accordance with the
	-- the elements "Calling party BCD number", "Called party BCD number"
	-- and "Connected number" defined in TS 24.008.
	-- This encoding includes type of number and number plan information
	-- together with a BCD encoded digit string.
	-- It may also contain both a presentation and screening indicator
	-- (octet 3a).
	-- For the avoidance of doubt, this field does not include
	-- octets 1 and 2, the element name and length, as this would be
	-- redundant.
*************************************************************************/
/*
*	Notes:
*		String data format:
*			ii|nnnn...
*		where 'ii' is the hex presentation and screening indicator
*		and the 'nnnn' part may be empty.
*/
int ber_decode_BCDDirNo(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen<1) {
		return -1;
	}

	// Directroy Number indicators
	char temp[4];
	static const char* dict = BCD_DICT;
	temp[0] = bcd_hi(raw[0]);
	temp[1] = bcd_lo(raw[0]);
	temp[2] = 0;
	value += temp;
	value += '|';

	return ber_decode_TBCD(raw+1, rawlen-1, value);
}

int ber_encode_BCDDirNo(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<1 || value.size()<3) {
		return -1;
	}

	char temp[3];
	strncpy(temp, value.c_str(), sizeof(temp)-1);
	raw[0] = atoi(temp);

	Uint len = (rawlen-1);
	if (ber_encode_TBCD(value.c_str()+3, raw+1, len)!=0) {
		return -1;
	}
	rawlen = len+1;
	return 0;
}

/*
*	Notes:
*		Same as BCDDirectoryNumber, but it's contained octet 1 and 2.
*		String data format is:
*			iiii|nnnn...
*/

int ber_decode_BCDTelNo(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen<2) {
		return -1;
	}
	char temp[4];
	temp[2] = 0;
	static const char* dict = BCD_DICT;
	temp[0] = bcd_hi(raw[0]);
	temp[1] = bcd_lo(raw[0]);
	value += temp;
	temp[0] = bcd_hi(raw[1]);
	temp[1] = bcd_lo(raw[1]);
	value += temp;
	value += '|';
	return ber_decode_BCDDirNo(raw+2, rawlen-2, value);
}

int ber_encode_BCDTelNo(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<2 || value.size()<5) {
		return -1;
	}
	char temp[3];
	temp[2] = 0;
	memcpy(temp, value.c_str(), sizeof(temp)-1);
	raw[0] = atoi(temp);
	memcpy(temp, value.c_str()+2, sizeof(temp)-1);
	raw[1] = atoi(temp);
	Uint len = rawlen-2;
	if (ber_encode_BCDDirNo(value+5, raw+2, len)!=0) {
		return -1;
	}
	rawlen = len+2;
	return 0;
}

/*************************************************************************
AddressString::= OCTET STRING (SIZE (1..maxAddressLength))
	-- This type is used to represent a number for addressing
	-- purposes. It is composed of
	--	a)	one octet for nature of address, and numbering plan
	--		indicator.
	--	b)	digits of an address encoded as TBCD-String.

	-- a)	The first octet includes a one bit extension indicator, a
	--		3 bits nature of address indicator and a 4 bits numbering
	--		plan indicator, encoded as follows:

	-- bit 8: 1  (no extension)

	-- bits 765: nature of address indicator
	--	000  unknown
	--	001  international number
	--	010  national significant number
	--	011  network specific number
	--	100  subscriber number
	--	101  reserved
	--	110  abbreviated number
	--	111  reserved for extension

	-- bits 4321: numbering plan indicator
	--	0000  unknown
	--	0001  ISDN/Telephony Numbering Plan (Rec CCITT E.164)
	--	0010  spare
	--	0011  data numbering plan (CCITT Rec X.121)
	--	0100  telex numbering plan (CCITT Rec F.69)
	--	0101  spare
	--	0110  land mobile numbering plan (CCITT Rec E.212)
	--	0111  spare
	--	1000  national numbering plan
	--	1001  private numbering plan
	--	1111  reserved for extension

	--	all other values are reserved.

	-- b)	The following octets representing digits of an address
	--		encoded as a TBCD-STRING.
*************************************************************************/

int ber_decode_AddressString(const Uchar* raw, Uint rawlen, string2& value)
{
	return ber_decode_BCDDirNo(raw, rawlen, value);
}

int ber_encode_AddressString(const string2& value, Uchar* raw, Uint& rawlen)
{
	return ber_encode_BCDDirNo(value, raw, rawlen);
}


/*************************************************************************
	-- This type is used to represent the code identifying a single
	-- teleservice, a group of teleservices, or all teleservices. The
	-- services are defined in TS GSM 02.03.
	-- The internal structure is defined as follows:

	-- bits 87654321: group (bits 8765) and specific service
	-- (bits 4321)
*************************************************************************/
int ber_decode_TeleserviceCode(const Uchar* raw, Uint rawlen, string2& value)
{
	if (rawlen<1) {
		return -1;
	}
	char cTemp[32];
	sprintf(cTemp, "%lu", (unsigned long)((unsigned char)(raw[0]&0xf0)>>4));
	value = cTemp;
	value += '|';
	sprintf(cTemp, "%lu", (unsigned long)((unsigned char)(raw[0]&0x0f)));
	value += cTemp;
	return 0;
}

int ber_encode_TeleserviceCode(const string2& value, Uchar* raw, Uint& rawlen)
{
	if (rawlen<1) {
		return -1;
	}
	unsigned char hi = atoi(value.c_str());
	unsigned char low = 0;
	const char* p = strchr(value.c_str(), '|');
	if (p) {
		p++;
		low = atoi(p);
	}
	raw[0] = ((hi<<4)|low);
	return 0;
}




//------------------------------------------------------------------
//
// Implement of CASN1CoderList
//
//------------------------------------------------------------------


CASN1CoderList::CASN1CoderList()
{
}

CASN1CoderList::~CASN1CoderList()
{
	Clear();
}

void CASN1CoderList::Clear()
{
	for (register size_t i = 0; i<m_list.size(); i++) {
		if (m_list[i]) {
			delete m_list[i];
			m_list[i] = NULL;
		}
	}
	m_list.clear();
}

int CASN1CoderList::AddCoder(const char* fdesc, const char* fspec, const char* rootname)
{
	PASN1Coder pASN1Coder = new CASN1Coder();
	if (!pASN1Coder) {
		return -1;
	}
	if (pASN1Coder->Init(fdesc, fspec, rootname)!=0) {
		delete pASN1Coder;
		return -1;
	}
	m_list.push_back(pASN1Coder);
	return m_list.size()-1;
}

void CASN1CoderList::DelCoder(size_t handle)
{
	delete m_list[handle];
	m_list[handle] = NULL;
}

int CASN1CoderList::ShowTree(size_t handle) const
{
	GetCoder(handle)->PrintTree();
	return 0;
}

int CASN1CoderList::ShowName(size_t handle) const
{
	GetCoder(handle)->Print();
	return 0;
}

int CASN1CoderList::ShowHash(size_t handle) const
{
	GetCoder(handle)->PrintHash();
	return 0;
}

int CASN1CoderList::GetShortName(const char* longname, string2& shortname, size_t handle) const
{
	return GetCoder(handle)->get_shortname(longname, shortname);
}

int CASN1CoderList::GetLongName(const char* shortname, string2& longname, size_t handle) const
{
	return GetCoder(handle)->get_longname(shortname, longname);
}

// add by shirly
int CASN1CoderList::Show(std::vector<ShowNode>& sublist,const char* name,size_t handle) 
{
		GetCoder(handle)->NewPrintTree(sublist,name);
		return 0;
}




//------------------------------------------------------------------
//
// Implement of CASN1Coder
//
//------------------------------------------------------------------

CASN1Coder::CASN1Coder():
	m_namehandles(HASH_MAX),
	m_fasthandles(HASH_MAX),
	m_asn1_coders(HASH_MAX),
	m_spec_coders(HASH_MAX)
{
	// Nothing to do here
}

CASN1Coder::~CASN1Coder()
{
	Free();
}

int CASN1Coder::Init(const char* fdesc, const char* fspec, const char* rootname)
{
	char log_buff[128] = {0};
	Free();

	//sprintf(log_buff,"%s - parsing ASN.1 desc file '%s'...",MODULE_NAME, fdesc);
	//g_log.addLog(log_buff);
	//printf("%s - parsing ASN.1 desc file '%s'...\n", MODULE_NAME, fdesc);
	if (!m_desc.Parse(fdesc, rootname)) {
		return -1;
	}
	if (m_desc.GetTree()==NULL) {
		return -1;
	}


	if (load_StandardBERCoder()!=0) {
		return -1;
	}

	if (load_SpecificBERCoder(fspec)!=0) {
		return -1;
	}

	//printf("%s - link BER coders...\n", MODULE_NAME);
	//sprintf(log_buff,"%s - link BER coders...",MODULE_NAME);
	//g_log.addLog(log_buff);
	link_BERCoder(m_desc.GetTree());

	//printf("%s - init successfully.\n", MODULE_NAME);
	//sprintf(log_buff,"%s - init successfully!!!",MODULE_NAME);
	//g_log.addLog(log_buff);
	return 0;
}

void CASN1Coder::Free()
{
	m_seperators.clear();
	m_namehandles.clear();
	m_fasthandles.clear();
	m_asn1_coders.clear();
	m_spec_coders.clear();
}

void CASN1Coder::FirstToUpper(string2& trans_value)
{
	*(trans_value.data()) -= 32;
}

const ASNNameHandle* CASN1Coder::get_namehandle(const char* name)
{

	CNameHandles::const_iter it = m_fasthandles.find(name);
	if (NULL==it) {
		if (NULL==(it = m_namehandles.find(name))) {
			it = m_fasthandles.set(name, ASNNameHandle(name, NULL));
		} else {
			it = m_fasthandles.set(name, it->value);
		}
	}

	return &it->value;
}

int CASN1Coder::decode(const ASNNameHandle* hName, const Uchar* raw, Uint rawlen, string2& value)
{
	if (hName) {
		if (hName->pbercoder) {
			int ret = hName->pbercoder->decode(raw, rawlen, value);
			const ASN1_NODE* pNode = asn1_node_GetSubTree(m_desc.GetTree(), (size_t *)hName->asnname.oids(), hName->asnname.size());
			if (!pNode)
			{
				return ret;
			}
			while (pNode && pNode->type==ASN1_TYPE_USER_DEFINED) {
				pNode = pNode->children;
			}
			
			//get_ActualType(pNode)
			if ((ASN1_TYPE_ENUMERATED == pNode->type||ASN1_TYPE_INTEGER==pNode->type) && pNode->children)
			{
				const ASN1_NODE* eval = pNode->children;
				while (eval)
				{
					if ( !strcmp(eval->const_value, value.c_str()))
					{
						
						value = eval->name;
						FirstToUpper(value);
						break;
					}
					eval = eval->brothers;
				}
			}
			return ret;
		}
	}
	return ber_decode_default(raw, rawlen, value);
}

int CASN1Coder::encode(const ASNNameHandle* hName, const string2& value, Uchar* raw, Uint& rawlen)
{
	if (hName) {
		if (hName->pbercoder) {
			return hName->pbercoder->encode(value, raw, rawlen);
		}
	}
	return ber_encode_default(value, raw, rawlen);
}

int CASN1Coder::get_shortname(const char* longname, string2& shortname) const
{
	shortname = OID_PREFIX;
	return find_ShortName(m_desc.GetTree(), longname, shortname);
}
/*	const ASNNameHandle* recordTypeNH = ber_getnamehandle(shortname, 0);	
		ASNName asnname = recordTypeNH->asnname;	
		char suboid[128] = "";
		strcpy(suboid,asnname.str());
		char *temp = strtok(suboid,".");
		int i = 0, num = 0;
		while(temp)
		{
			if(i != 0)
			{
				i--;
				asnname.oids[i] = atoi(temp);
				num ++;
			}
			printf("temp = %s \n",temp);
			temp = strtok(NULL,".");
			i++;
		}
	
		printf("[ %s ] [ %d ] [ %d ] temp = %s	num = %d \n",__FILE__,__func__,__LINE__,temp,num);
*/

int CASN1Coder::get_longname(const char* shortname, string2& longname) const
{


	ASNName asnname;
	asnname.set(shortname);
//	printf("[%s] [%s] [%d] shortname = %s\n",__FILE__,__func__,__LINE__,shortname);


/*	for(int i = 0;i < asnname.size();i++)
	{
			printf("[%s] [%s] [%d] asnname.str() = %s, (asnname.oids())[%d] = %d\n",__FILE__,__func__,__LINE__,asnname.str(),i,(asnname.oids())[i]);
	}	
	printf("[%s] [%s] [%d] asnname.oids() = %d\n",__FILE__,__func__,__LINE__,asnname.oids());
*/
	return find_LongName(m_desc.GetTree(), (const size_t*)asnname.oids(), asnname.size(), longname);
}

// int CASN1Coder::get_seperator(const ASNCdr* pCdr, const ASNNameHandle*& hNameSep)
// {
// 	for (size_t i = 0; i<m_seperators.size(); i++) {
// 		if (NULL==(hNameSep = get_namehandle(m_seperators[i]))) {
// 			continue;
// 		}
// 		int count = (int)pCdr->count_seqof(hNameSep);
// 		if (count>=2) {
// 			return count;
// 		} else if (count>=1) {
// 			return count;
// 		}
// 	}
// 	hNameSep = NULL;
// 	return 0;
// }


void CASN1Coder::Print(FILE* fp/* = stdout*/) const
{
	for (CNameHandles::const_iter it = m_namehandles.begin(); it; it = it->next) {
		const BERCoder* pCoder = it->value.pbercoder;
		if (pCoder) {
			continue;
		}
		string2 lname;
		get_longname(it->name, lname);
		char codername[sizeof(pCoder->name)+2];
		sprintf(codername, "(%s)", pCoder->name);
		fprintf(fp, "%- 24s %- 24s ::= %s\n", it->name, codername, lname.c_str());
	}
}

const ASN1_NODE* search(const ASN1_NODE* pNode,const char * Sname)
{
	for (const ASN1_NODE* pChild = pNode->children; pChild; pChild = pChild->brothers) {
			if (strcmp(pChild->name,Sname) == 0) {
				//printf("1348%s%s\n",pChild->name,Sname);
					return pChild;
			} else if(pNode->tag == -1)
				{
					const ASN1_NODE* found = search(pChild,Sname);
					if(found)
						{
							return found;
						}
				}
		}
		return NULL;

}

void CASN1Coder::PrintTree(FILE* fp/* = stdout*/) const
{
	asn1_node_Print(m_desc.GetTree(), fp);
}

void CASN1Coder::NewPrintTree(std::vector<ShowNode>& sublist,const char* rootname)
{
	const ASN1_NODE* pchild = search(m_desc.GetTree(), rootname);
	char buff[1024] = {'\0'}; 
	char name[128] = {'\0'};
	sprintf(name,"%s",rootname);
	string rname = rootname;
	std::vector<ASNShowNode> asnlist;
	asnlist.clear();
	AsnNodePrint(sublist,rname,asnlist,name,buff,pchild->children->children);
		/*// debug
		 	std::vector<ShowNode>::iterator iter = sublist.begin();
		
		printf("--------------------------begin------------------\n");
		for(;iter!=sublist.end();iter++)
		{
			printf("%s=%s\n",iter->node_name,iter->node_value);
		}
		printf("--------------------------end------------------\n");*/
	std::vector<ASNShowNode>::iterator it1 = asnlist.begin();
	for(;it1!= asnlist.end();it1++)
	{	
		// copy the value
		for(std::vector<ShowNode>::iterator it2 = sublist.begin();it2 != sublist.end();it2++)
		{
			if(strcmp(it1->node_name,it2->node_name) == 0)
			{
				//printf("1411 ---%s\n",it2->node_name);
				string spc_name = rname+".mMTelInformation.listOfSupplServices[1]";
				vector<ASNShowNode>::iterator result; 
				result=find_if(asnlist.begin(),asnlist.end(),NodeNameFind(spc_name.c_str()));
				if(result != asnlist.end())
				{
					string name_tmp = rname+".mMTelInformation.listOfSupplServices[0]";
					if(0==strcmp(it2->node_name,name_tmp.c_str()))
					{
						char* p1 = strrchr(it2->node_name,'.');
						if(p1 != NULL)
							strcpy(it2->node_name,++p1);
						printf("%c%c%s\n",9,9,it2->node_name);
						it2++;
						char* p = strrchr(it2->node_name,'.');
						if(p != NULL)
							strcpy(it2->node_name,++p);
						printf("%c%c%c%s=%s\n",9,9,9,it2->node_name,it2->node_value);
						//printf("%c%c%c%s=%s\n",9,9,9,"serviceMode","empty_value");
						it2++;
						while(StringFind(it2->node_name, "listOfSupplServices"))
						{
							char* str = strrchr(it2->node_name,'.');
							if(str != NULL)
								strcpy(it2->node_name,++str);
							if(strlen(it2->node_value)>0)
								printf("%c%c%c%s=%s\n",9,9,9,it2->node_name,it2->node_value);
							else
								printf("%c%c%s\n",9,9,it2->node_name);
							it2++;
						}
					}
				}
				strcpy(it1->node_value,it2->node_value);
			}

		}

		//Adjust the print format
		int count = 0;
		for(int i = 0;i<strlen(it1->node_name);i++)
		{
			if(it1->node_name[i] == '.')
			{
				count++;
			}
		}
		char* ptr = strrchr(it1->node_name,'.');
		if(ptr != NULL)
		{
			strcpy(it1->node_name,++ptr);
			/* Special treatment for some AVPs  */
			if(!strcmp(it1->node_name,"online-charging-flag"))
			{
				if(!strcmp(it1->node_value,"empty_value"))
				{
					strcpy(it1->node_value,"offline");
				}
			}
			if(!strcmp(it1->node_name,"iMSEmergencyIndicator"))
			{
				if(!strcmp(it1->node_value,"empty_value"))
				{
					strcpy(it1->node_value,"NoEmergencyCall");
				}
			}
			if(!strcmp(it1->node_name,"aCRStartLost"))
			{
				if(!strcmp(it1->node_value,"255"))
				{
					strcpy(it1->node_value,"TRUE");
				}
				else
				{
					strcpy(it1->node_value,"FALSE");
				}
			}
			if(!strcmp(it1->node_name,"aCRStopLost"))
			{
				if(!strcmp(it1->node_value,"255"))
				{
					strcpy(it1->node_value,"TRUE");
				}
				else
				{
					strcpy(it1->node_value,"FALSE");
				}
			}
			if(!strcmp(it1->node_name,"call-property"))
			{
				int type_select = atoi(it1->node_value);
				if(type_select < 0 || type_select> 3)
				{
					//strcpy(it1->node_value,"NoSupport");
					sprintf(it1->node_value,"Value No Support(%d)",type_select);
				}
			}
			
		}

		// Adjust the print format
		if(strlen(it1->node_value) > 0)
		{
			for(int i = 0;i<count;i++)
			{
				printf("%c",9);
			}
			//if(0==strcmp(it1->node_name,"listOfSupplServices[0]"))
				//continue;
			printf("%s=%s\n",it1->node_name,it1->node_value);
		}
		else
		{
			if(0==strcmp(it1->node_name,"listOfSupplServices[1]"))
			{
				int n = 0;
				for(;n<11;n++)
				{
					it1=asnlist.erase(it1);
				}

			}
			char* ptr = strrchr(it1->node_name,'.');
			if(ptr != NULL)
			{
				strcpy(it1->node_name,++ptr);
				//continue;
			}
			vector<ASNShowNode>::iterator ret;
			ret=find_if(asnlist.begin(),asnlist.end(),NodeNameFind("aSRecord.mMTelInformation.listOfSupplServices[1]"));
			if(ret != asnlist.end() && 0==strcmp(it1->node_name,"listOfSupplServices[0]"))
			{
				continue;
			}
			for(int i = 0;i<count;i++)
			{
				printf("%c",9);
			}
			printf("%s\n",it1->node_name);
		}
	}
}


void CASN1Coder::PrintHash(FILE* fp/* = stdout*/) const
{
	fprintf(fp, "All name handles\n");
	m_namehandles.show_hash(fp);
	//rintf(fp, "Fast name handles\n");
	//fasthandles.show_hash(fp);
}


int CASN1Coder::load_StandardBERCoder()
{
	//printf("%s - loading standard ASN.1 coder...\n", MODULE_NAME);
	// Load standard ASN.1 coders
	for (size_t i = 0; i<dim(s_ASN1Coders); i++) {
		PBERCoder pBERCoder = GetBERCoder(s_ASN1Coders[i].coder_name);
		if (pBERCoder) {
			m_asn1_coders.set(s_ASN1Coders[i].asn1_name, pBERCoder);
		} else {
			//printf("Error: %s init failed - coder of name '%s' not found!!!\n",
			//	MODULE_NAME, s_ASN1Coders[i].coder_name);
			return -1;
		}
	}
	return 0;
}

int CASN1Coder::load_SpecificBERCoder(const char* fname)
{
	if (!fname) {
		return 0;
	}
	printf("%s - loading specific ASN.1 coder '%s'...\n", MODULE_NAME, fname);
	FILE* fp = fopen(fname, "r");
	if (!fp) {
		printf("%s - open file '%s' failed - %s!\n", MODULE_NAME, fname, strerror(errno));
		return -1;
	}
	int line = 0;
	char buff[1024];
	for (;;) {
		if (fgets(buff, sizeof(buff), fp)==NULL) {
			if (feof(fp)) {
				break;
			} else {
				printf("%s - read file '%s' failed (line: %d) - %s!", MODULE_NAME, fname, line, strerror(errno));
				fclose(fp);
				return -1;
			}
		}
		line++;
		const char* p = MyTrim(buff);
		if (*p=='#' || *p=='\0') {
			continue;
		}
		if (strncmp(ASN1_SPEC_SEPERATOR, p, sizeof(ASN1_SPEC_SEPERATOR)-1)==0) {
			if ((p = passby_token(ASN1_SPEC_SEPERATOR, p))==NULL) {
				printf("%s - read file '%s' failed (line: %d) - %s!", MODULE_NAME, fname, line, strerror(errno));
				return -1;
			}
			if (strncmp(p, OID_PREFIX, OID_PREFIX_SIZE)==0) {
				string2 longname;
				if (get_longname(p, longname)!=0) {
					printf("%s - read file '%s' failed (line: %d) - %s!",
						MODULE_NAME, fname, line, strerror(errno));
					return -1;
				}
				m_seperators.push_back(p);
				printf("%s - add %s {%s} of '%s'.\n", MODULE_NAME, ASN1_SPEC_SEPERATOR, p, longname.c_str());
			} else {
				string2 shortname;
				if (get_shortname(p, shortname)!=0) {
					printf("%s - read file '%s' failed (line: %d) - %s!",
						MODULE_NAME, fname, line, strerror(errno));
					return -1;
				}
				m_seperators.push_back(shortname);
				printf("%s - add %s {%s} of '%s'\n", MODULE_NAME, ASN1_SPEC_SEPERATOR, shortname.c_str(), p);
			}
		} else {
			char cName[1024];
			if ((p = get_next_token(cName, sizeof(cName), p, gASN1SpecToken))==NULL) {
				printf("%s - read file '%s' failed (line: %d) - ASN.1 name expected!!!\n", MODULE_NAME, fname, line);
				fclose(fp);
				return -1;
			}
			if ((p = passby_token("::=", p, gASN1SpecToken))==NULL) {
				printf("%s - read file '%s' failed (line: %d) - '::=' expected!!!\n", MODULE_NAME, fname, line);
				fclose(fp);
				return -1;
			}
			char cValue[256];
			if ((p = get_next_token(cValue, sizeof(cValue), p, gASN1SpecToken))==NULL) {
				printf("%s - read file '%s' failed (line: %d) - Coder name expected!!!\n", MODULE_NAME, fname, line);
				fclose(fp);
				return -1;
			}
			PBERCoder pBERCoder = GetBERCoder(cValue);
			if (pBERCoder) {
				if (strncmp(cName, OID_PREFIX, OID_PREFIX_SIZE)==0) {
					add_BERCoder(cName, pBERCoder);
				} else {
					m_spec_coders.set(cName, pBERCoder);
				}
			} else {
				printf("%s - read file '%s' failed (line: %d) - Coder of name '%s' not found!!!\n", MODULE_NAME, fname, line, cValue);
				fclose(fp);
				return -1;
			}
		}
	}
	fclose(fp);
	return 0;
}


bool CASN1Coder::link_BERCoder(const ASN1_NODE* pNode, string2 full_name/* = ""*/, string2 short_name/* = "OID"*/)
{
	if (full_name.size()>0) {
		full_name += ".";
	}

	full_name += pNode->name;

	if (pNode->tag!=ASN1_TAG_NONE) {
		char cOid[32];
		sprintf(cOid, ".%d", pNode->tag);	// Context tag
		short_name += cOid;
//		add_UTAG(pNode, short_name);		// Universal tag
	}
	if (!strcmp(pNode->name, "subscriptionIDData"))
	{
		int a = 1;
	}
	PBERCoder pBERCoder = find_NamedBERCoder(full_name, pNode->tname);
	if (pBERCoder) {
		add_BERCoder(short_name.c_str(), pBERCoder);
	} else {
		for (const ASN1_NODE* pChild = pNode->children; pChild; pChild = pChild->brothers) {
			link_BERCoder(pChild, full_name, short_name);
		}
	}
	return true;
}

int CASN1Coder::find_ShortName(const ASN1_NODE* pNode, const char* longname, string2& shortname) const
{
	if (*longname==0) {
		return 0;
	}

	if (!pNode) {
		printf("Error: %s init failed - nil parent for name '%s'!\n",
			MODULE_NAME, longname);
		return -1;
	}

	// 2004-11-25 Mike Liu: Add this section...
	if (strncmp(longname, SUM_PREFIX, SUM_PREFIX_SIZE)==0) {
		shortname += '.';
		shortname += SUM_PREFIX;
		longname += SUM_PREFIX_SIZE;
		if (*longname=='.') {
			longname++;
		}
	}

	if (add_UTAG(pNode, shortname, &longname)) {
		pNode = pNode->children;
	}

	size_t len = 0;
	const char* next = strchr(longname, '.');
	if (next) {
		len = next-longname;
	} else {
		len = strlen(longname);
	}

	const ASN1_NODE* pChild = get_ChildOf(pNode, longname, len);
	if (pChild) {
		char cTemp[32];
		sprintf(cTemp, ".%lu", pChild->tag);
		shortname += cTemp;
	} else {
		printf("Error: %s init failed - no child for name '%s'!\n",
			MODULE_NAME, longname);
		return -1;
	}

	if (next) {
		return find_ShortName(pChild, next+1, shortname);
	} else {
		return find_ShortName(pChild, longname+len, shortname);
	}
}

int CASN1Coder::find_LongName(const ASN1_NODE* pNode, const size_t* iNames, int iNamesLen, string2& longname) const
{
	
	if (iNamesLen<=0) {
		return 0;
	}
	
//	printf("[ %s ] [ %s ] [ %d ] [sizeof(Uint) = %d [ sizeof(int) = %d ] [sizeof(size_t) = %d ] [sizeof(long int) = %d ]iNames = %d *iNames = %d,iNamesLen = %d\n" \
	,__FILE__,__func__,__LINE__,sizeof(Uint),sizeof(int),sizeof(size_t),sizeof(long int),iNames,*iNames,iNamesLen);
	if (!pNode) {
		string2 sName;
		get_StringOid(iNames, iNamesLen, sName);
//		printf("[ %s ] [ %s ] [ %d ] iNames = %d\n",__FILE__,__func__,__LINE__,*iNames);
		printf("Error: %s init failed - nil parent for oids {%s}!\n",
			MODULE_NAME, sName.c_str());
		return -1;
	}
	const ASN1_NODE* pChild = get_ChildOf(pNode, iNames[0]);
	if (pChild) {
		if (longname.size()>0) {
//		printf("[ %s ] [ %s ] [ %d ] pChild.iNames = %d\n",__FILE__,__func__,__LINE__,*iNames);
		longname += '.';
		}
		longname += pChild->name;
	} else {
		string2 sName;
		get_StringOid(iNames, iNamesLen, sName);
	//	printf("Error: %s init failed - no child for oid {%s}!\n",MODULE_NAME, sName.c_str());
		return -1;
	}

	skip_UTAG(pChild, iNames, iNamesLen);
	
	return find_LongName(pChild, iNames + 1, iNamesLen-1, longname);
//	return find_LongName(pChild, (const size_t*)((char*)iNames+4), iNamesLen-1, longname);
}

CASN1Coder::PBERCoder CASN1Coder::find_NamedBERCoder(const string2& full_name, const string2& type_name)
{
	PBERCoder pBERCoder = NULL;

	// First find in specific named coders
	pBERCoder = find_SubNamedBERCoder(m_spec_coders, full_name);
	if (NULL==pBERCoder) {
		pBERCoder = m_spec_coders.get(type_name.c_str());
	}

	// If not found, use the standard named coders
	if (NULL==pBERCoder) {
		pBERCoder = find_SubNamedBERCoder(m_asn1_coders, full_name);
		if (NULL==pBERCoder) {
			pBERCoder = m_asn1_coders.get(type_name.c_str());
		}
	}
	return pBERCoder;
}

CASN1Coder::PBERCoder CASN1Coder::find_SubNamedBERCoder(CNameCoders& coders, const string2& full_name)
{
	PBERCoder pBERCoder = coders.get(full_name.c_str());
	if (pBERCoder) {
		return pBERCoder;
	} else {
		const char* p = strchr(full_name.c_str(), '.');
		if (!p) {
			return NULL;
		} else {
			p++;
			string2 sub_name = p;
			return find_SubNamedBERCoder(coders, sub_name);
		}
	}
}

const ASN1_NODE* CASN1Coder::get_ChildOf(const ASN1_NODE* pParent, const char* name, size_t len) const
{
	for (register const ASN1_NODE* pChild = pParent->children; pChild; pChild = pChild->brothers) {
		if (pChild->tag==ASN1_TAG_NONE) {
			const ASN1_NODE* pFound = get_ChildOf(pChild, name, len);
			if (pFound) {
				return pFound;
			}
		} else if (strncmp(pChild->name, name, len)==0) {
			return pChild;
		} else if ('0'<=*name && *name<='9' && atoi(name)==pChild->tag) {	// 2004-11-25 Mike Liu: Added.
			return pChild;
		}
	}
	return NULL;
}

const ASN1_NODE* CASN1Coder::get_ChildOf(const ASN1_NODE* pParent, size_t tag) const
{
	for (register const ASN1_NODE* pChild = pParent->children; pChild; pChild = pChild->brothers) {
		if (pChild->tag==tag) {
				return pChild;
		} else if (pChild->tag==ASN1_TAG_NONE) {
			const ASN1_NODE* pFound = get_ChildOf(pChild, tag);
			if (pFound) {
				return pFound;
			}
		}
	}
	return NULL;
}

void CASN1Coder::get_StringOid(const size_t* iNames, int iNamesLen, string2& sName) const
{

	for (register int i = 0; i<iNamesLen; i++) {
//		printf("[ %s ] [ %s ] [ %d ] iNames = %d iNames[%d] = %d\n",__FILE__,__func__,__LINE__,iNames,i,iNames[i]);
		if (i!=0) {
			sName += '.';
		}
		char cTemp[32];
		sprintf(cTemp, "%lu", iNames[i]);
		sName += cTemp;
	}
}

int CASN1Coder::add_BERCoder(const char* name, const char* type)
{
	PBERCoder item = GetBERCoder(type);
	if (NULL==item) {
		return -1;
	}
	return add_BERCoder(name, item);
}

int CASN1Coder::add_BERCoder(const char* name, PBERCoder pBERCoder)
{
	ASNNameHandle nameattr(name, pBERCoder);
	m_namehandles.set(name, nameattr);
	return 0;
}

const ASN1_NODE* CASN1Coder::get_ActualType(const ASN1_NODE* pNode) const
{
	while (pNode && pNode->type==ASN1_TYPE_USER_DEFINED) {
		pNode = pNode->children;
	}
	return pNode;
}

bool CASN1Coder::add_UTAG(const ASN1_NODE* pNode, string2& shortname, const char** pnext) const
{
	char cTemp[32];
	if (NULL==(pNode = get_ActualType(pNode))) {
		return false;
	}
	switch (pNode->type) {
	case ASN1_TYPE_SEQUENCE_OF:
		if (pNode->children) {
			//if (pNode->children->type==ASN1_TYPE_SEQUENCE || pNode->children->type==ASN1_TYPE_SET) {
				sprintf(cTemp, ".%d", ASN1_UTAG_SEQUENCE_SEQUENCE_OF);
				shortname += cTemp;
				if (pnext && *pnext) {
					if (atoi(*pnext)==ASN1_UTAG_SEQUENCE_SEQUENCE_OF) {
						if (NULL!=(*pnext = strchr(*pnext, '.'))) {
							(*pnext)++;
						}
					}
				}
				return true;
			//}
		}
		break;

	case ASN1_TYPE_SET_OF:
		if (pNode->children) {
			//if (pNode->children->type==ASN1_TYPE_SET || pNode->children->type==ASN1_TYPE_SEQUENCE) {
				sprintf(cTemp, ".%d", ASN1_UTAG_SET_SET_OF);
				shortname += cTemp;
				if (pnext && *pnext) {
					if (atoi(*pnext)==ASN1_UTAG_SET_SET_OF) {
						if (NULL!=(*pnext = strchr(*pnext, '.'))) {
							(*pnext)++;
						}
					}
				}
				return true;
			//}
		}
		break;
	}
	return false;
}

bool CASN1Coder::skip_UTAG(const ASN1_NODE* pNode, const size_t*& iNames, int& iNamesLen) const
{
	if (NULL==(pNode = get_ActualType(pNode))) {
		return false;
	}
	switch (pNode->type) {
	case ASN1_TYPE_SEQUENCE_OF:
		if (pNode->children) {
			//if ((pNode->children->type==ASN1_TYPE_SEQUENCE || pNode->children->type==ASN1_TYPE_SET) && iNamesLen>1) {
			if (iNamesLen>1) {
				if (iNames[1]==ASN1_UTAG_SEQUENCE_SEQUENCE_OF) {
					iNames++;
					iNamesLen--;
					return true;
				}
			}
		}
		break;

	case ASN1_TYPE_SET_OF:
		if (pNode->children) {
			//if ((pNode->children->type==ASN1_TYPE_SET || pNode->children->type==ASN1_TYPE_SEQUENCE)  && iNamesLen>1) {
			if (iNamesLen>1) {
				if (iNames[1]==ASN1_UTAG_SET_SET_OF) {
					iNames++;
					iNamesLen--;
					return true;
				}
			}
		}
		break;
	}
	return false;
}











//------------------------------------------------------------------
//
// Implemantation of class Name value pair list
//
//------------------------------------------------------------------

ASNName::ASNName()
{

	// Nothing to do here.
}

ASNName::ASNName(const char* name)
{
	set(name);
}

ASNName::ASNName(const Uint* oids, Uint oidscount)
{
	set(oids, oidscount);
}

ASNName::ASNName(const ASNName& other)
{
	*this = other;
}

ASNName& ASNName::operator =(const ASNName& other)
{
	if (this!=&other) {
		m_name = other.m_name;
		m_oids = other.m_oids;
	}
	return *this;
}

ASNName::~ASNName()
{

	// Nothing to do here
}

bool ASNName::set(const char* name)
{
	if (!set_int(m_oids, name)) {
		return false;
	}
	return set_str(m_name, m_oids.get(), m_oids.size());
}

bool ASNName::set(const Uint* oids, Uint oidscount)
{
	m_oids.set(oids, oidscount);
	return set_str(m_name, oids, oidscount);
}

bool ASNName::append(const Uint* oids, Uint oidscount)
{

	for (register int i = 0; i<oidscount; i++) {
		m_oids.append(oids[i]);
	}
	return set_str(m_name, m_oids.get(), m_oids.size());
}

bool ASNName::append(const ASNName& other)
{
	return append(other.oids(), other.size());
}


bool ASNName::set_str(string2& name, const Uint* iname, Uint isize) const
{
	name = OID_PREFIX;
	char temp[32];
	temp[0] = '.';
	for (register size_t i = 0; i<isize; i++) {
		myutostr(temp+1, iname[i]);
		name += temp;
	}
//	printf("[ %s ] [ %s ] [ %d ] name.c_str() = %s\n",__FILE__,__func__,__LINE__,name.c_str());
	return true;
}

bool ASNName::set_int(UIBuff& iname, const char* name) const
{
	iname.clear();
	long int num;
	if (strncmp(name, OID_PREFIX, OID_PREFIX_SIZE)==0) {
		char* tag = (char*)name+OID_PREFIX_SIZE+1;	// Skip '.', so add  1
		for (;;) {
			if (strncmp(tag, SUM_PREFIX, SUM_PREFIX_SIZE)==0) {
				break;
			}
			iname.append((num = strtol(tag, &tag, 10)));
//				printf("[%s] [%s] [%d] long int num = %ld\n",__FILE__,__func__,__LINE__,num);
			if (*tag=='.') {		// Seperator
				tag++;
			} else if (*tag==0) {	// End of name
				break;
			} else {
				iname.clear();
				return false;
			}
		}
	} else if (strncmp(name, SUM_PREFIX, SUM_PREFIX_SIZE)==0) {
		char* tag = (char*)name+SUM_PREFIX_SIZE+1;	// Skip '.', so add  1
		for (;;) {
			iname.append(strtol(tag, &tag, 10));
			if (*tag=='.') {		// Seperator
				tag++;
			} else if (*tag==0) {	// End of name
				break;
			} else {
				iname.clear();
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}








ASNNameHandle::ASNNameHandle():
	pbercoder(NULL)
{
	// Nothing to do here.
}

ASNNameHandle::ASNNameHandle(const ASNNameHandle& other):
	asnname(other.asnname),
	pbercoder(other.pbercoder)
{
}

ASNNameHandle::ASNNameHandle(const ASNName& asnn, const BERCoder* pberc):
	asnname(asnn),
	pbercoder(pberc)
{
}

ASNNameHandle::~ASNNameHandle()
{
	// Nothing to do here
}

