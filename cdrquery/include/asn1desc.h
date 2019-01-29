// asn1desc.h

#ifndef __asn1desc_h__
#define __asn1desc_h__

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>


using namespace std;

#define ASN1_MAX_NAME			256
#define ASN1_TAG_NONE			size_t(-1)

enum ASN1_MODULE_DEFAULT_TAG {
	ASN1_MODULE_EXPLICIT_TAGS = 0,
	ASN1_MODULE_IMPLICIT_TAGS,
	ASN1_MODULE_AUTOMATIC_TAGS
};

enum ASN1_TYPE {
	ASN1_TYPE_USER_DEFINED = 0,			// Notes: Do NOT delete this item!!!
	// Reference type
	ASN1_TYPE_ANY_DEFINED_BY,
	// Basic type
	ASN1_TYPE_OCTET_STRING,
	ASN1_TYPE_BIT_STRING,
	ASN1_TYPE_INTEGER,
	ASN1_TYPE_BOOLEAN,
	ASN1_TYPE_OBJECT_IDENTIFIER,
	ASN1_TYPE_IA5String,
	ASN1_TYPE_GraphicString,
	ASN1_TYPE_UTF8String,
	ASN1_TYPE_ENUMERATED,
	// Construct type
	ASN1_TYPE_CHOICE,
	ASN1_TYPE_SEQUENCE,
	ASN1_TYPE_SEQUENCE_OF,
	ASN1_TYPE_SET,
	ASN1_TYPE_SET_OF,
	ASN1_TYPE_CLASS
};

enum ASN1_CONSTRAIN_TYPE {
	ASN1_CONSTRAIN_TYPE_NONE = 0,
	ASN1_CONSTRAIN_TYPE_SIZE1,
	ASN1_CONSTRAIN_TYPE_SIZE2,
	ASN1_CONSTRAIN_TYPE_RANGE
};


struct ASN1_CONSTRAIN_SIZE1 {
	char	size_name[ASN1_MAX_NAME];
	size_t	size;
};

struct ASN1_CONSTRAIN_SIZE2 {
	char	low_name[ASN1_MAX_NAME];
	char	high_name[ASN1_MAX_NAME];
	size_t	low;
	size_t	high;
};

struct ASN1_CONSTRAIN_RANGE {
	char	low_name[ASN1_MAX_NAME];
	char	high_name[ASN1_MAX_NAME];
	size_t	low;
	size_t	high;
};

union ASN1_CONSTRAIN_UNION {
	ASN1_CONSTRAIN_SIZE1		size1;
	ASN1_CONSTRAIN_SIZE2	size2;
	ASN1_CONSTRAIN_RANGE	range;
};

struct ASN1_CONSTRAIN {
	ASN1_CONSTRAIN_TYPE		constrain_type;
	ASN1_CONSTRAIN_UNION	constrain;
};

struct ASN1_NODE {
	char					name[ASN1_MAX_NAME];
	size_t					tag;
	ASN1_TYPE				type;
	char					tname[ASN1_MAX_NAME];	// Type name
	bool					optional_flag;
	bool					explicit_flag;
	ASN1_CONSTRAIN			constrain;
	char*					const_value;
	ASN1_NODE*				brothers;
	ASN1_NODE*				children;
	
};

struct ShowNode
{
	char		node_name[ASN1_MAX_NAME];
	char		node_value[ASN1_MAX_NAME];
	ShowNode()
	{
		memset(node_name,'\0',sizeof(node_name));
		memset(node_value,'\0',sizeof(node_value));
	}

};




struct ASNShowNode{
	char		node_name[ASN1_MAX_NAME];
	char		node_value[ASN1_MAX_NAME];
	ASNShowNode()
	{
		memset(node_name,'\0',sizeof(node_name));
		memset(node_value,'\0',sizeof(node_value));
	}
};

class NodeNameFind{
	public:
	NodeNameFind(const char* val) : str(val){}
	bool operator()(ShowNode& obj) { return !strcmp(str,obj.node_name);}
	bool operator()(ASNShowNode& obj) { return !strcmp(str,obj.node_name);}
	private:
		const char* str;
};



ASN1_NODE* asn1_node_Alloc();
ASN1_NODE* asn1_node_Duplicate(const ASN1_NODE* pNode);
void asn1_node_Free(ASN1_NODE* &pNode);
void asn1_node_AddChild(ASN1_NODE* pParent, ASN1_NODE* pChild);
void asn1_node_AddBrother(ASN1_NODE* pNode1, ASN1_NODE* pNode2);
void asn1_node_SetConstValue(ASN1_NODE* pNode, const char* cValue);
/*modify by shirly:
	ASNnode: save all of AVPs we need
	name : Save the prefix for each entry
	buff : Save temporary variables of children
	pNode : all of the AVPs
*/
void asn1_node_Print(const ASN1_NODE* pNode, FILE* fp = stdout, int level = 0);
char* asn1_node_DupStr(const char* cStr);
bool asn1_node_IsBasicType(const ASN1_NODE* pNode);

const ASN1_NODE* asn1_node_GetSubTree(const ASN1_NODE* pNode, size_t *oid, int oidlen);

// add by shirly:Find a specific AVPs by name
const ASN1_NODE* search(const ASN1_NODE* pNode,char * name);
void AsnNodePrint(std::vector<ShowNode>& subnode,string& namesave,std::vector<ASNShowNode>& node,char* name,char* buff,const ASN1_NODE* pNode);



class CASN1Token
{
protected:
	char		m_fname[256];
	FILE*		m_fp;
	int			m_line;
	int			m_column;
	char		m_buff[1024];
	const char*	m_ptr;

public:
	CASN1Token();
	CASN1Token(const char* fname);
	~CASN1Token();
	int Init(const char* fname);
	void Free();
	int GetNToken(char* buff, size_t size);
	bool IsToken(const char* buff, const char* token) const;

	const char* GetFileName() const { return m_fname; }
	int GetLine() const { return m_line; }
	int GetColumn() const { return m_column; }
};



class CASN1Desc
{
	typedef ASN1_NODE*				PASN1_NODE;
	typedef std::vector<PASN1_NODE>	CNodeList;

	char		m_fname[256];
	CASN1Token	m_toker;
	bool		m_unget_flag;
	char		m_unget[ASN1_MAX_NAME];

	CNodeList	m_node_list;
	ASN1_NODE*	m_pRoot;

	char						m_module_name[256];
	char						m_module_objname[256];
	int							m_module_objid;
	ASN1_MODULE_DEFAULT_TAG		m_module_default_tags;
protected:
	void SetFileName(const char* fname);	// Set parse file name
	const char* GetFileName() const;			// Get parse file name

	int Init(const char* fname);	// Init toker object
	void Free();					// Free toker object

	// Token operators
	bool Get_Token(char* cToken, size_t uSize);
	bool Unget_Token(const char* cToken);
	bool Is_Token(const char* cBuff, const char* cToken) const;
	bool Is_Number(const char* cBuff) const;
	bool Passby_Token(const char* token);


	// Node list operators
	void node_list_Print(FILE* fp = stdout) const;
	const ASN1_NODE* node_list_Get(const char* cNodeName) const;
	const ASN1_NODE* node_list_GetFirst() const;
	void node_list_Clear();

	// Like decription tree
	ASN1_NODE* create_Tree(const char* cRootName = NULL) const;
	void delete_Tree(ASN1_NODE*& pRoot) const;
	bool link_Tree(ASN1_NODE* pNode, const ASN1_NODE* pParent = NULL) const;
	bool link_Constrain(ASN1_NODE* pNode) const;
	bool link_ConstantInteger(const char* cName, size_t& value) const;


protected:
	// Parse operators
	bool parse_Define();
	bool parse_TypeDefine(ASN1_NODE* pNode);
	bool parse_ConstDefine(ASN1_NODE* pNode);
	bool parse_ConstValue(ASN1_NODE* pNode);
	bool parse_ChoiceBody(ASN1_NODE* pNode);
	bool parse_SetBody(ASN1_NODE* pNode);
	bool parse_SequenceBody(ASN1_NODE* pNode);
	bool parse_ClassBody(ASN1_NODE* pNode);
	bool parse_EnumeratedBody(ASN1_NODE* pNode);
	bool parse_BodyDefine(ASN1_NODE* pNode);
	bool parse_RangeDesc(ASN1_NODE* pNode);
	bool parse_SizeDesc(ASN1_NODE* pNode);

	void on_Exception(const std::string& msg) const;

public:
	CASN1Desc();
	~CASN1Desc();
	bool Parse(const char* fname, const char* cRootName = NULL);
	const ASN1_NODE* GetTree() const { return m_pRoot; }
};

#endif


