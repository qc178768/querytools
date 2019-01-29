/*
 * Copyright (C) 1997-2000 by Objective Systems, Inc.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Objective Systems, Inc.
 *
 * PROPRIETARY NOTICE
 *
 * This software is an unpublished work subject to a confidentiality agreement
 * and is protected by copyright and trade secret law.  Unauthorized copying,
 * redistribution or other use of this work is prohibited.
 *
 * The above notice of copyright on this source code product does not indicate
 * any actual or intended publication of such source code.
 *
 *****************************************************************************/

/* asn1type.h - ASN.1 runtime constants and data structure definitions */

#ifndef _ASN1TYPE_H_
#define _ASN1TYPE_H_

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#ifndef FALSE
#define FALSE		0
#define TRUE		!FALSE
#endif

#define MINMSGLEN       8	/* minimum message length               */

/* Error Code Constants */

#define ASN_OK		 0	/* normal completion status		*/
#define ASN_I_EXTEN      1      /* value falls within extension range   */
#define ASN_OK_FRAG      2      /* message fragment detected            */
#define ASN_E_BUFOVFLW	-1	/* encode buffer overflow		*/
#define ASN_E_ENDOFBUF	-2	/* unexpected end of buffer on decode	*/
#define ASN_E_IDNOTFOU	-3	/* identifer not found			*/
#define ASN_E_INVOBJID	-4	/* invalid object identifier		*/
#define ASN_E_INVLEN	-5	/* invalid field length			*/
#define ASN_E_INVENUM	-6	/* enumerated value not in defined set	*/
#define ASN_E_SETDUPL	-7	/* duplicate element in set		*/
#define ASN_E_SETMISRQ  -8	/* missing required element in set      */
#define ASN_E_NOTINSET  -9	/* element not part of set              */
#define ASN_E_SEQOVFLW	-10	/* sequence of field overflow           */
#define ASN_E_INVOPT    -11	/* invalid option encountered in choice */
#define ASN_E_NOMEM     -12	/* no dynamic memory available          */
#define ASN_E_INVSTYP   -13	/* invalid string type                  */
#define ASN_E_INVHEXS   -14	/* invalid hex string                   */
#define ASN_E_INVBINS   -15     /* invalid binary string                */
#define ASN_E_INVREAL	-16	/* invalid real value			*/
#define ASN_E_STROVFLW	-17	/* octet or bit string field overflow   */
#define ASN_E_BADVALUE  -18     /* invalid value specification          */
#define ASN_E_UNDEFVAL  -19     /* no def found for ref'd defined value */
#define ASN_E_UNDEFTYP  -20     /* no def found for ref'd defined type  */
#define ASN_E_BADTAG    -21     /* invalid tag value			*/
#define ASN_E_TOODEEP   -22	/* nesting level is too deep		*/
#define ASN_E_CONSVIO   -23     /* value constraint violation           */
#define ASN_E_RANGERR   -24     /* invalid range (lower > upper)        */
#define ASN_E_ENDOFFILE -25     /* end of file on file decode           */
#define ASN_E_NOTSUPP   -99     /* non-supported ASN construct          */
#define ASN_K_INDEFLEN	-9999	/* indefinite length message indicator	*/

/* Tagging Value and Mask Constants */

/* tag class value constants */

#define TV_UNIV         0	/* universal                    */
#define TV_APPL         1	/* application-wide             */
#define TV_CTXT         2	/* context-specific             */
#define TV_PRIV         3	/* private-use                  */

/* tag form value constants */

#define TV_PRIM		0	/* primitive                    */
#define TV_CONS     	1	/* constructor                  */

/* tag mask values - these can be logically or'd together to 	*/
/* form tags compatible with the ASNTAG structure..		*/

#define TM_UNIV		0x0000	/* universal class		*/
#define TM_APPL		0x4000	/* application-wide class	*/
#define TM_CTXT		0x8000	/* context-specific class	*/
#define TM_PRIV		0xC000	/* private-use class		*/

#define TM_PRIM		0x0000	/* primitive form		*/
#define TM_CONS		0x2000  /* constructor form		*/
                                                                
#define TM_CLASS	0xC0	/* class mask			*/
#define TM_FORM		0x20	/* form mask			*/
#define TM_CLASS_FORM	0xE0	/* class/form mask		*/
#define TM_B_IDCODE	0x1F	/* id code mask	(byte)		*/
#define TM_W_IDCODE	0x1FFF	/* id code mask	(word)		*/

#define ASN_K_BADTAG    0xFFFF  /* invalid tag code             */
#define ASN_K_NOTAG     0xFFFF  /* no tag input parameter       */

/* universal built-in type ID code value constants */

#define ASN_ID_EOC      0       /* end of contents              */
#define ASN_ID_BOOL     1       /* boolean                      */
#define ASN_ID_INT      2       /* integer                      */
#define ASN_ID_BITSTR   3       /* bit string                   */
#define ASN_ID_OCTSTR   4       /* byte (octet) string          */
#define ASN_ID_NULL     5       /* null                         */
#define ASN_ID_OBJID    6	/* object ID                    */
#define ASN_ID_OBJDSC   7       /* object descriptor            */
#define ASN_ID_EXTERN   8	/* external type                */
#define ASN_ID_REAL	9	/* real                         */
#define ASN_ID_ENUM	10	/* enumerated value		*/
#define ASN_ID_SEQ      16	/* sequence, sequence of        */
#define ASN_ID_SET      17      /* set, set of                  */

/* Restricted character string type ID's */

#define ASN_ID_NumericString    18
#define ASN_ID_PrintableString  19
#define ASN_ID_TeletexString    20
#define ASN_ID_T61String        ASN_ID_TeletexString
#define ASN_ID_VideotexString   21
#define ASN_ID_IA5String        22
#define ASN_ID_UTCTime          23
#define ASN_ID_GeneralTime      24
#define ASN_ID_GraphicString    25
#define ASN_ID_VisibleString    26
#define ASN_ID_GeneralString    27
#define ASN_ID_UniversalString  28
#define ASN_ID_BMPString        30

/* flag mask values */

#define XM_SEEK		0x01	/* seek match until found or end-of-buf */
#define XM_ADVANCE	0x02	/* advance pointer to contents on match	*/
#define XM_DYNAMIC	0x04	/* alloc dyn mem for decoded variable   */
#define XM_SKIP		0x08	/* skip to next field after parsing tag */

/* Sizing Constants */

#define ASN_K_MAXDEPTH	32	/* maximum nesting depth for messages	*/
#define ASN_K_MAXSUBIDS	128	/* maximum sub-id's in an object ID	*/
#define ASN_K_MAXENUM	100	/* maximum enum values in an enum type	*/
#define ASN_K_MAXERRP	5	/* maximum error parameters		*/
#define ASN_K_MAXERRSTK 8       /* maximum levels on error ctxt stack   */
#define	ASN_K_ENCBUFSIZ	16*1024	/* dynamic encode buffer extent size	*/
#define ASN_K_MAXMSGLEN	64*1024	/* max encoded message length		*/

/* Constants for encoding/decoding real values */

#define ASN1_K_PLUS_INFINITY	0x40
#define ASN1_K_MINUS_INFINITY	0x41

#define REAL_BINARY 		0x80
#define REAL_SIGN   		0x40
#define REAL_EXPLEN_MASK 	0x03
#define REAL_EXPLEN_1 		0x00
#define REAL_EXPLEN_2 		0x01
#define REAL_EXPLEN_3 		0x02
#define REAL_EXPLEN_LONG 	0x03
#define REAL_FACTOR_MASK 	0x0c
#define REAL_BASE_MASK 		0x30
#define REAL_BASE_2 		0x00
#define REAL_BASE_8 		0x10
#define REAL_BASE_16 		0x20

/* ASN.1 Primitive Type Definitions */

typedef unsigned char	ASN1OCTET;
typedef ASN1OCTET	ASN1BOOL;
typedef int		ASN1INT;
typedef unsigned int    ASN1UINT;
typedef ASN1INT		ASN1ENUM;
typedef double		ASN1REAL;
typedef short           ASN1SINT;
typedef unsigned short	ASN1USINT;
typedef ASN1USINT	ASN1TAG;
typedef ASN1USINT	ASN116BITCHAR;
typedef void*		ASN1ANY;

#define ASN1UINT_MAX    4294967295U
#define ASN1INT_MAX     2147483647L
#define ASN1INT_MIN    (-ASN1INT_MAX-1)

typedef enum { ASN1IMPL, ASN1EXPL } ASN1TagType;
typedef	enum { ASN1HEX, ASN1BIN, ASN1CHR } ASN1StrType;
typedef enum { ASN1ENCODE, ASN1DECODE } ASN1ActionType;

#ifdef __cplusplus
typedef const char* ASN1ConstCharPtr;
typedef const ASN1OCTET* ASN1ConstOctetPtr;
#else
typedef char* ASN1ConstCharPtr;
typedef ASN1OCTET* ASN1ConstOctetPtr;
#endif

typedef struct { 	/* object identifier */
   int		numids;
   unsigned	subid[ASN_K_MAXSUBIDS];
} ASN1OBJID;

typedef union {		/* double floating point (encode/decode real) */
   ASN1OCTET	byte[10];
   ASN1USINT	word[4];
   double	value;
} ASN1_DFLOAT;

typedef struct {	/* generic octet string structure */
   ASN1UINT	numocts;
   ASN1OCTET	data[1];
} ASN1OctStr;

typedef struct {	/* generic octet string structure (dynamic) */
   ASN1UINT	numocts;
   ASN1ConstOctetPtr data;
} ASN1DynOctStr;

typedef struct {	/* generic bit string structure (dynamic) */
   ASN1UINT	numbits;
   ASN1ConstOctetPtr data;
} ASN1DynBitStr;

typedef struct {	/* generic sequence of structure */
   ASN1UINT	n;
   void*	elem;
} ASN1SeqOf;

typedef struct _ASN1ListElem {  /* generic linked list structure */
   struct _ASN1ListElem* next;
   void*        data_p;
} ASN1ListElem;

typedef ASN1DynOctStr ASN1OpenType;

/* ASN.1 useful type definitions */

typedef struct {
   ASN1UINT       nchars;
   ASN116BITCHAR* data;
} Asn116BitCharString;

typedef ASN1ConstCharPtr   ASN1GeneralizedTime;
typedef ASN1ConstCharPtr   ASN1GeneralString;
typedef ASN1ConstCharPtr   ASN1GraphicString;
typedef ASN1ConstCharPtr   ASN1IA5String;
typedef ASN1ConstCharPtr   ASN1ISO646String;
typedef ASN1ConstCharPtr   ASN1NumericString;
typedef ASN1ConstCharPtr   ASN1ObjectDescriptor;
typedef ASN1ConstCharPtr   ASN1PrintableString;
typedef ASN1ConstCharPtr   ASN1TeletexString;
typedef ASN1ConstCharPtr   ASN1T61String;
typedef ASN1ConstCharPtr   ASN1UniversalString;
typedef ASN1ConstCharPtr   ASN1UTCTime;
typedef ASN1ConstCharPtr   ASN1UTF8String;
typedef ASN1ConstCharPtr   ASN1VideotexString;
typedef ASN1ConstCharPtr   ASN1VisibleString;

typedef Asn116BitCharString ASN1BMPString;

/* Singly-linked list */

typedef struct _Asn1RTSListNode {
   void* data;
   struct _Asn1RTSListNode* next;
} Asn1RTSListNode;

typedef struct _Asn1RTSList {
   int count;
   Asn1RTSListNode* head;
   Asn1RTSListNode* tail;
} Asn1RTSList;

/* Stack */

typedef struct _Asn1RTStack {
   int level;
   void* data[ASN_K_MAXDEPTH];
} Asn1RTStack;

/* ASN.1 encode/decode buffer info structure */

typedef struct {
   ASN1OCTET*   data;           /* pointer to start of data buffer      */
   ASN1UINT     byteIndex;      /* byte index                           */
   ASN1UINT     size;           /* current buffer size                  */
   ASN1SINT     bitOffset;      /* current bit offset (8 - 1)           */
   ASN1SINT	ilcnt;		/* indefinite length count	        */
   ASN1BOOL     dynamic;        /* is buffer dynamic?                   */
   ASN1BOOL     aligned;        /* is buffer byte aligned?              */
} ASN1BUFFER;

/* This structure is used to save the current state of the buffer */

typedef struct {
   ASN1UINT     byteIndex;      /* byte index                           */
   ASN1SINT     bitOffset;      /* current bit offset (8 - 1)           */
   ASN1SINT	ilcnt;		/* indefinite length count	        */
} ASN1BUFSAVE;

/* ASN.1 context control block - this structure is used to keep track	*/
/* of pointer and length context values when decoding sets or sequence	*/
/* of constructs.							*/

#define ASN1_K_CCBMaskSize      32
#define ASN1_K_NumBitsPerMask   16
#define ASN1_K_MaxSetElements   (ASN1_K_CCBMaskSize*ASN1_K_NumBitsPerMask)

typedef struct {		/* context control block	*/
   ASN1OCTET*	ptr;		/* constructor pointer		*/
   int		len;		/* constructor length		*/
   ASN1TagType  tagging;        /* implicit or explicit         */
   unsigned short mask[ASN1_K_CCBMaskSize];  /* set mask value */
} ASN1CCB;

/* ASN.1 run-time error info structures */

typedef struct {
   ASN1ConstCharPtr module;
   int		lineno;
} ASN1ErrLocn;

typedef struct {
   ASN1ErrLocn  stack[ASN_K_MAXERRSTK];
   int          stkx;
   int		status;
   int		parmcnt;
   ASN1ConstCharPtr parms[ASN_K_MAXERRP];
} ASN1ErrInfo;

/* ASN.1 memory allocation structures */

#define	XM_K_MEMBLKSIZ	((4*1024) - (sizeof(long) + sizeof(void*)))

typedef struct MemBlk {
   struct MemBlk* next_p;
   long		free_x;
   char		data[XM_K_MEMBLKSIZ];
} ASN1MemBlk;

/* ASN.1 size constraint structure */

typedef struct _Asn1SizeCnst {
   ASN1BOOL     extended;
   ASN1UINT     lower;
   ASN1UINT     upper;
   struct _Asn1SizeCnst* next;
} Asn1SizeCnst;

/* ASN.1 encode/decode context block structure */

typedef struct {		/* ASN.1 context block                  */
   ASN1BUFFER   buffer;         /* data buffer                          */
   ASN1BUFSAVE  savedInfo;      /* saved buffer info                    */
   ASN1ErrInfo	errInfo;	/* run-time error info                  */
   ASN1MemBlk*	memBlk_p;	/* memory block list                    */
   ASN1USINT    initCode;       /* code word to indicate init           */
   ASN1USINT    flags;          /* flag bits                            */
   Asn1RTSList  fieldList;      /* PER field list                       */
   Asn1SizeCnst* pSizeConstraint;  /* Size constraint list              */
   ASN1ConstCharPtr pCharSet;   /* String of permitted characters       */
   Asn1RTStack  nameStack;      /* Element name stack                   */
} ASN1CTXT;

#define ASN1CTXTINIT    0x0a0a
#define ASN1DYNCTXT     0x8000
#define ASN1INDEFLEN    0x4000
#define ASN1TRACE       0x2000
#define ASN1LASTEOC     0x1000

/* ASN.1 dump utility callback function definition */

typedef int (*ASN1DumpCbFunc) (ASN1ConstCharPtr text_p, void* cbArg_p);

/* ASNLIB function macros and prototypes */

#ifndef MAX
#define MAX(a,b)	(a>b)?a:b
#endif

#ifndef MIN
#define MIN(a,b)	(a<b)?a:b
#endif

#define ALLOC_ASN1ARRAY(pctxt,pseqof,type) \
pseqof->elem = (type*) rtMemAlloc (&pctxt->memBlk_p, sizeof(type)*pseqof->n); \
if (pseqof->elem == 0) return LOG_ASN1ERR (pctxt, ASN_E_NOMEM)

#define ALLOC_ASN1ELEM(pctxt,type) \
(type*) rtMemAlloc (&pctxt->memBlk_p, sizeof(type))

#define ASN1MALLOC(pctxt,nbytes) rtMemAlloc(&(pctxt)->memBlk_p,nbytes)
#define ASN1MEMFREE(pctxt)      rtMemFree(pctxt->memBlk_p)

#define ASN1BUFCUR(cp)          cp->buffer.data[cp->buffer.byteIndex]
#define ASN1BUFPTR(cp)          &cp->buffer.data[cp->buffer.byteIndex]
#define ASN1BUF_INDEFLEN(cp)    ((cp->flags&ASN1INDEFLEN)!=0)
#define ASN1BUF_PTR(cp)         ASN1BUFPTR(cp)
#define ASN1BUF_SAVE(cp)	xu_SaveBufferState(cp,0)
#define ASN1BUF_RESTORE(cp)	xu_RestoreBufferState(cp,0)

#define ASN1_PRINT_OCTSTR(os) \
printf ("%-*.*s", os.numocts, os.numocts, os.data)

#define LOG_ASN1ERR(ctxt,stat) \
rtErrSetData(&ctxt->errInfo,stat,__FILE__,__LINE__)

#define XU_DUMP(msg)		xu_dump(msg,0,0)

#define HEXCHARTONIBBLE(ch,b) \
if (ch >= '0' && ch <= '9') b = (unsigned char)(ch - '0'); \
else if (ch >= 'a' && ch <= 'f') b = (unsigned char)((ch - 'a') + 10); \
else if (ch >= 'A' && ch <= 'F') b = (unsigned char)((ch - 'A') + 10); \
else b = 0xFF;

#define NIBBLETOHEXCHAR(b,ch) \
if (b >= 0 && b <= 9) ch = (char)b + '0'; \
else if (b >= 0x0a && b <= 0x0f) ch = (char)((b - 10)+ 'a'); \
else ch = '?'

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ASN1DLL
#define EXTERN __declspec(dllexport)
#else
#define EXTERN
#endif

/* decode functions */

EXTERN int xd_tag (ASN1CTXT* ctxt_p, ASN1TAG *tag_p);

EXTERN int xd_tag_len (ASN1CTXT *ctxt_p, ASN1TAG *tag_p, 
                       int *len_p, ASN1OCTET flags);

EXTERN int xd_match (ASN1CTXT *ctxt_p, ASN1TAG tag, 
                     int *len_p, ASN1OCTET flags);

EXTERN int xd_boolean (ASN1CTXT *ctxt_p, ASN1BOOL *object_p, 
                       ASN1TagType tagging, int length);

EXTERN int xd_integer (ASN1CTXT *ctxt_p, ASN1INT *object_p, 
                       ASN1TagType tagging, int length);

EXTERN int xd_unsigned (ASN1CTXT *ctxt_p, ASN1UINT *object_p, 
                        ASN1TagType tagging, int length);

EXTERN int xd_bigint (ASN1CTXT *ctxt_p, ASN1ConstCharPtr* object_p, 
                      ASN1TagType tagging, int length);

EXTERN int xd_bitstr_s (ASN1CTXT *ctxt_p, ASN1ConstOctetPtr object_p, 
                        ASN1UINT* numbits_p, ASN1TagType tagging, int length);

EXTERN int xd_bitstr (ASN1CTXT *ctxt_p, ASN1ConstOctetPtr* object_p2, 
                      ASN1UINT* numbits_p, ASN1TagType tagging, int length);

EXTERN int xd_octstr_s (ASN1CTXT *ctxt_p, ASN1ConstOctetPtr object_p, 
                        ASN1UINT* numocts_p, ASN1TagType tagging, int length);

EXTERN int xd_octstr (ASN1CTXT *ctxt_p, ASN1ConstOctetPtr* object_p2, 
                      ASN1UINT* numocts_p, ASN1TagType tagging, int length);

EXTERN int xd_charstr (ASN1CTXT* ctxt_p, ASN1ConstCharPtr* object_p, 
                       ASN1TagType tagging, ASN1TAG tag, int length);

EXTERN int xd_16BitCharStr (ASN1CTXT* ctxt_p, Asn116BitCharString* object_p,
                            ASN1TagType tagging, ASN1TAG tag, int length);

EXTERN int xd_null (ASN1CTXT *ctxt_p, ASN1TagType tagging);

EXTERN int xd_objid (ASN1CTXT *ctxt_p, ASN1OBJID *object_p, 
                     ASN1TagType tagging, int length);

EXTERN int xd_real (ASN1CTXT *ctxt_p, ASN1REAL *object_p, 
                    ASN1TagType tagging, int length);

EXTERN int xd_enum (ASN1CTXT *ctxt_p, ASN1ENUM *object_p, 
                    ASN1TagType tagging, int length);

EXTERN int xd_OpenType (ASN1CTXT *ctxt_p, ASN1OpenType *object_p);

EXTERN int xd_OpenTypeExt (ASN1CTXT* ctxt_p, ASN1CCB* ccb_p, 
                           ASN1TAG tag, ASN1OpenType *object_p);

EXTERN int xd_setp (ASN1CTXT *ctxt_p, ASN1ConstOctetPtr msg_p, 
                    int msglen, ASN1TAG *tag_p, int *len_p);

EXTERN int xd_indeflen (ASN1OCTET* msg_p);
EXTERN int xd_memcpy (ASN1CTXT* ctxt_p, ASN1OCTET *object_p, int length) ;
EXTERN int xd_len (ASN1CTXT *ctxt_p, int *len_p);
EXTERN int xd_chkend (ASN1CTXT *ctxt_p, ASN1CCB* ccb_p);
EXTERN int xd_count (ASN1CTXT *ctxt_p, int length, int *count_p);
EXTERN int xd_NextElement (ASN1CTXT* ctxt_p);

/* file decode functions */

EXTERN int xdf_tag (FILE* fp, ASN1TAG* ptag, ASN1OCTET* buffer, int* pbufidx);
EXTERN int xdf_len (FILE* fp, ASN1INT* plen, ASN1OCTET* buffer, int* pbufidx);

EXTERN int xdf_TagAndLen (FILE* fp, ASN1TAG* ptag, ASN1INT* plen, 
                          ASN1OCTET* buffer, int* pbufidx);

EXTERN int xdf_ReadPastEOC (FILE* fp, ASN1OCTET* buffer, 
                            int bufsiz, int* pbufidx);

EXTERN int xdf_ReadContents (FILE* fp, int len, ASN1OCTET* buffer,
                             int bufsiz, int* pbufidx);

/* encode functions */

EXTERN int xe_tag_len (ASN1CTXT *ctxt_p, ASN1TAG tag, int length) ;

EXTERN int xe_boolean (ASN1CTXT* ctxt_p, ASN1BOOL *object_p, 
                       ASN1TagType tagging);

EXTERN int xe_integer (ASN1CTXT* ctxt_p, int *object_p, ASN1TagType tagging);

EXTERN int xe_unsigned (ASN1CTXT* ctxt_p, ASN1UINT *object_p, 
                        ASN1TagType tagging);

EXTERN int xe_bigint (ASN1CTXT* ctxt_p, ASN1ConstCharPtr object_p, 
                      ASN1TagType tagging);

EXTERN int xe_bitstr (ASN1CTXT* ctxt_p, ASN1ConstOctetPtr object_p, 
                      ASN1UINT numbits, ASN1TagType tagging);

EXTERN int xe_octstr (ASN1CTXT* ctxt_p, ASN1ConstOctetPtr object_p, 
                      ASN1UINT numocts, ASN1TagType tagging);

EXTERN int xe_charstr (ASN1CTXT* ctxt_p, ASN1ConstCharPtr object_p, 
                       ASN1TagType tagging, ASN1TAG tag);

EXTERN int xe_16BitCharStr (ASN1CTXT* ctxt_p, Asn116BitCharString* object_p, 
                            ASN1TagType tagging, ASN1TAG tag);

EXTERN int xe_null (ASN1CTXT* ctxt_p, ASN1TagType tagging);
EXTERN int xe_objid (ASN1CTXT* ctxt_p, ASN1OBJID *object_p, 
                     ASN1TagType tagging);
EXTERN int xe_enum (ASN1CTXT* ctxt_p, ASN1ENUM *object_p, ASN1TagType tagging);
EXTERN int xe_real (ASN1CTXT* ctxt_p, ASN1REAL *object_p, ASN1TagType tagging);
EXTERN int xe_OpenType (ASN1CTXT* ctxt_p, ASN1OpenType* object_p);
EXTERN void xe_setp (ASN1CTXT* ctxt_p, ASN1OCTET *buf_p, int bufsiz);
EXTERN void xe_setbufp (ASN1CTXT* ctxt_p, ASN1OCTET *buf_p, int bufsiz);
EXTERN ASN1OCTET* xe_getp (ASN1CTXT* ctxt_p);
EXTERN void xe_free (ASN1CTXT* ctxt_p);
EXTERN int xe_expandBuffer (ASN1CTXT *ctxt_p, int length);
EXTERN int xe_memcpy (ASN1CTXT *ctxt_p, ASN1OCTET *object_p, int length) ;
EXTERN int xe_len (ASN1CTXT *ctxt_p, int length) ;
EXTERN int xe_derCanonicalSort (ASN1CTXT* ctxt_p, Asn1RTSList* pList);

/* utility functions */

EXTERN int xu_verify_len (ASN1OCTET *msg_p);
EXTERN void *xu_parse_mmbuf (
  ASN1OCTET **buf_p2, int *buflen_p, ASN1OCTET *start_p, int bufsiz);
EXTERN void *xu_malloc (ASN1CTXT* ctxt_p, int nbytes);
EXTERN void xu_alloc_array (
  ASN1CTXT* ctxt_p, ASN1SeqOf* seqOf_p, int recSize, int recCount);
EXTERN void xu_freeall (ASN1CTXT* ctxt_p);
EXTERN void xu_octscpy_s (
  ASN1UINT* nocts_p, ASN1OCTET *data_p, char *cstr, char zterm);
EXTERN void xu_octscpy_ss (ASN1OctStr *octStr_p, char *cstring, char zterm);
EXTERN void xu_octscpy_d (
  ASN1CTXT* ctxt_p, ASN1UINT* nocts_p, ASN1OCTET** data_p2, 
  char* cstring, char zterm);
EXTERN void xu_octscpy_ds (
  ASN1CTXT* ctxt_p, ASN1DynOctStr *octStr_p, char *cstring, char zterm);
EXTERN void xu_octmcpy_s (ASN1OctStr *octStr_p, void* data_p, int datalen);
EXTERN void xu_octmcpy_d (
  ASN1CTXT* ctxt_p, ASN1DynOctStr *octStr_p, void* data_p, int datalen);
EXTERN char* xu_fetchstr (int numocts, char *data);
EXTERN int xu_hexstrcpy (char *data, char *hstring);
EXTERN int xu_binstrcpy (char *data, char *bstring);
EXTERN int xu_dump (ASN1OCTET *msgptr, ASN1DumpCbFunc cb, void* cbArg_p);
EXTERN int xu_fdump (FILE *file_p, ASN1OCTET *msgptr);
EXTERN void xu_hex_dump (ASN1OCTET *data, int numocts, ASN1OCTET hdrflg);
EXTERN void xu_fmt_tag (
  ASN1TAG *tag_p, char *class_p, char *form_p, char *id_code);
EXTERN char* xu_fmt_tag2 (ASN1TAG* tag_p, char* bufp);
EXTERN char* xu_fmt_contents (ASN1CTXT* ctxt_p, int len, int *count);
EXTERN int xu_error (ASN1CTXT* ctxt_p, int status, char* module, int lineno);
EXTERN int xu_addErrParm (ASN1CTXT* ctxt_p, char* errprm_p);
EXTERN int xu_addIntErrParm (ASN1CTXT* ctxt_p, int errParm);
EXTERN int xu_addUnsignedErrParm (ASN1CTXT* ctxt_p, unsigned int errParm);
EXTERN int xu_addTagErrParm (ASN1CTXT* ctxt_p, ASN1TAG errParm);
EXTERN void xu_perror (ASN1CTXT* ctxt_p);
EXTERN void xu_log_error (ASN1CTXT* ctxt_p, ASN1DumpCbFunc cb, void* cbArg_p);
EXTERN char* xu_fmtErrMsg (ASN1CTXT* ctxt_p, char* bufp);
EXTERN ASN1REAL xu_GetPlusInfinity ();
EXTERN ASN1REAL xu_GetMinusInfinity ();
EXTERN int xu_fread (FILE* fp, ASN1OCTET* bufp, int bufsiz);
EXTERN ASN1ListElem* xu_list_append (
  ASN1CTXT* ctxt_p, ASN1ListElem* list_p, void* data_p);
EXTERN void xu_SaveBufferState (ASN1CTXT* pCtxt, ASN1BUFSAVE* pSavedInfo);
EXTERN void xu_RestoreBufferState (ASN1CTXT* pCtxt, ASN1BUFSAVE* pSavedInfo);

/* run-time error and diagnostic functions */

extern int g_debug;

EXTERN void  rtCopyContext (ASN1CTXT* pdest, ASN1CTXT* psrc);
EXTERN void  rtdiag (ASN1ConstCharPtr fmtspec, ...);
EXTERN int   rtErrAddIntParm (ASN1ErrInfo* pErrInfo, int errParm);
EXTERN int   rtErrAddStrParm (ASN1ErrInfo* pErrInfo, char* errprm_p);
EXTERN int   rtErrAddTagParm (ASN1ErrInfo* pErrInfo, ASN1TAG errParm);
EXTERN int   rtErrAddUIntParm (ASN1ErrInfo* pErrInfo, unsigned int errParm);
EXTERN void  rtErrFreeParms (ASN1ErrInfo* pErrInfo);
EXTERN char* rtErrFmtMsg (ASN1ErrInfo* pErrInfo, char* bufp);
EXTERN void  rtErrLogUsingCB (ASN1ErrInfo* pErrInfo, ASN1DumpCbFunc cb, void* cbArg);
EXTERN void  rtErrPrint (ASN1ErrInfo* pErrInfo);
EXTERN int   rtErrSetData (ASN1ErrInfo* pErrInfo, int status, ASN1ConstCharPtr module, int lno);
EXTERN void  rtFreeContext (ASN1CTXT* ctxt_p);
EXTERN void  rtHexDump (ASN1OCTET* data, ASN1UINT numocts);
EXTERN int   rtInitContextBuffer (ASN1CTXT* ctxt_p, ASN1ConstOctetPtr bufaddr, ASN1UINT bufsiz);
EXTERN int   rtInitContext (ASN1CTXT* ctxt_p, ASN1OCTET* bufaddr, ASN1UINT bufsiz);
EXTERN void* rtMemAlloc (ASN1MemBlk** ppMemBlk, int nbytes);
EXTERN void  rtMemFree (ASN1MemBlk* pMemBlk);
EXTERN ASN1CTXT* rtNewContext (ASN1OCTET* bufaddr, ASN1UINT bufsiz);
EXTERN void  rtSetDiag (int value);
EXTERN void  rtSetOID (ASN1OBJID* ptarget, ASN1OBJID* psource);
EXTERN void  rtSListInit (Asn1RTSList* pList);
EXTERN void  rtSListFree (Asn1RTSList* pList);
EXTERN Asn1RTSList* rtSListCreate ();
EXTERN Asn1RTSListNode* rtSListAppend (Asn1RTSList* pList, void* pData);
EXTERN Asn1RTStack* rtStackCreate ();
EXTERN void  rtStackInit (Asn1RTStack* pStack);
EXTERN void* rtStackPop (Asn1RTStack* pStack);
EXTERN int   rtStackPush (Asn1RTStack* pStack, void* pData);

/* run-time print functions */

EXTERN void rtPrintBoolean (ASN1ConstCharPtr name, ASN1BOOL value);
EXTERN void rtPrintInteger (ASN1ConstCharPtr name, ASN1INT value);
EXTERN void rtPrintUnsigned (ASN1ConstCharPtr name, ASN1UINT value);
EXTERN void rtPrintBitStr (ASN1ConstCharPtr name, ASN1UINT numbits, 
                           ASN1ConstOctetPtr data, ASN1ConstCharPtr conn);
EXTERN void rtPrintOctStr (ASN1ConstCharPtr name, ASN1UINT numocts, 
                           ASN1ConstOctetPtr data, ASN1ConstCharPtr conn);
EXTERN void rtPrintCharStr (ASN1ConstCharPtr name, ASN1ConstCharPtr cstring);
EXTERN void rtPrint16BitCharStr (ASN1ConstCharPtr name, 
                                 Asn116BitCharString* bstring,
                                 ASN1ConstCharPtr conn);
EXTERN void rtPrintReal (ASN1ConstCharPtr name, ASN1REAL value);
EXTERN void rtPrintOID (ASN1ConstCharPtr name, ASN1OBJID* pOID);
EXTERN void rtPrintOIDValue (ASN1OBJID* pOID);
EXTERN void rtPrintOpenType (ASN1ConstCharPtr name, ASN1UINT numocts, 
                             ASN1ConstOctetPtr data, ASN1ConstCharPtr conn);

EXTERN void rtEvalDateCheck ();

#ifdef __cplusplus
}
#endif

#endif
