// asntree.h
// Functions:
//		Use CASNTree class to parse BER encoded ASN.1 CDR to a
//		tree structure. And then you may operate on the object
//		and re-encode the object into BER encoded binary octet
//		array.
// History:
//		2001-02-16	Mike Liu	Create this file.
//------------------------------------------------------------------

#ifndef __asntree_h__
#define __asntree_h__

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
//#include <stddef.h>
#include <stdio.h>
#include "objalloc.h"
#include "string2.h"

// Tree Node type.
#define TAGTYPE_MASK			0xc0
#define TAGTYPE_GENERIC			0x00
#define TAGTYPE_APPLICATION		0x40
#define TAGTYPE_CONTEXT			0x80
#define TAGTYPE_PRIVATE			0xc0

// Multi-octet tag value?
#define TAGVALUE_MASK			0x1f

// Primary or Construct?
#define TAGPC_MASK				0x20
#define TAGPC_PRIMARY			0x00
#define TAGPC_CONSTRUCT			0x20

// Multi-octet length value?
#define MULTILEN_MASK			0x80
#define LENGTHSIZE_MASK			(~MULTILEN_MASK)

// Indent of one level.
#define INDENT					4

// Max OID length.
#define MAXNAMELEN				256

typedef unsigned char Uchar;
typedef unsigned long int Uint;

struct ASNBinTreeNode {
	// Base information
	Uchar				type;				// Node type.
	Uint				tag;				// Node tag.
	Uchar				construct;			// Is construct?

	// Valid only when this is a primary
	Uint				length;				// Size of raw value.
	Uchar*				value;				// Pointer to raw value.
	mutable string2		svalue;

	// Valid only when this is a CONSTRUCT
	ASNBinTreeNode*	children;			// Children list of this node.

	// Other linkage
	ASNBinTreeNode*	brothers;			// Brothers list of this node.
	ASNBinTreeNode*	sequenceof;		// Seqeuence-of list of this node.

	void* operator new (size_t sz, void* addr) {
		return mmalloc_alloc(sz, addr);
	}

	void operator delete(void* addr) {
		mmalloc_free(addr);
	}
};


typedef mmalloc<ASNBinTreeNode>	TreeNodeAlloc;

class CASNTree
{
private:
	ASNBinTreeNode*			m_proot;			// Root node (unaccessable)
	mutable TreeNodeAlloc	m_alloc;

private:
	int NodeOp(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode, bool bReplace = true);

public:
	// Constructors & destructor
	CASNTree();
	CASNTree(const CASNTree& other);
	CASNTree& operator =(const CASNTree& other);
	virtual ~CASNTree();

	// Object operators
	int Assign(const CASNTree& other);
	int Parse(const Uchar* buff, Uint size);
	int Parse(const Uchar* buff, const Uchar* const end);
	int Build(Uchar* buff, Uint& size);
	int Build(Uchar*& buff, const Uchar* const end);

	// Member access functions

	// Function:
	//	Get node of oid.
	const ASNBinTreeNode* GetNode(Uint* oid, Uint oidlen) const;

	// Function:
	//	If the same oid node exist, replace the node with pNode,
	//	else create new node with content of pNode.
	int SetNode(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode);

	// Function:
	// 	If the same oid node exist, add an element of sequence-of,
	//	else, create a child node same as pNode.
	int AddNode(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode);

	void DelNode(Uint* oid, Uint oidlen);

	// Function:
	//	Set a leaf node value.
	int SetNode(Uint* oid, Uint oidlen, const Uchar* raw, Uint rawlen);

	const ASNBinTreeNode* GetElem(Uint* oid, Uint oidlen, int index = 0) const;
	int AddElem(Uint* oid, Uint oidlen, const ASNBinTreeNode* pNode);
	int DelElem(Uint* oid, Uint oidlen, int index = -1);
	int GetElemCount(Uint* oid, Uint oidlen) const;

	const ASNBinTreeNode* GetTree() const;
	Uint GetSize() const;

	// Debug function
	void Print(FILE* fp = stdout, size_t index = 0) const;

	const ASNBinTreeNode* GetrootNode() const{ return m_proot;}
};




// Local assistant functions
ASNBinTreeNode* asn_alloc_TreeNode(TreeNodeAlloc& alloc);
void asn_free_TreeNode(ASNBinTreeNode* pNode);

// Top level tree operators.
int asn_parse_SubTree(ASNBinTreeNode* root, const Uchar*& buff, const Uchar* const end, TreeNodeAlloc& alloc, int level = 0);
int asn_build_SubTree(const ASNBinTreeNode* pNode, Uchar*& buff, const Uchar* const end, int level = 0);
void asn_print_SubTree(FILE* fp, const ASNBinTreeNode* pNode, size_t* name, size_t level = 0, Uint index = 0);
ASNBinTreeNode* asn_dup_SubTree(const ASNBinTreeNode* pNode, TreeNodeAlloc& alloc);

// Refresh length fields of all node of a parent node.
Uint asn_calculate_SubTreeLength(ASNBinTreeNode* pParent, int level = 0);


// Get/set tree node according to OID of the node.
const ASNBinTreeNode* asn_get_SubTree(const ASNBinTreeNode* pParent, Uint* oid, Uint oidlen);
ASNBinTreeNode* asn_get_gen_SubTree(ASNBinTreeNode* pParent, Uint* oid, Uint oidlen, TreeNodeAlloc& alloc, bool bConstruct = false, Uchar type = TAGTYPE_CONTEXT);

// Get direct child of a parent.
ASNBinTreeNode* asn_get_Child(ASNBinTreeNode* pParent, Uint tag);
void asn_delete_Child(ASNBinTreeNode* pParent, Uint tag);
ASNBinTreeNode* asn_insert_Child(ASNBinTreeNode* pParent, Uint tag, const ASNBinTreeNode* pChild, TreeNodeAlloc& alloc);

// Print textual type name.
int asn_print_Type(FILE* fp, Uchar type);

// Length field parse/build.
int asn_parse_Length(const Uchar*& buff, const Uchar* const end, Uint& length);
int asn_build_Length(Uchar*& buff, const Uchar* const end, Uint length);

// Insert sub-tree-node.
int asn_add_SubTree(ASNBinTreeNode* pParent, ASNBinTreeNode* pNode);
int asn_add_SeqOf(ASNBinTreeNode* pHead, ASNBinTreeNode* pNode);
int asn_fill_LeafValue(ASNBinTreeNode* pNode, const Uchar* const raw, Uint rawlen);

#endif


