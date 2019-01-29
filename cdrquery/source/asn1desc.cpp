/******************************************************************************

asn1desc.cpp


<Define> =>		token <Const-Define> |
			<Type-Name> ::= <Type-Define>

<Type-Define> => 	CHOICE <Choice-Body> |
			SET <Set-Body> |
			SET OF <Type-Define> |
			EXPLICIT <Type-Define> |
			IMPLICIT <Type-Define> |
			SEQUENCE <Sequence-Body> |
			SEQUENCE OF <Type-Define> |
			SEQUENCE <Size-Desc> OF <Type-Name> |
			SEQUENCE <Size-Desc> OF <Sequence-Body> |
			ENUMERATED <Enumerated-Body> |
			CLASS <Class-Body> |
			OCTET STRING [(<Size-Desc>)] |
			BIT STRING [<Enumerated-Body>] |
			INTEGER [(<Range-Desc>) | <Enumerated-Body>] |
			BOOLEAN |
			OBJECT IDENTIFIER |
			<Type-Name> |
			<Class-Name>.&<Type-Name> ({<Type-Name>} [{@<Type-Name>}])

<Const-Define> =>	<Type-Name> ::= <Const-Value>

<Type-Name> = >		OBJECT IDENTIFIER |
			INTEGER |
			OCTET STRING |
			token

<Type-Desc> =>		<Range-Desc> |
			<Size-Desc>
<Range-Desc> =>		low..high
<Size-Desc> =>		SIZE(int<Size-Desc-Sufix>

<Size-Desc-Sufix> =>	) | ..int)

<Const-Value> =>	'{' { oid-name (oid) } '}' |
			integer



<Choice-Body} =>	<Body-Define>
<Set-Body> =>		<Body-Define>
<Sequence-Body> =>	<Body-Define>

<Class-Body> =>		'{' &field-name [type-name [OPTIONAL]] {,&field-name [type-name [OPTIONAL]]}  '}'
<Enumerated-Body> =>	'{' token (id) {,token (id)}  [, ...]  '}'
<Body-Define> =>	'{' <Body-Item-Define> { , <Body-Item-Define> } [, ...] '}' |
			'{'...'}'
<Body-Item-Define> =>	token '['integer']' <Type-Define> [OPTIONAL] |
			token <Type-Define> [OPTIONAL]



MAP-EXTENSION  ::= CLASS {
	&ExtensionType	OPTIONAL,
	&extensionId 	OBJECT IDENTIFIER }

PrivateExtension ::= SEQUENCE {
	extId		MAP-EXTENSION.&extensionId ({ExtensionSet}),
	extType		MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId})	OPTIONAL
}


History:
	2004-11-19	Mike Liu	Fixed a potential bug in
					asn1_free_Node.

******************************************************************************/
//#include "stdafx.h"

#include "tokpsr.h"
#include "tools.h"
#include <errno.h>
#include "asn1desc.h"

const char* ASN1_TOKENS[] =
{
	"::=",
	"--",
	"[",
	"]",
	"{",
	"}",
	"(",
	")",
	".",
	",",
	"&",
	"@",
	";",
	NULL
};



//-----------------------------------------------------------------------------
//
//	Implementation for CASN1Desc
//
//-----------------------------------------------------------------------------

CASN1Desc::CASN1Desc()
{
	m_fname[0] = 0;
	m_unget_flag = false;
	m_unget[0] = 0;
	m_pRoot = NULL;

	m_module_name[0] = 0;
	m_module_objname[0] = 0;
	m_module_objid = 0;
	m_module_default_tags = ASN1_MODULE_EXPLICIT_TAGS;
}

CASN1Desc::~CASN1Desc()
{
	Free();
}

int CASN1Desc::Init(const char* fname)
{
	
	Free();
	return m_toker.Init(fname);
}

void CASN1Desc::Free()
{
	m_unget_flag = false;
	m_unget[0] = 0;
	m_toker.Free();
	node_list_Clear();
	delete_Tree(m_pRoot);
}


bool CASN1Desc::Parse(const char* fname, const char* cRootName/* = NULL*/)
{

	if (!fname) {
		printf("parse failed - file name is null!!!\n");
		return false;
	}
	SetFileName(fname);
	if (*GetFileName()==0) {
		printf("parse failed - no file name!!!\n");
		return false;
	}

	if (Init(GetFileName())!=0) {
		printf("parse failed - open file failed '%s'!!!\n", GetFileName());
		return false;
	}

	char cTemp[ASN1_MAX_NAME];


	/* UTStarcom ASN.1 CDR descriptor header...
	Passby_Token("w3g_cdr");
	Passby_Token("{");
	Passby_Token(".");
	Passby_Token(".");
	Passby_Token(".");
	Passby_Token("}");

	Passby_Token("DEFINITIONS");
	Passby_Token("IMPLICIT");
	Passby_Token("TAGS");
	Passby_Token("::=");

	Passby_Token("BEGIN");
	*/

	// Ignore the headers, search the 'BEGIN'
	try {
	/*
		for (;;) {
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "BEGIN")) {
				break;
			}
		}
	*/
		m_module_name[0] = 0;
		m_module_objname[0] = 0;
		m_module_objid = 0;
		m_module_default_tags = ASN1_MODULE_EXPLICIT_TAGS;

		Get_Token(m_module_name, sizeof(m_module_name));
		Passby_Token("{");
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "}")) {
			// Nothing to do here
		} else if (Is_Token(cTemp, ".")) {
			Passby_Token(".");
			Passby_Token(".");
			Passby_Token("}");
		} else if (Is_Number(cTemp)) {
			m_module_objid = atoi(cTemp);
			Passby_Token("}");
		} else {
			sncpy(m_module_objname, cTemp, sizeof(m_module_objname));
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Number(cTemp)) {
				m_module_objid = atoi(cTemp);
				Passby_Token("}");
			} else if (!Is_Token(cTemp, "}")) {
				throw "identifier '}' or tag expected!";
			}
		}

		Passby_Token("DEFINITIONS");

		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "EXPLICIT")) {
			Passby_Token("TAGS");
			m_module_default_tags = ASN1_MODULE_EXPLICIT_TAGS;
		} else if (Is_Token(cTemp, "IMPLICIT")) {
			Passby_Token("TAGS");
			m_module_default_tags = ASN1_MODULE_IMPLICIT_TAGS;
		} else if (Is_Token(cTemp, "AUTOMATIC")) {
			Passby_Token("TAGS");
			m_module_default_tags = ASN1_MODULE_AUTOMATIC_TAGS;
		} else {
			Unget_Token(cTemp);
		}

		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "EXTENSIBILITY")) {
			Passby_Token("IMPLIED");
		} else {
			Unget_Token(cTemp);
		}
		Passby_Token("::=");
		Passby_Token("BEGIN");
	} catch (const std::string msg) {
		on_Exception("can't found 'BEGIN'!!!");
		return false;
	}


	// Start the definitions
	try {
		for (;;) {
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "END")) {
				break;
			} else {
				Unget_Token(cTemp);
			}
			if (!parse_Define()) {
				return false;
			}
			Get_Token(cTemp, sizeof(cTemp));
			if (!Is_Token(cTemp, ";")) {
				Unget_Token(cTemp);
			}
		}
	} catch (const std::string& msg) {
		on_Exception(msg);
		return false;
	}

	// Now, we've constructed the intermediate node list...
/*
	node_list_Print();
*/
	// Create the tree according to the node list...
	if ((m_pRoot = create_Tree(cRootName))==NULL) {
		return false;
	}


	//node_list_Print();
	// Now, release the intmediate node list...
	node_list_Clear();

	return true;
}


bool CASN1Desc::parse_Define()
{

	ASN1_NODE* pNode = asn1_node_Alloc();
	try {
		Get_Token(pNode->name, sizeof(pNode->name));
		char cTemp[ASN1_MAX_NAME];
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "::=")) {
			parse_TypeDefine(pNode);
		} else {
			Unget_Token(cTemp);
			parse_ConstDefine(pNode);
		}

		m_node_list.push_back(pNode);

		return true;
	} catch (const std::string msg) {
		std::string msg2 = "parse '";
		msg2 = msg2+pNode->name+"' failed - "+msg;
		on_Exception(msg2);
		asn1_node_Free(pNode);
		return false;
	}
}

bool CASN1Desc::parse_TypeDefine(ASN1_NODE* pNode)
{
	char cTemp[ASN1_MAX_NAME];
	Get_Token(cTemp, sizeof(cTemp));
	if (Is_Token(cTemp, "CHOICE")) {
		pNode->type = ASN1_TYPE_CHOICE;
		parse_ChoiceBody(pNode);
	} else if (Is_Token(cTemp, "SET")) {
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "OF")) {	// SET OF
			pNode->type = ASN1_TYPE_SET_OF;
			parse_TypeDefine(pNode);
		} else {			// SET
			pNode->type = ASN1_TYPE_SET;
			Unget_Token(cTemp);
			parse_SetBody(pNode);
		}
	} else if (Is_Token(cTemp, "SEQUENCE")) {
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "{")) {
			pNode->type = ASN1_TYPE_SEQUENCE;
			Unget_Token(cTemp);
			parse_SequenceBody(pNode);
		} else if (Is_Token(cTemp, "OF")) {	// SEQUENCE OF
			pNode->type = ASN1_TYPE_SEQUENCE_OF;
			parse_TypeDefine(pNode);
		} else if (Is_Token(cTemp, "SIZE")) {
			Unget_Token(cTemp);
			parse_SizeDesc(pNode);
			Passby_Token("OF");
			pNode->type = ASN1_TYPE_SEQUENCE_OF;
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "{")) {
				Unget_Token(cTemp);
				parse_SequenceBody(pNode);
			} else {
				Unget_Token(cTemp);
				parse_TypeDefine(pNode);
			}
		} else {
			std::string msg = "expected '{', 'OF' or 'SIZE', but '";
			msg = msg + cTemp + "' encountered!!!";
			throw msg;
		}
	} else if (Is_Token(cTemp, "ENUMERATED")) {
		pNode->type = ASN1_TYPE_ENUMERATED;
		sncpy(pNode->tname, "ENUMERATED", sizeof(pNode->tname));
		parse_EnumeratedBody(pNode);
	} else if (Is_Token(cTemp, "CLASS")) {
		pNode->type = ASN1_TYPE_CLASS;
		parse_ClassBody(pNode);
	} else if (Is_Token(cTemp, "OCTET")) {
		Passby_Token("STRING");
		sncpy(pNode->tname, "OCTET STRING", sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_OCTET_STRING;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			parse_SizeDesc(pNode);
			Passby_Token(")");
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "BIT")) {
		Passby_Token("STRING");
		sncpy(pNode->tname, "BIT STRING", sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_BIT_STRING;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			parse_SizeDesc(pNode);
			Passby_Token(")");
		} else if (Is_Token(cTemp, "{")) {
			Unget_Token(cTemp);
			parse_EnumeratedBody(pNode);
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "INTEGER")) {
		sncpy(pNode->tname, cTemp, sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_INTEGER;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "SIZE")) {
				Unget_Token(cTemp);
				parse_SizeDesc(pNode);
			} else {
				Unget_Token(cTemp);
				parse_RangeDesc(pNode);
			}
			Passby_Token(")");
		} else if (Is_Token(cTemp, "{")) {
			Unget_Token(cTemp);
			parse_EnumeratedBody(pNode);
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "BOOLEAN")) {
		sncpy(pNode->tname, cTemp, sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_BOOLEAN;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "DEFAULT")) {
			Get_Token(cTemp, sizeof(cTemp));
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "OBJECT")) {
		Passby_Token("IDENTIFIER");
		sncpy(pNode->tname, "OBJECT IDENTIFIER", sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_OBJECT_IDENTIFIER;
	} else if (Is_Token(cTemp, "IA5String")) {
		sncpy(pNode->tname, cTemp, sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_IA5String;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			parse_SizeDesc(pNode);
			Passby_Token(")");
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "GraphicString")) {
		sncpy(pNode->tname, cTemp, sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_GraphicString;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			parse_SizeDesc(pNode);
			Passby_Token(")");
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "UTF8String")) {
		sncpy(pNode->tname, cTemp, sizeof(pNode->tname));
		pNode->type = ASN1_TYPE_UTF8String;
		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "(")) {
			parse_SizeDesc(pNode);
			Passby_Token(")");
		} else {
			Unget_Token(cTemp);
		}
	} else if (Is_Token(cTemp, "IMPLICIT")) {
		pNode->explicit_flag = false;
		parse_TypeDefine(pNode);
	} else if (Is_Token(cTemp, "EXPLICIT")) {
		pNode->explicit_flag = true;
		parse_TypeDefine(pNode);
	} else if (Is_Token(cTemp, "ANY")) {
		Passby_Token("DEFINED");
		Passby_Token("BY");
		pNode->type = ASN1_TYPE_ANY_DEFINED_BY;
		parse_TypeDefine(pNode);
	} else {
		std::string tname = cTemp;

		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, ".")) {
			tname += ".";
			Passby_Token("&");
			tname += "&";
			Get_Token(cTemp, sizeof(cTemp));	// .&field
			tname += cTemp;
			tname += " ";
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "(")) {
				tname += cTemp;
				Passby_Token("{");
				tname += "{";
				Get_Token(cTemp, sizeof(cTemp));
				tname += cTemp;
				Passby_Token("}");
				tname += "}";
				Get_Token(cTemp, sizeof(cTemp));
				if (Is_Token(cTemp, "{")) {
					tname += "{";
					Passby_Token("@");
					tname += "@";
					Get_Token(cTemp, sizeof(cTemp));
					tname += cTemp;
					Passby_Token("}");
					tname += "}";
				} else {
					Unget_Token(cTemp);
				}
				Passby_Token(")");
				tname += ")";
			} else {
				Unget_Token(cTemp);
			}
		} else if (Is_Token(cTemp, "(")) {
			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "SIZE")) {
				Unget_Token(cTemp);
				parse_SizeDesc(pNode);
			} else if (Is_Number(cTemp)) {
				Unget_Token(cTemp);
				parse_RangeDesc(pNode);
			} else {
				std::string msg = "'SIZE' or number expected instead of '";
				msg = msg + cTemp + "'!!!";
				throw msg;
			}
			Passby_Token(")");
		} else {
			Unget_Token(cTemp);
		}
		strncpy(pNode->tname, tname.c_str(), sizeof(pNode->tname));
	}
	return true;
}

bool CASN1Desc::parse_ConstDefine(ASN1_NODE* pNode)
{
	parse_TypeDefine(pNode);
	Passby_Token("::=");
	parse_ConstValue(pNode);
	return true;
}

bool CASN1Desc::parse_ConstValue(ASN1_NODE* pNode)
{
	char cTemp[ASN1_MAX_NAME];
	Get_Token(cTemp, sizeof(cTemp));
	if (Is_Token(cTemp, "{")) {	// OID constant
		for (;;) {
			Get_Token(cTemp, sizeof(cTemp));	// Name
			if (Is_Token(cTemp, "}")) {
				Unget_Token(cTemp);
				break;
			}

			ASN1_NODE* pChild = asn1_node_Alloc();
			strncpy(pChild->name, cTemp, sizeof(pChild->name)-1);
			asn1_node_AddChild(pNode, pChild);

			Get_Token(cTemp, sizeof(cTemp));
			if (Is_Token(cTemp, "(")) {
				Get_Token(cTemp, sizeof(cTemp)); // id
				asn1_node_SetConstValue(pChild, cTemp);
				Passby_Token(")");
			} else {
				Unget_Token(cTemp);
			}
		}
		Passby_Token("}");
	// TODO: Also need to process string constant...
	}else {
		asn1_node_SetConstValue(pNode, cTemp);
	}
	return true;
}

bool CASN1Desc::parse_ChoiceBody(ASN1_NODE* pNode)
{
	return parse_BodyDefine(pNode);
}

bool CASN1Desc::parse_SetBody(ASN1_NODE* pNode)
{
	return parse_BodyDefine(pNode);
}

bool CASN1Desc::parse_SequenceBody(ASN1_NODE* pNode)
{
	return parse_BodyDefine(pNode);
}

// '{' &field-name [type-name [OPTIONAL]]
// {  ,&field-name [type-name [OPTIONAL]] }  '}'
bool CASN1Desc::parse_ClassBody(ASN1_NODE* pNode)
{
	char cTemp[ASN1_MAX_NAME];
	Passby_Token("{");
	for (;;) {
		Get_Token(cTemp, sizeof(cTemp));	// &
		if (Is_Token(cTemp, ".")) {
			Passby_Token(".");
			Passby_Token(".");
			break;
		}
		if (Is_Token(cTemp, "}")) {
			Unget_Token(cTemp);
			break;
		}

		// cTemp is '&'
		Get_Token(cTemp, sizeof(cTemp));	// Name

		ASN1_NODE* pChild = asn1_node_Alloc();
		pChild->name[0] = '&';
		strncpy(pChild->name+1,  cTemp, sizeof(pChild->name)-2);
		asn1_node_AddChild(pNode, pChild);

		if (Is_Token(cTemp, "OPTIONAL")) {
			pChild->optional_flag = true;
			Passby_Token(",");
		} else if (Is_Token(cTemp, "}")) {
			Unget_Token(cTemp);
			break;
		} else if (Is_Token(cTemp, ",")) {
			continue;
		} else {
			parse_TypeDefine(pChild);
			Get_Token(cTemp, sizeof(cTemp));
			if (!Is_Token(cTemp, ",")) {	// Assume it's '}'
				Unget_Token(cTemp);
				break;
			}
		}
	}
	Passby_Token("}");
	return true;
}

bool CASN1Desc::parse_EnumeratedBody(ASN1_NODE* pNode)
{
	char cTemp[ASN1_MAX_NAME];
	Passby_Token("{");
	for (;;) {
		Get_Token(cTemp, sizeof(cTemp));	// name
		if (Is_Token(cTemp, ".")) {
			Passby_Token(".");
			Passby_Token(".");
			break;
		}
		if (Is_Token(cTemp, "}")) {
			Unget_Token(cTemp);
			break;
		}

		ASN1_NODE* pChild = asn1_node_Alloc();
		strncpy(pChild->name,  cTemp, sizeof(pChild->name)-1);
		strncpy(pChild->tname, "INTEGER", sizeof(pChild->tname)-1);
		pChild->type = ASN1_TYPE_INTEGER;
		asn1_node_AddChild(pNode, pChild);

		Passby_Token("(");
		Get_Token(cTemp, sizeof(cTemp));	// id

		asn1_node_SetConstValue(pChild, cTemp);

		Passby_Token(")");
		Get_Token(cTemp, sizeof(cTemp));
		if (!Is_Token(cTemp, ",")) {	// Assume it's '}'
			Unget_Token(cTemp);
			break;
		}
	}
	Passby_Token("}");
	return true;
}

// ASN.1 syntax:
//	<Body-Define> => '{' <Body-Item> {, <Body-Item> } '}'
//	<Body-Item> => itemname [ '[' tag ']' ] <Type-Define> [OPTIONAL]
//	<Body-Item> => ...
bool CASN1Desc::parse_BodyDefine(ASN1_NODE* pNode)
{
	size_t tag = 0;
	char cTemp[ASN1_MAX_NAME];
	Passby_Token("{");
	for (;;) {
		Get_Token(cTemp, sizeof(cTemp));	// name
		if (Is_Token(cTemp, ".")) {
			Passby_Token(".");
			Passby_Token(".");
			break;
		}
		if (Is_Token(cTemp, "}")) {
			Unget_Token(cTemp);
			break;
		}

		ASN1_NODE* pChild = asn1_node_Alloc();
		strncpy(pChild->name, cTemp, sizeof(pChild->name)-1);
		asn1_node_AddChild(pNode, pChild);

		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "[")) {
			Get_Token(cTemp, sizeof(cTemp));	// id
			pChild->tag = atoi(cTemp);
			tag = pChild->tag;
			Passby_Token("]");
		} else {
			if (pNode->type!=ASN1_TYPE_CHOICE) {
				pChild->tag = tag;
			}
			Unget_Token(cTemp);
		}

		tag++;

		parse_TypeDefine(pChild);

		Get_Token(cTemp, sizeof(cTemp));
		if (Is_Token(cTemp, "OPTIONAL")) {
			pChild->optional_flag = true;
			Get_Token(cTemp, sizeof(cTemp));
		}

		if (!Is_Token(cTemp, ",")) {
			Unget_Token(cTemp);
			break;
		}
	}
	Passby_Token("}");
	return true;
}

// ASN.1 syntax:
//	<Range-Desc> => low..high
bool CASN1Desc::parse_RangeDesc(ASN1_NODE* pNode)
{
	// (low..high)
	char cTemp[ASN1_MAX_NAME];
	Get_Token(cTemp, sizeof(cTemp));	// low
	if (Is_Number(cTemp)) {
		pNode->constrain.constrain.range.low = atoi(cTemp);
	} else {
		strcpy(pNode->constrain.constrain.range.low_name,  cTemp);
	}
	Passby_Token(".");
	Passby_Token(".");
	Get_Token(cTemp, sizeof(cTemp));	// high
	if (Is_Number(cTemp)) {
		pNode->constrain.constrain.range.high = atoi(cTemp);
	} else {
	strcpy(pNode->constrain.constrain.range.high_name, cTemp);
	}
	pNode->constrain.constrain_type = ASN1_CONSTRAIN_TYPE_RANGE;
	return true;
}


// ASN.1 syntax:
//	<Size-Desc> => SIZE (low[..high])
bool CASN1Desc::parse_SizeDesc(ASN1_NODE* pNode)
{
	Passby_Token("SIZE");
	Passby_Token("(");
	char cSize1[ASN1_MAX_NAME];
	char cSize2[ASN1_MAX_NAME];
	Get_Token(cSize1, sizeof(cSize1));	// size or low
	Get_Token(cSize2, sizeof(cSize2));
	if (Is_Token(cSize2, ".")) {
		Passby_Token(".");
		Get_Token(cSize2, sizeof(cSize2));	// high
		if (Is_Number(cSize1)) {
			pNode->constrain.constrain.size2.low = atoi(cSize1);
		} else {
			strcpy(pNode->constrain.constrain.size2.low_name,  cSize1);
		}
		if (Is_Number(cSize2)) {
			pNode->constrain.constrain.size2.high = atoi(cSize2);
		} else {
			strcpy(pNode->constrain.constrain.size2.high_name, cSize2);
		}
		pNode->constrain.constrain_type = ASN1_CONSTRAIN_TYPE_SIZE2;
	} else {
		if (Is_Number(cSize1)) {
			pNode->constrain.constrain.size1.size = atoi(cSize1);
		} else {
			strcpy(pNode->constrain.constrain.size1.size_name, cSize1);
		}
		pNode->constrain.constrain_type = ASN1_CONSTRAIN_TYPE_SIZE1;
		Unget_Token(cSize2);
	}
	Passby_Token(")");
	return true;
}




//----------------------------------------------------------------------------
//
//	Token operators
//
//----------------------------------------------------------------------------


bool CASN1Desc::Is_Number(const char* cBuff) const
{
	for (const char* p = cBuff; *p; p++) {
		if (*p<'0' || '9'<*p) {
			return false;
		}
	}
	return true;
}

bool CASN1Desc::Is_Token(const char* cBuff, const char* cToken) const
{
	return is_token(cToken, cBuff);
}

bool CASN1Desc::Get_Token(char* cToken, size_t uSize)
{
	if (m_unget_flag) {
		if (*m_unget==0) {
			throw "unget buffer empty!!!";
		}
		sncpy(cToken, m_unget, uSize);
		m_unget_flag = false;
		m_unget[0] = 0;
		return true;
	} else {
		if (m_toker.GetNToken(cToken, uSize)!=0) {
			throw "identifier expected!";
		}
		return true;
	}
}

bool CASN1Desc::Unget_Token(const char* cToken)
{
	if (m_unget_flag) {
		m_unget_flag = false;
		throw "already unget a token, please check your code!!!";
	} else {
		sncpy(m_unget, cToken, sizeof(m_unget));
		m_unget_flag = true;
	}
	return true;
}

bool CASN1Desc::Passby_Token(const char* cToken)
{
	char cTemp[ASN1_MAX_NAME];
	Get_Token(cTemp, sizeof(cTemp));
	if (!Is_Token(cToken, cTemp)) {
		std::string msg = "identifier '";
		msg = msg+cToken+"' expected instead of '"+cTemp+"' !!!";
		throw msg;
	}
	return true;
}

void CASN1Desc::on_Exception(const std::string& msg) const
{
	printf("%s, line %d: %s\n",
		m_toker.GetFileName(),
		m_toker.GetLine(),
		msg.c_str());
}





//----------------------------------------------------------------------------
//
//	Syntax tree operations
//
//----------------------------------------------------------------------------

ASN1_NODE* CASN1Desc::create_Tree(const char* cRootName/* = NULL*/) const
{
	if (m_node_list.size()==0) {
		printf("Err: create description tree failed - no node defined!!!\n");
		return NULL;
	}

	const ASN1_NODE* pFound = NULL;
	if (cRootName) {
		pFound = node_list_Get(cRootName);
	} else {
		pFound = node_list_GetFirst();
	}
	if (!pFound) {
		return NULL;
	}
	ASN1_NODE* pRoot = asn1_node_Duplicate(pFound);
	if (!link_Tree(pRoot)) {
		delete_Tree(pRoot);
		return NULL;
	}
	return pRoot;
}

void CASN1Desc::delete_Tree(ASN1_NODE*& pRoot) const
{
	asn1_node_Free(pRoot);
}

bool CASN1Desc::link_Tree(ASN1_NODE* pNode, const ASN1_NODE* pParent /*= NULL*/) const
{
	link_Constrain(pNode);
	if (!pNode->children) {
		if (asn1_node_IsBasicType(pNode)) {
			return true;
		} else if (pNode->type==ASN1_TYPE_ANY_DEFINED_BY) {
			if (pParent) {
				bool bFound = false;
				while (pNode->type==ASN1_TYPE_ANY_DEFINED_BY) {
					for (const ASN1_NODE* p = pParent->children; p; p = p->brothers) {
						if (strcmp(p->name, pNode->tname)==0) {
							strcpy(pNode->tname, p->tname);
							pNode->type = p->type;
							bFound = true;
							break;
						}
					}
					if (!bFound) {
						printf("name '%s' not defined!!!\n", pNode->tname);
						break;
					}
				}
			} else {
				printf("Warning: parent is null!!!\n");
			}
		} else {
			const ASN1_NODE* pFound = node_list_Get(pNode->tname);
			if (pFound) {
				pNode->children = asn1_node_Duplicate(pFound);
			} else {
				//char buff[128] = {0};
				//sprintf(buff,"Waring: type '%s' of '%s' not defined!",pNode->tname, pNode->name);
				//DataLog g_log;
				//g_log.addLog(buff);
				//printf("Waring: type '%s' of '%s' not defined!\n", pNode->tname, pNode->name);
				//return false;
			}
		}
	}
	for (ASN1_NODE* pChild = pNode->children; pChild; pChild = pChild->brothers) {
		if (!link_Tree(pChild, pNode)) {
			return false;
		}
	}
	return true;
}

bool CASN1Desc::link_Constrain(ASN1_NODE* pNode) const
{
	if (pNode->constrain.constrain_type!=ASN1_CONSTRAIN_TYPE_NONE) {
		switch (pNode->constrain.constrain_type) {
		case ASN1_CONSTRAIN_TYPE_SIZE1:
			if (*pNode->constrain.constrain.size1.size_name) {
				if (!link_ConstantInteger(pNode->constrain.constrain.size1.size_name, pNode->constrain.constrain.size1.size)) {
					return false;
				}
			}
			break;

		case ASN1_CONSTRAIN_TYPE_SIZE2:
			if (*pNode->constrain.constrain.size2.low_name) {
				if (!link_ConstantInteger(pNode->constrain.constrain.size2.low_name, pNode->constrain.constrain.size2.low)) {
					return false;
				}
			}
			if (*pNode->constrain.constrain.size2.high_name) {
				if (!link_ConstantInteger(pNode->constrain.constrain.size2.high_name, pNode->constrain.constrain.size2.high)) {
					return false;
				}
			}
			break;

		case ASN1_CONSTRAIN_TYPE_RANGE:
			if (*pNode->constrain.constrain.range.low_name) {
				if (!link_ConstantInteger(pNode->constrain.constrain.range.low_name, pNode->constrain.constrain.range.low)) {
					return false;
				}
			}
			if (*pNode->constrain.constrain.range.high_name) {
				if (!link_ConstantInteger(pNode->constrain.constrain.range.high_name, pNode->constrain.constrain.range.high)) {
					return false;
				}
			}
			break;
		}
	}
	return true;
}

bool CASN1Desc::link_ConstantInteger(const char* cName, size_t& value) const
{
	const ASN1_NODE* pFound = node_list_Get(cName);
	if (pFound) {
		if (pFound->const_value) {
			if (Is_Number(pFound->const_value)) {
				value = atoi(pFound->const_value);
				return true;
			} else {
				return link_ConstantInteger(pFound->const_value, value);
			}
		} else {
			printf("Warning: value of constant '%s' not defined!!!\n", cName);
		}
	} else {
		printf("Warning: constant '%s' not defined!!!\n", cName);
	}
	return false;
}

void CASN1Desc::node_list_Print(FILE* fp) const
{
	fprintf(fp, "Total %lu nodes defined:\n", m_node_list.size());
	fprintf(fp, "BEGIN\n");
	fprintf(fp, "-------------------------------------------------------------\n");
	for (size_t i = 0; i<m_node_list.size(); i++) {
		//asn1_node_Print(m_node_list[i], fp);
	}
	fprintf(fp, "-------------------------------------------------------------\n");
	fprintf(fp, "END\n");
}

const ASN1_NODE* CASN1Desc::node_list_Get(const char* cNodeName) const
{
	for (register size_t i = 0; i<m_node_list.size(); i++) {
		const ASN1_NODE* pNode = m_node_list[i];
		if (strcmp(cNodeName, pNode->name)==0) {
			return pNode;
		}
	}
	return NULL;
}

const ASN1_NODE* CASN1Desc::node_list_GetFirst() const
{
	if (m_node_list.size()>0) {
		return m_node_list[0];
	} else {
		return NULL;
	}
}

void CASN1Desc::node_list_Clear()
{
	for (size_t i = 0; i<m_node_list.size(); i++) {
		asn1_node_Free(m_node_list[i]);
	}
	m_node_list.clear();
}

void CASN1Desc::SetFileName(const char* fname)
{
	sncpy(m_fname, fname, sizeof(m_fname));
}

const char* CASN1Desc::GetFileName() const
{
	return m_fname;
}










//-----------------------------------------------------------------------------
//
//	Implementation for CASN1Token
//
//-----------------------------------------------------------------------------

CASN1Token::CASN1Token()
{
	m_fp = NULL;
	m_line = 0;
	m_ptr = NULL;
	memset(m_fname, 0, sizeof(m_fname));
	memset(m_buff, 0, sizeof(m_buff));
}

CASN1Token::CASN1Token(const char* fname)
{
	m_fp = NULL;
	m_line = 0;
	m_column = 0;
	m_ptr = NULL;
	memset(m_fname, 0, sizeof(m_fname));
	memset(m_buff, 0, sizeof(m_buff));
	Init(fname);
}

CASN1Token::~CASN1Token()
{
	Free();
}

int CASN1Token::Init(const char* fname)
{
	Free();
	sncpy(m_fname, fname, sizeof(m_fname));
	if (NULL==(m_fp = fopen(m_fname, "r"))) {
		printf("open '%s' failed: %s!\n", m_fname, strerror(errno));
		return -1;
	}
	return 0;
}

void CASN1Token::Free()
{
	if (m_fp) {
		fclose(m_fp);
		m_fp = NULL;
	}
	m_line = 0;
	m_column = 0;
	m_ptr = NULL;
	memset(m_fname, 0, sizeof(m_fname));
	memset(m_buff, 0, sizeof(m_buff));
}

int CASN1Token::GetNToken(char* buff, size_t size)
{
	for (;;) {
		if (NULL==m_ptr) {
			// Load next line
			if (!fgets(m_buff, sizeof(m_buff), m_fp)) {
				if (!feof(m_fp)) {
					// Error ocurss
				}
				return -1;
			}
			m_ptr = m_buff;
			m_line++;
		}
		// No more token in this line
		if (NULL==(m_ptr = get_next_token(buff, size, m_ptr, ASN1_TOKENS))) {
			continue;
		}
		// Comments
		if (is_token("--", buff)) {
			m_ptr = NULL;	// Discard remain
			continue;
		}
		m_column = ((m_ptr-m_buff)+1);
		break;
	}

	return 0;
}


bool CASN1Token::IsToken(const char* buff, const char* token) const
{
	return strncmp(buff, token, strlen(token))==0;
}



//-----------------------------------------------------------------------------
//
//	Assistant functions
//
//-----------------------------------------------------------------------------

ASN1_NODE* asn1_node_Alloc()
{
	ASN1_NODE* pNode = new ASN1_NODE;
	memset(pNode, 0, sizeof(*pNode));
	pNode->tag = ASN1_TAG_NONE;
	return pNode;
}

/*****************************************************************************
* Function:
*	Duplicate a subtree. The new tree has been allocated from memory.
* Parameter:
*	pNode - The source subtree.
* Return:
*	The new subtree same as pNode.
* History:
*	2004-11-19	Mike Liu	Use loop instead of recursion call
*					to duplicate child nodes.
*****************************************************************************/
ASN1_NODE* asn1_node_Duplicate(const ASN1_NODE* pNode)
{
	ASN1_NODE* pNew = asn1_node_Alloc();
	memcpy(pNew, pNode, sizeof(*pNew));
	if (pNode->const_value) {
		pNew->const_value = asn1_node_DupStr(pNode->const_value);
	}
	pNew->brothers = NULL;
	pNew->children = NULL;

	ASN1_NODE* pLastNewChild = NULL;
	for (const ASN1_NODE* pChild = pNode->children; pChild; pChild = pChild->brothers) {
		if (pLastNewChild==NULL) {
			pNew->children = asn1_node_Duplicate(pChild);
			pLastNewChild = pNew->children;
		} else {
			pLastNewChild->brothers = asn1_node_Duplicate(pChild);
			pLastNewChild = pLastNewChild->brothers;
		}
	}

	return pNew;
}

/*****************************************************************************
* Function:
*	Delete memory space of pNode (include subtree of the node). And then,
*	set the pNode to NULL.
* Parameter:
*	pNode - The subtree to be free.
* History:
*	2004-11-19	Mike Liu	Use loop instead of recursion call
*					to free child nodes.
*****************************************************************************/
void asn1_node_Free(ASN1_NODE* &pNode)
{
	if (pNode) {
		register ASN1_NODE* pChild = pNode->children;
		while (pChild) {
			register ASN1_NODE* pNextChild = pChild->brothers;
			asn1_node_Free(pChild);
			pChild = pNextChild;
		}

		if (pNode->const_value) {
			delete []pNode->const_value;
			pNode->const_value = NULL;
		}
		delete pNode;
		pNode = NULL;
	}
}

void asn1_node_AddChild(ASN1_NODE* pParent, ASN1_NODE* pChild)
{
	if (pParent) {
		if (pParent->children) {
			asn1_node_AddBrother(pParent->children, pChild);
		} else {
			pParent->children = pChild;
		}
	}
}

void asn1_node_AddBrother(ASN1_NODE* pNode1, ASN1_NODE* pNode2)
{
	if (pNode1) {
		ASN1_NODE* pNode = pNode1;
		while (pNode->brothers) {
			pNode = pNode->brothers;
		}
		pNode->brothers = pNode2;
	}
}

void asn1_node_SetConstValue(ASN1_NODE* pNode, const char* cValue)
{
	if (pNode->const_value) {
		delete []pNode->const_value;
	}
	pNode->const_value = asn1_node_DupStr(cValue);
}

bool asn1_node_IsBasicType(const ASN1_NODE* pNode)
{
	return	ASN1_TYPE_OCTET_STRING==pNode->type ||
		ASN1_TYPE_BIT_STRING==pNode->type ||
		ASN1_TYPE_INTEGER==pNode->type ||
		ASN1_TYPE_BOOLEAN==pNode->type ||
		ASN1_TYPE_OBJECT_IDENTIFIER==pNode->type ||
		ASN1_TYPE_IA5String==pNode->type ||
		ASN1_TYPE_GraphicString==pNode->type;

}

const ASN1_NODE* asn1_node_GetSubTree(const ASN1_NODE* pParent, size_t *oid, int oidlen)
{

	if (oidlen<=0) {
		return pParent;
	}
	const ASN1_NODE* pNode = NULL;
	for (pNode = pParent->children; pNode; pNode = pNode->brothers) {
		if (pNode->tag==ASN1_TAG_NONE)
		{
			pNode = pNode->children;
		}
		if (pNode->tag==*oid) {
			return asn1_node_GetSubTree(pNode, oid+1, oidlen-1);
		}
	}
	return NULL;
}

char* asn1_node_DupStr(const char* cStr)
{
	char* p = new char[strlen(cStr)+1];
	strcpy(p, cStr);
	return p;
}


#define INDENT_asn1_node_Print	"    "
void asn1_node_Print(const ASN1_NODE* pNode, FILE* fp, int level)
{

	for (int i = 0; i<level; i++) {
		fprintf(fp, INDENT_asn1_node_Print);
	}
	fprintf(fp, "%s ", pNode->name);
	if (pNode->tag!=ASN1_TAG_NONE) {
		fprintf(fp, "[%lu] ", pNode->tag);
	}
	fprintf(fp, "::= ");
	if (pNode->explicit_flag) {
		fprintf(fp, "EXPLICIT ");
	}
	switch (pNode->type) {
	case ASN1_TYPE_ANY_DEFINED_BY:
		fprintf(fp, "ANY DEFINED BY ");
		break;
	case ASN1_TYPE_CHOICE:
		fprintf(fp, "CHOICE ");
		break;
	case ASN1_TYPE_SEQUENCE:
		fprintf(fp, "SEQUENCE ");
		break;
	case ASN1_TYPE_SEQUENCE_OF:
		fprintf(fp, "SEQUENCE OF ");
		break;
	case ASN1_TYPE_SET:
		fprintf(fp, "SET ");
		break;
	case ASN1_TYPE_SET_OF:
		fprintf(fp, "SET OF ");
		break;
	}
	fprintf(fp, "%s ", pNode->tname);
	switch (pNode->constrain.constrain_type) {
	case ASN1_CONSTRAIN_TYPE_NONE:
		break;
	case ASN1_CONSTRAIN_TYPE_SIZE1:
		fprintf(fp, "(SIZE (%lu)) ", pNode->constrain.constrain.size1.size);
		break;
	case ASN1_CONSTRAIN_TYPE_SIZE2:
		fprintf(fp, "(SIZE (%lu..%lu)) ", pNode->constrain.constrain.size2.low, pNode->constrain.constrain.size2.high);
		break;
	case ASN1_CONSTRAIN_TYPE_RANGE:
		fprintf(fp, "(%lu..%lu) ", pNode->constrain.constrain.range.low, pNode->constrain.constrain.range.high);
		break;
	}
	if (pNode->optional_flag) {
		fprintf(fp, "OPTIONAL ");
	}

	if (pNode->const_value) {
		fprintf(fp, "= %s ", pNode->const_value);
	}
	if (pNode->children) {
		fprintf(fp, "{\n");
		asn1_node_Print(pNode->children, fp, level+1);
		for (int i = 0; i<level; i++) {
			fprintf(fp, INDENT_asn1_node_Print);
		}
		fprintf(fp, "}\n");
	} else {
		fprintf(fp, "\n");
	}
	if (pNode->brothers) {
		asn1_node_Print(pNode->brothers, fp, level);
	}
}


void AsnNodePrint(std::vector<ShowNode>& nodelist,string& namesave,std::vector<ASNShowNode>& asnlist,char* name,char* buff,const ASN1_NODE* pNode)
{

	if(pNode == NULL)
		return;
	ASNShowNode tmp;
	if(pNode->tag != -1)
	{
			char name_tmp[128] = {0};
			strncpy(name_tmp,name,strlen(name));
			sprintf(name_tmp+strlen(name_tmp),".%s",pNode->name);
			strncpy(tmp.node_name,name_tmp,strlen(name_tmp));
			asnlist.push_back(tmp);
			string root_name = namesave+".mMTelInformation.listOfSupplServices";
			if(strcmp(tmp.node_name,root_name.c_str())==0)
			{
				char tmp1[32]={'\0'};
				strcpy(tmp1,tmp.node_name);
				sprintf(tmp.node_name+strlen(tmp.node_name),"%s","[0]");
				asnlist.push_back(tmp);

				memset(tmp.node_name,'\0',sizeof(tmp.node_name));
				strcpy(tmp.node_name,tmp1);
				sprintf(tmp.node_name+strlen(tmp.node_name),"%s","[1]");
				/*std::vector<ShowNode>::iterator it = nodelist.begin();
				for(;it!=nodelist.end();it++)
					{
						if(0==strcmp(it->node_name,"aSRecord.mMTelInformation.listOfSupplServices[1]"))
							asnlist.push_back(tmp);
					}*/
				vector<ShowNode>::iterator result1 = find_if(nodelist.begin(),nodelist.end(),NodeNameFind(tmp.node_name));
				if(result1!=nodelist.end())
				{
					//printf("--tmp.name = %s\n",tmp.node_name);
					asnlist.push_back(tmp);
				}
				
				memset(tmp.node_name,'\0',sizeof(tmp.node_name));
				strcpy(tmp.node_name,tmp1);
				sprintf(tmp.node_name+strlen(tmp.node_name),"%s","[2]");
				vector<ShowNode>::iterator result2 = find_if(nodelist.begin(),nodelist.end(),NodeNameFind(tmp.node_name));
				if(result2!=nodelist.end())
				{
				//	asnlist.push_back(tmp);
				}
				
				memset(tmp.node_name,'\0',sizeof(tmp.node_name));
				strcpy(tmp.node_name,tmp1);
				sprintf(tmp.node_name+strlen(tmp.node_name),"%s","[3]");
				vector<ShowNode>::iterator result3 = find_if(nodelist.begin(),nodelist.end(),NodeNameFind(tmp.node_name));
				if(result3!=nodelist.end())
				{
				//	asnlist.push_back(tmp);
				}
			}
			sprintf(buff+strlen(buff),"%s.%s",name,pNode->name);

	}
	
	if (pNode->children) {
		if(pNode->tag != -1)
		{
			sprintf(name+strlen(name),".%s",pNode->name);
		}
		AsnNodePrint(nodelist,namesave,asnlist,name,buff,pNode->children);
		
	} else {
		if(strlen(buff)>1)
		{
			strcpy(asnlist[asnlist.size() -1].node_value,"empty_value");
		}
		memset(buff,0,sizeof(buff));
	}
	if (pNode->brothers) {
		char * p2 = strstr(name,pNode->name);
		if(p2 != NULL)
		{
			*(name+(strlen(name)-strlen(p2)-1)) = 0;
		}
		AsnNodePrint(nodelist,namesave,asnlist,name,buff,pNode->brothers);
		}
	
}




