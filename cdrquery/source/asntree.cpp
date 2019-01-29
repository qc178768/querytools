// asntree.cpp
//
// Function:
//    Use this class to parse or build a BER encoded ASN.1 cdr.
//
// History:
//	2004-11-19 Mike Liu	Fixed a potential bug in asn_free_TreeNode.
//----------------------------------------------------------------------------
//#include "stdafx.h"

//#include "asn1cdr.h"
#include "bercodec.h"
#ifndef WIN32
#include <netinet/in.h>
#endif
#include <memory.h>
#include "asntree.h"


//--------------------------------------------------
// CASNTree class
//--------------------------------------------------

CASNTree::CASNTree()
{
	m_proot = asn_alloc_TreeNode(m_alloc);
	m_proot->type = TAGTYPE_CONTEXT;
	m_proot->construct = TAGPC_CONSTRUCT;
}

CASNTree::CASNTree(const CASNTree& other)
{
	m_proot = NULL;
	Assign(other);
}

CASNTree& CASNTree::operator =(const CASNTree& other)
{
	Assign(other);
	return *this;
}

CASNTree::~CASNTree()
{
	asn_free_TreeNode(m_proot);
}

// Function:
//		Copy the tree from another.
int CASNTree::Assign(const CASNTree& other)
{
	if (this!=&other) {
		asn_free_TreeNode(m_proot);
		if ((m_proot = asn_dup_SubTree(other.m_proot, m_alloc))==NULL) {
			return -1;
		}
	}
	return 0;
}

int CASNTree::Parse(const Uchar* buff, Uint size)
{
	return Parse(buff, buff+size);
}

int CASNTree::Parse(const Uchar* buff, const Uchar* const end)
{
	asn_free_TreeNode(m_proot);
	if ((m_proot = asn_alloc_TreeNode(m_alloc))!=NULL) {
		m_proot->type = TAGTYPE_CONTEXT;
		m_proot->construct = TAGPC_CONSTRUCT;
		if (asn_parse_SubTree(m_proot, buff, end, m_alloc)!=0) {
			return -1;
		}
		asn_calculate_SubTreeLength(m_proot);
		return 0;
	} else {
		return -1;
	}
}

int CASNTree::Build(Uchar* buff, Uint& size)
{
	Uchar* p = buff;
	if (Build(p, p+size)!=0) {
		return -1;
	}
	size = p-buff;
	return 0;
}

int CASNTree::Build(Uchar*& buff, const Uchar* const end)
{
	if (!m_proot->children) {
		return -1;
	}
	return asn_build_SubTree(m_proot->children, buff, end, 0);
}

const ASNBinTreeNode* CASNTree::GetNode(Uint* oid, Uint oidlen) const
{
	return asn_get_SubTree(m_proot, oid, oidlen);
}

int CASNTree::AddNode(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode)
{
	return NodeOp(oid, oidlen, pNode, false);
}

/*
*
* Notes:
*	If pNode==NULL, will delete the node identified by oid, or
*	else, replace the node by pNode.
*
*/
int CASNTree::SetNode(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode)
{
	return NodeOp(oid, oidlen, pNode);
}

void CASNTree::DelNode(Uint* oid, Uint oidlen)
{
	NodeOp(oid, oidlen, NULL);
}

// Function:
//		Specify value of leaf node. If the node exist, reset it's value, else
//		insert a new leaf node with the value specified.
int CASNTree::SetNode(Uint* oid, Uint oidlen, const Uchar* raw, Uint rawlen)
{
	ASNBinTreeNode Node;
	memset(&Node, 0, sizeof(Node));
	if (asn_fill_LeafValue(&Node, raw, rawlen)!=0) {
		printf(" ---135 %d\n",Node.tag);
		return -1;
	}
	printf(" ---138 %d\n",Node.tag);
	return SetNode(oid, oidlen, &Node);
}


int CASNTree::GetElemCount(Uint* oid, Uint oidlen) const
{
	const ASNBinTreeNode* pNode = GetElem(oid, oidlen, 0);
	if (!pNode) {
		return 0;
	}
	register int ret = 0;
	while (pNode) {
		ret++;
		pNode = pNode->sequenceof;
	}
	return ret;
}

const ASNBinTreeNode* CASNTree::GetElem(Uint* oid, Uint oidlen, int index) const
{
	
	const ASNBinTreeNode* pNode = GetNode(oid, oidlen);
	if (pNode) {
		if (NULL==(pNode = pNode->children)) {
			return NULL;
		}
		for (register int i = 0; i<index; i++) {
			if (NULL==(pNode = pNode->sequenceof)) {
				break;
			}
		}
		return pNode;
	} else {
		return NULL;
	}
}

int CASNTree::AddElem(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode)
{
	if (!pNode) {
		return -1;
	}
	Uint eoidlen = oidlen+1;
	Uint* eoid = new Uint[eoidlen];
	memcpy(eoid, oid, sizeof(*eoid)*oidlen);
	eoid[eoidlen-1] = pNode->tag;
	int ret = NodeOp(eoid, eoidlen, pNode, false);
	delete []eoid;
	return ret;
}

int CASNTree::DelElem(Uint* oid, Uint oidlen, int index)
{
	ASNBinTreeNode* pParent = const_cast<ASNBinTreeNode*>(GetNode(oid, oidlen));
	ASNBinTreeNode* pElem = pParent->children;
	if (!pElem) {
		return -1;
	}

	if (index<0) {	// Del all
		asn_delete_Child(pParent, pElem->tag);
	} else if (index==0) {
		pParent->children= pElem->sequenceof;
		pElem->sequenceof = NULL;
		asn_free_TreeNode(pElem);
	} else {
		ASNBinTreeNode* pPrev = NULL;
		for (register int i = 0; i<index; i++) {
			pPrev = pElem;
			if (NULL==(pElem = pElem->sequenceof)) {
				return -1;
			}
		}
		pPrev->sequenceof = pElem->sequenceof;
		pElem->sequenceof = NULL;
		asn_free_TreeNode(pElem);
	}

	asn_calculate_SubTreeLength(m_proot);

	return 0;
}

const ASNBinTreeNode* CASNTree::GetTree() const
{
	if (m_proot) {
		return m_proot->children;
	} else {
		return NULL;
	}
}

Uint CASNTree::GetSize() const
{
	if (!m_proot) {
		return 0;
	}
	return m_proot->length;
}

int CASNTree::NodeOp(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode, bool bReplace)
{
	if (oidlen<=0) {
		return -1;
	}

	ASNBinTreeNode* pParent = NULL;
	if (pNode) {	// Add or replace node
		// If parent not exist, generate it.
		if (NULL==(pParent = asn_get_gen_SubTree(m_proot, oid, oidlen-1, m_alloc, true))) {
			return -1;
		}
	} else {	// Del node
		if (NULL==(pParent = (ASNBinTreeNode*)asn_get_SubTree(m_proot, oid, oidlen-1))) {
			return 0;
		}
	}

	if (!pParent->construct) {
		return -1;
	}

	int tag = oid[oidlen-1];

	if (bReplace) {
		asn_delete_Child(pParent, tag);
	}

	if (pNode) {
		if (!asn_insert_Child(pParent, tag, pNode, m_alloc)) {
			return -1;
		}
	}

	asn_calculate_SubTreeLength(m_proot);

	return 0;
}

void CASNTree::Print(FILE* fp, size_t index) const
{
	if (!m_proot->children) {
		return;
	}
	size_t name[MAXNAMELEN];
	asn_print_SubTree(fp, m_proot->children, name, 0, index);
}






//--------------------------------------------------
//
// Local assistant functions
//
//--------------------------------------------------

/*
* Function:
*	Parse the raw data and construct a tree.
* Parameters:
*	pParent	- INPUT, Pointer to parent node.
*	buff	- INPUT, The buffer start.
*				OUTPUT, The buffer stop.
*	end		- INPUT, Pointer to after last char of the buffer.
*	level	- INPUT, Level of the subtree.
*/
int asn_parse_SubTree(ASNBinTreeNode* pParent, const Uchar*& buff, const Uchar* const end, TreeNodeAlloc& alloc, int level)
{
	if (buff==end) {
		return 0;
	} else if (buff>end) {
		return -1;
	}

	while (buff<end) {
		ASNBinTreeNode* pNode = asn_alloc_TreeNode(alloc);
		if (!pNode) {
			return -1;
		}

		pNode->type = (*buff & TAGTYPE_MASK);
		pNode->construct = (buff[0] & TAGPC_MASK);
		pNode->tag = (*buff & TAGVALUE_MASK);
		if (pNode->tag==31) {
			buff++;
			if (buff>=end) {
				asn_free_TreeNode(pNode);
				return -1;
			}
			pNode->tag = 0;
			for (;;) {
				pNode->tag |= ((*buff) & 0x7f);
				if (((*buff) & 0x80)==0) {
					break;
				} else {
					pNode->tag = pNode->tag<<7;
				}
				buff++;
				if (buff>=end) {
					asn_free_TreeNode(pNode);
					return -1;
				}
			}
		}

		buff++;	// Skip tag octet
		if (buff>=end) {
			asn_free_TreeNode(pNode);
			return -1;
		}

		if (asn_parse_Length(buff, end, pNode->length)) {	// Skip length octets
			asn_free_TreeNode(pNode);
			return -1;
		}

		if (buff+pNode->length>end) {
			asn_free_TreeNode(pNode);
			return -1;
		}

		if (pNode->construct) {
			const Uchar* newend = buff+pNode->length;
			if (asn_parse_SubTree(pNode, buff, newend, alloc, level+1)) {
				asn_free_TreeNode(pNode);
				return -1;
			}
		} else {
			pNode->value = new Uchar[pNode->length];
			if (!pNode->value) {
				asn_free_TreeNode(pNode);
				return -1;
			}
			memcpy(pNode->value, buff, pNode->length);
			buff+=pNode->length;
		}

		if (asn_add_SubTree(pParent, pNode)) {
			asn_free_TreeNode(pNode);
			return -1;
		}
	}
	return 0;
}

/*
* Function:
*	Serialize the subtree.
* Parameters:
*	pNode	- INPUT, The subtree to be serialize.
*	buff	- INPUT, The start of storage of the serialize buffer;
*				OUTPUT, The stop of buffer after serialize.
*	end		- INPUT, The pointer to after the latest char of the buffer.
*	level	- INPUT, Level of the subtree.
* Return:
*	0		- Success
*	else	- Failed
*/
int asn_build_SubTree(const ASNBinTreeNode* pNode, Uchar*& buff, const Uchar* const end, int level)
{
	if (buff>=end) {
		return -1;
	}

	// Tag byte
	*buff = 0;
	if (pNode->tag>=31) {	// Append tag
		*buff = (pNode->type | pNode->construct | 31);
		if (++buff>=end) {
			return -1;
		}
		//*buff = pNode->tag;
		if (pNode->tag<=0x7f) { // 7 bits
			*buff = pNode->tag;
		} else if (pNode->tag<=0x3fff) { // 14 bits
			*buff = (0x80|((pNode->tag&0x3f80)>>7));
			if (++buff>=end) {
				return -1;
			}
			*buff = (pNode->tag&0x7f);
		} else if (pNode->tag<=0x1fffff) { // 21 bits
			*buff = (0x80|((pNode->tag&0x1fc000)>>14));
			if (++buff>=end) {
				return -1;
			}
			*buff = (0x80|((pNode->tag&0x3f80)>>7));
			
			if (++buff>=end) {
				return -1;
			}
			*buff = (pNode->tag&0x7f);
		} else if (pNode->tag<=0xfffffff) { // 28 bits
			*buff = (0x80|(pNode->tag&0xfe00000)>>21);
			if (++buff>=end) {
				return -1;
			}
			
			*buff = (0x80|((pNode->tag&0x1fc000)>>14));
			if (++buff>=end) {
				return -1;
			}
			
			*buff = (0x80|((pNode->tag&0x3f80)>>7));
			if (++buff>=end) {
				return -1;
			}
			
			*buff = (pNode->tag&0x7f);
		} else {
			// Here should be report error!
			printf("Error: tag size is wrong!\n");
		}

	} else {
		*buff = (pNode->type | pNode->construct | pNode->tag);
	}
	if (++buff>=end) {
		return -1;
	}

	// Length
	if (asn_build_Length(buff, end, pNode->length)!=0) {
		return -1;
	}
	if (buff>=end) {
		return -1;
	}

	// Value
	if (!pNode->construct) {
		if (buff+pNode->length>=end) {
			return -1;
		}
		memcpy(buff, pNode->value, pNode->length);
		buff += pNode->length;
		return 0;
	}

	// Biuld Children
	for (const ASNBinTreeNode* pChild = pNode->children; pChild; pChild = pChild->brothers) {
		// Build Sequence Of
		for (const ASNBinTreeNode* pseqof = pChild; pseqof; pseqof = pseqof->sequenceof) {
			if (asn_build_SubTree(pseqof, buff, end, level+1)!=0) {
				return -1;
			}
		}
	}

	return 0;
}

/*
* Function:
*	Get the total size of the subtree.
* Parameters:
*	pParent	- Pointer to the subtree.
*/
Uint asn_calculate_SubTreeLength(ASNBinTreeNode* pParent, int level)
{
	if (!pParent) {
		return 0;
	}

	// Length of tag.
	Uint tagsize = 1;
         if (pParent->tag>=31) {
		if (pParent->tag<=0x7f) { // 7 bits
			tagsize += 1;
		} else if (pParent->tag<=0x3fff) { // 14 bits
			tagsize += 2;
		} else if (pParent->tag<=0x1fffff) { // 21 bits
			tagsize += 3;
		} else if (pParent->tag<=0xfffffff) { // 28 bits
			tagsize += 4;
		} else {
			// Here should be report error!
			printf("Error, tag size is wrong!\n");
		}
	}

	Uint valsize = 0;
	if (!pParent->construct) {
		valsize = pParent->length;
	} else {
		pParent->length = 0;
		for (ASNBinTreeNode* pChild = pParent->children; pChild; pChild = pChild->brothers) {
			for (ASNBinTreeNode* pSeqOf = pChild; pSeqOf; pSeqOf = pSeqOf->sequenceof) {
				pParent->length+=asn_calculate_SubTreeLength(pSeqOf, level+1);
			}
		}
		valsize = pParent->length;
	}

	Uint lensize = 0;
	if (pParent->length<=0x7f) {
		lensize = 1;
	} else if (pParent->length<=0xff) {
		lensize = 2;
	} else if (pParent->length<=0xffff) {
		lensize = 3;
	} else if (pParent->length<=0xffffff) {
		lensize = 4;
	} else if (pParent->length<=0xffffffff) {
		lensize = 5;
	} else {
		lensize = 1;
	}

	return (tagsize+lensize+valsize);
}

void asn_print_SubTree(FILE* fp, const ASNBinTreeNode* pNode, size_t* name, size_t level, Uint index)
{
	if (!fp || !pNode) {
		return;
	}
	name[level] = pNode->tag;
	//printf("[%s] [%d] level = %d name[level] = %d \n",__FILE__,__LINE__,level,name[level]);
/*	for (int i = 0; i<level; i++) {
		for (int j = 0; j<INDENT; j++) {
			fprintf(fp, " ");
		}
	}
*/		
	//	printf("[%s] [%d]  name = %d\n",__FILE__,__LINE__,*name);
		ASNName asnname((Uint*)name, (Uint)level+1);
//		printf("[%s] [%s] [%d] asnname.str() = %s,asnname.oids() = %d\n",__FILE__,__func__,__LINE__,asnname.str(),*asnname.oids());
		char* csLongName = NULL;
		string2 longname;
		if (ber_getlongname(asnname.str(), longname, 0))
		{
	//		printf("[%s] [%d] longname.c_str() = %s\n",__FILE__,__LINE__,longname.c_str());
		}
		else
		{
			//printf("[%s] [%d] longname.c_str()=> %s\n",__FILE__,__LINE__,longname.c_str());
			csLongName = longname;
		}
	/*asn_print_Type(fp, pNode->type);
	fprintf(fp,  " ");
	fprintf(fp, "[% 3lu]", (Uint)pNode->tag);
	fprintf(fp, " ");
	fprintf(fp, "%s", pNode->construct ? "Construct" : "Primary");
	fprintf(fp, " Len=% 3u", pNode->length);*/
	if (!pNode->construct) {
//		fprintf(fp, " ");


		const ASNNameHandle* hName = ber_getnamehandle(asnname.str(), index);
		string2 svalue;
//		fprintf(fp, "'");
		if (ber_decode(hName, pNode->value, pNode->length, svalue, index)==0) {
			if (svalue.size()>0) {
				//fprintf(fp, "%s", svalue.c_str());
				printf("[%s] [%d] longname = svalue:%s = %s\n",__FILE__,__LINE__,longname.c_str(),svalue.c_str());
			}
		}
		/*fprintf(fp, "'");
		fprintf(fp, " [");
		for (Uint i = 0; i<pNode->length; i++) {
			fprintf(fp, "%02X", (Uint)pNode->value[i]);
		}
		fprintf(fp, "]");*/
	}
//	fprintf(fp, "\n");

	// Print Children
	for (const ASNBinTreeNode* pChild = pNode->children; pChild; pChild = pChild->brothers) {
		// Print Sequence Of
		for (const ASNBinTreeNode* pSeqOf = pChild; pSeqOf; pSeqOf = pSeqOf->sequenceof) {
			asn_print_SubTree(fp, pSeqOf, name, level+1, index);
		}
	}
}

/*
* Function:
*	Create a new subtree same as source subtree.
* Parameters:
*	pNode	- The source subtree.
* Return:
*	NULL	- Failed.
*	else	- Pointer to the new allocated subtree.
*/
ASNBinTreeNode* asn_dup_SubTree(const ASNBinTreeNode* pNode, TreeNodeAlloc& alloc)
{
	ASNBinTreeNode* pNewNode = asn_alloc_TreeNode(alloc);
	if (!pNewNode) {
		return NULL;
	}

	pNewNode->type = pNode->type;
	pNewNode->tag = pNode->tag;
	pNewNode->construct = pNode->construct;

	if (!pNewNode->construct) {	// Primary
		if (asn_fill_LeafValue(pNewNode, pNode->value, pNode->length)!=0) {
			asn_free_TreeNode(pNewNode);
			return NULL;
		}
	} else {				// Construct
		pNewNode->children = NULL;
		for (const ASNBinTreeNode* pChild = pNode->children; pChild; pChild = pChild->brothers) {
			for (const ASNBinTreeNode* pSeqOf = pChild; pSeqOf; pSeqOf = pSeqOf->sequenceof) {
				ASNBinTreeNode* pNewSeqOf = asn_dup_SubTree(pSeqOf, alloc);
				if (!pNewSeqOf) {
					asn_free_TreeNode(pNewNode);
					return NULL;
				}
				if (asn_add_SubTree(pNewNode, pNewSeqOf)!=0) {
					asn_free_TreeNode(pNewSeqOf);
					asn_free_TreeNode(pNewNode);
					return NULL;
				}
			}
		}
	}
	return pNewNode;
}


/*
* Function:
*	Allocate a tree node memory block, and memset to zero.
* Return:
*	The new allocated tree node
*/
ASNBinTreeNode* asn_alloc_TreeNode(TreeNodeAlloc& alloc)
{
	ASNBinTreeNode* pNode = new(alloc.alloc()) ASNBinTreeNode;
	if (pNode) {
		// Base information
		pNode->type = 0;				// Node type.
		pNode->tag = 0;				// Node tag.
		pNode->construct = 0;			// Is construct?

		// Valid only when this is a primary
		pNode->length = 0;				// Size of raw value.
		pNode->value = NULL;				// Pointer to raw value.
		// Inited by constructor...
		// pNode->svalue;

		// Valid only when this is a CONSTRUCT
		pNode->children = NULL;			// Children list of this node.

		// Other linkage
		pNode->brothers = NULL;			// Brothers list of this node.
		pNode->sequenceof = NULL;		// Seqeuence-of list of this node.
	}
	return pNode;
}

/*
* Function:
*	Free a subtee pointed by pNode.
* Parameters:
*	pNode	- Pointer to the subtree.
*
* History:
*	2004-11-19 Mike Liu:	Used the loop instead of recursion call to
*				free the child and sequence of the node.
*/
void asn_free_TreeNode(ASNBinTreeNode* pNode)
{
	if (!pNode) {
		return;
	}

	// Remove all child of this node
	register ASNBinTreeNode* pChild = pNode->children;
	while (pChild) {
		register ASNBinTreeNode* pNextChild = pChild->brothers;
		register ASNBinTreeNode* pSeqOf = pChild;
		while (pSeqOf) {
			ASNBinTreeNode* pNextSeqOf = pSeqOf->sequenceof;
			asn_free_TreeNode(pSeqOf);
			pSeqOf = pNextSeqOf;
		}
		pChild = pNextChild;
	}

	// Now remove this node.
	if (pNode->value) {
		delete []pNode->value;
	}
	delete pNode;
}

/*
* Function:
*	Get node of the sub-tree according to the oid from parent level.
* Parameters:
*	pParent - Pointer to the parent node.
*	oid - Name of expected subtree.
*	oidlen - Count of name.
*/
const ASNBinTreeNode* asn_get_SubTree(const ASNBinTreeNode* pParent, Uint* oid, Uint oidlen)
{
	//printf(" ---722 %d\n",*oid);
	if (oidlen<=0) {
		//printf("---738--")
		return pParent;
	}
	const ASNBinTreeNode* pNode = 0;
	for (pNode = pParent->children; pNode; pNode = pNode->brothers) {
		if (pNode->tag==(Uint)(*oid)) {
			return asn_get_SubTree(pNode, oid+1, oidlen-1);
		}
	}
	return NULL;
}

/*
* Function:
*	Get node of the sub-tree according to the oid from parent level.
*	If node NOT exist, create them. The intermediate node will be
*	create as construct type, the leaf node will be constructed by
*	indicate of bConstruct.
*	If node exist, return it directly.
* Parameters:
*	pParent - Pointer to the parent node.
*	oid - Name of expected subtree.
*	oidlen - Count of name.
*	bConstruct - Indicate the leaf node type if need to create.
*/
ASNBinTreeNode* asn_get_gen_SubTree(ASNBinTreeNode* pParent, Uint* oid, Uint oidlen, TreeNodeAlloc& alloc, bool bConstruct, Uchar type)
{
	ASNBinTreeNode* pNode = pParent;
	ASNBinTreeNode* pChild = NULL;
	for (int i = 0; i<oidlen; i++) {
		if ((pChild = asn_get_Child(pNode, oid[i]))==NULL) {
			if ((pChild = asn_alloc_TreeNode(alloc))==NULL) {
				// Lack of memory.
				return NULL;
			}
			pChild->type = type;
			pChild->tag = oid[i];
			pChild->brothers = pNode->children;
			pNode->children = pChild;
			if (i<oidlen-1) {	// Not a Leaf
				pChild->construct |= TAGPC_CONSTRUCT;
			} else {
				if (bConstruct) {
					pChild->construct |= TAGPC_CONSTRUCT;
				}
			}
		} else {	// Already exist this node
			if (i<oidlen-1) {	// Not a leaf
				if (!pChild->construct) {
					return NULL;
				}
			}
		}
		pNode = pChild;
	}
	return pChild;
}


/*
* Function:
*	Set the value of primary node.
* Parameters:
*	pNode	- The primary node to be set value, it MUST be a primary.
*	raw		- The raw value.
*	rawlen	- Size of the raw value.
* Return:
*	0		- Success.
*	else	- Failed.
*/
int asn_fill_LeafValue(ASNBinTreeNode* pNode, const Uchar* const raw, Uint rawlen)
{
	if (NULL==pNode) {
		return -1;
	}
	if (pNode->construct) {
		return -1;
	}
	if (pNode->value==raw) {	// Ignore the selfish copy
		return 0;
	}
	if (pNode->value) {
		delete []pNode->value;
		pNode->length = 0;
	}
	pNode->value = new Uchar[rawlen];
	memcpy(pNode->value, raw, rawlen);
	pNode->length = rawlen;
	return 0;
}

/*
* Function:
*	Get direct subtree of specific tag from parent.
* Parameters:
*	pParent - parent node.
*	tag - child tag.
*/
ASNBinTreeNode* asn_get_Child(ASNBinTreeNode* pParent, Uint tag)
{
	for (ASNBinTreeNode* pChild = pParent->children; pChild; pChild = pChild->brothers) {
		if (pChild->tag==tag) {
			return pChild;
		}
	}
	return NULL;
}

/*
* Function:
*	Delete direct subtree of specific tag from parent.
* Parameters:
*	pParent - parent node.
*	tag - child tag.
*/
void asn_delete_Child(ASNBinTreeNode* pParent, Uint tag)
{
	ASNBinTreeNode* pPrev = NULL;
	ASNBinTreeNode* pChild = NULL;
	for (pChild = pParent->children; pChild; pChild = pChild->brothers) {
		if (pChild->tag==tag) {
			break;
		}
		pPrev = pChild;
	}
	if (pChild) {
		if (pPrev) {
			pPrev->brothers = pChild->brothers;
		} else {
			pParent->children = pChild->brothers;
		}
		pChild->brothers = NULL;
		asn_free_TreeNode(pChild);
	}
}

/*
* Function:
*	Delete direct subtree of specific tag from parent.
* Parameters:
*	pParent - parent node.
*	tag - child tag.
*/
ASNBinTreeNode* asn_insert_Child(ASNBinTreeNode* pParent, Uint tag, const ASNBinTreeNode* pChild, TreeNodeAlloc& alloc)
{
	ASNBinTreeNode* pNode = asn_dup_SubTree(pChild, alloc);
	if (!pNode) {
		return NULL;
	}
	pNode->tag = tag;
	if (asn_add_SubTree(pParent, pNode)!=0) {
		asn_free_TreeNode(pNode);
		return NULL;
	}
	return pNode;
}

/*
* Function:
*	Show type name.
*		'G' - Generic
*		'A' - Application
*		'C' - Context
*		'P' - Private
*		'E' - Error (Unkown type)
*/
int asn_print_Type(FILE* fp, Uchar type)
{
	switch (type) {
	case TAGTYPE_GENERIC:
		//fprintf(fp, "Generic");
		fprintf(fp, "G");
		break;
	case TAGTYPE_APPLICATION:
		//fprintf(fp, "Application");
		fprintf(fp, "A");
		break;
	case TAGTYPE_CONTEXT:
		//fprintf(fp, "Context");
		fprintf(fp, "C");
		break;
	case TAGTYPE_PRIVATE:
		//fprintf(fp, "Private");
		fprintf(fp, "P");
		break;
	default:
		//fprintf(fp, "Unkown type!");
		fprintf(fp, "E");
		break;
	}
	return 0;
}

/*
* Function:
*	Parse the object lenght field.
*/
int asn_parse_Length(const Uchar*& buff, const Uchar* const end, Uint& length)
{
	if (buff>=end) {
		return -1;
	}
	if (buff[0] & MULTILEN_MASK) {
		Uchar size = (buff[0] & LENGTHSIZE_MASK);
		buff++;
		if (buff+size>=end) {
			return -1;
		}
		unsigned short hv = 0;
		unsigned long lv = 0;
		switch (size) {
		case 1:
			length = buff[0];
			break;

		case 2:
			memcpy(&hv, buff, sizeof(hv));
			length = ntohs(hv);
			break;

		case 3:
			memcpy(((char*)&lv)+1, buff, sizeof(lv)-1);
			length = ntohl(lv);
			break;

		case 4:
			memcpy(&lv, buff, sizeof(lv));
			length = ntohl(lv);
			break;

		default:
			return -1;
		}
		buff+=size;
	} else {
		length = (buff[0] & LENGTHSIZE_MASK);
		buff++;
	}
	return 0;
}

/*
* Function:
*	Build the object lenght field.
*/
int asn_build_Length(Uchar*& buff, const Uchar* const end, Uint length)
{
	if (buff>=end) {
		return -1;
	}
	if (length<=0x7f) {		// <128
		*buff = length;
		buff++;
	} else if (length<=0xff) {	// <256
		*buff = 1;	// 1 byte
		*buff |= MULTILEN_MASK;
		buff++;

		if (buff>=end) {
			return -1;
		}

		*buff = length;
		buff++;
	} else if (length<=0xffff) {
		*buff = 2;	// 2 bytes
		*buff |= MULTILEN_MASK;
		buff++;

		if (buff+2>=end) {
			return -1;
		}

		unsigned short hv = length;
		hv = htons(hv);
		memcpy(buff, &hv, 2);
		buff += 2;
	} else if (length<=0xffffff) {
		*buff = 3;
		*buff |= MULTILEN_MASK;
		buff++;

		if (buff+3>=end) {
			return -1;
		}

		unsigned long lv = length;
		lv = htonl(lv);
		memcpy(buff, ((char*)&lv)+1, 3);
		buff += 3;
	} else {
		*buff = 4;
		*buff |= MULTILEN_MASK;
		buff++;

		if (buff+4>=end) {
			return -1;
		}

		unsigned long lv = length;
		lv = htonl(lv);
		memcpy(buff, &lv, 4);
		buff += 4;
	}
	return 0;
}

/*
* Function:
*	Add a subtree to a parent. If already exist same child tag,
*	add as sequence-of, or else add as child.
* Notes:
*	The pNode should be allocated by outside!
*/
int asn_add_SubTree(ASNBinTreeNode* pParent, ASNBinTreeNode* pNode)
{
	if (!pParent || !pNode) {
		return -1;
	}

	ASNBinTreeNode* pChild = pParent->children;
	if (!pChild) {
		pParent->children = pNode;	// Append as the first chiled
	} else {
		while (pChild->brothers) {
			if (pChild->tag==pNode->tag) {
				break;
			}
			pChild = pChild->brothers;
		}
		if (pChild->tag==pNode->tag) {
			return asn_add_SeqOf(pChild, pNode);	// Append as the sequence of
		} else {
			pChild->brothers = pNode;	// Append as the last child
		}
	}
	return 0;
}

/*
* Function:
*	Append a 'SEQUENCE OF' node to the squence of list
* Notes:
*	The pNode should be allocated by outside!
*/
int asn_add_SeqOf(ASNBinTreeNode* pHead, ASNBinTreeNode* pNode)
{
	if (!pHead || !pNode) {
		return -1;
	}

	if (pHead->sequenceof) {
		ASNBinTreeNode* pTail = pHead->sequenceof;
		while (pTail->sequenceof) {
			pTail = pTail->sequenceof;
		}
		pTail->sequenceof = pNode;
	} else {
		pHead->sequenceof = pNode;
	}
	return 0;
}
