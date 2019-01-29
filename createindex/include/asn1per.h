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

/* asn1per.h - ASN.1 runtime constants and data structures for PER */

#ifndef _ASN1PER_H_
#define _ASN1PER_H_

#include "asn1type.h"

/* The following constant is stored in an enumerated value if the       */
/* parsed value is not within the defined set of extended enum values   */
/* (note: this is only done in the case of PER because the actual value */
/* is not known - in BER, the actual value is stored)..                 */

#define ASN_K_EXTENUM   999

/* Structures to track encoded PER fields */

typedef struct PERField {
   char*        name;
   int          bitOffset;
   int          numbits;
} PERField;

typedef struct {
   unsigned char lb, lbm;
   char fmtBitBuffer[40], fmtHexBuffer[10], fmtAscBuffer[10];
   int  fmtBitCharIdx, fmtHexCharIdx, fmtAscCharIdx;
} BinDumpBuffer;

/* ASN.1 PER constrained string structures */

typedef struct {
   int          nchars;
   char         data[255];
} Asn1CharArray;

typedef struct {
   Asn1CharArray charSet;
   ASN1ConstCharPtr canonicalSet;
   int          canonicalSetSize;
   unsigned     canonicalSetBits;
   unsigned     charSetUnalignedBits;
   unsigned     charSetAlignedBits;
} Asn1CharSet;

typedef struct {
   Asn116BitCharString charSet;
   ASN1USINT    firstChar, lastChar;
   unsigned     unalignedBits;
   unsigned     alignedBits;
} Asn116BitCharSet;

/* Macro to create a character set */

#define PU_SETCHARSET(csetvar, canset, abits, ubits) \
csetvar.charSet.nchars = 0; \
csetvar.canonicalSet = canset; \
csetvar.canonicalSetSize = sizeof(canset)-1; \
csetvar.canonicalSetBits = pu_bitcnt(csetvar.canonicalSetSize); \
csetvar.charSetUnalignedBits = ubits; \
csetvar.charSetAlignedBits = abits;

/* Canonical character set definitions */

#define NUM_ABITS  4
#define NUM_UBITS  4
#define NUM_CANSET \
" 0123456789"

#define PRN_ABITS  8
#define PRN_UBITS  7
#define PRN_CANSET \
" '()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

#define VIS_ABITS  8
#define VIS_UBITS  7
#define VIS_CANSET \
" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"\
"^_`abcdefghijklmnopqrstuvwxyz{|}~"

#define T61_ABITS  8
#define T61_UBITS  7
#define T61_CANSET \
" !\"%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]"\
"_abcdefghijklmnopqrstuvwxyz"

#define IA5_ABITS  8
#define IA5_UBITS  7
#define IA5_CANSET \
"\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"\
"\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"\
" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"

#define GEN_ABITS  8
#define GEN_UBITS  7
#define GEN_CANSET \
"\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"\
"\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"\
" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"\
"`abcdefghijklmnopqrstuvwxyz{|}~\177\200\201\202\203\204\205\206\207"\
"\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"\
"\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"\
"\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"\
"\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"\
"\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"\
"\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357"\
"\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377"

#define BMP_ABITS  16
#define BMP_UBITS  16
#define BMP_FIRST  0
#define BMP_LAST   0xffff

/* Macros */

#ifdef _TRACE
#define PU_INSLENFLD(ctxt_p)       pu_insLenField(ctxt_p)
#define PU_NEWFIELD(ctxt_p,suffix) pu_newField(ctxt_p,suffix)
#define PU_PUSHNAME(ctxt_p,name)   pu_pushName(ctxt_p,name)
#define PU_POPNAME(ctxt_p)         pu_popName(ctxt_p)
#define PU_SETBITOFFSET(ctxt_p)    pu_setFldBitOffset(ctxt_p)
#define PU_SETBITCOUNT(ctxt_p)     pu_setFldBitCount(ctxt_p)
#define PU_PUSHELEMNAME(ctxt_p,idx) pu_pushElemName(ctxt_p,idx)
#else
#define PU_INSLENFLD(ctxt_p)
#define PU_NEWFIELD(ctxt_p,suffix)
#define PU_PUSHNAME(ctxt_p,name)
#define PU_POPNAME(ctxt_p)
#define PU_SETBITOFFSET(ctxt_p)
#define PU_SETBITCOUNT(ctxt_p)
#define PU_PUSHELEMNAME(ctxt_p,idx)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */

EXTERN int pd_bit (ASN1CTXT* ctxt_p, ASN1BOOL* pvalue);
EXTERN int pd_bits (ASN1CTXT* ctxt_p, ASN1UINT* pvalue, ASN1UINT nbits);

EXTERN int pd_BigInteger (ASN1CTXT *ctxt_p, char** ppvalue);

EXTERN int pd_BitString (
   ASN1CTXT* ctxt_p, ASN1UINT* numbits_p, ASN1ConstOctetPtr buffer,
   ASN1UINT bufsiz);

EXTERN int pd_BMPString (
   ASN1CTXT* ctxt_p, ASN1BMPString* pvalue, Asn116BitCharSet* permCharSet);

EXTERN int pd_byte_align (ASN1CTXT* ctxt_p);

EXTERN int pd_ChoiceOpenTypeExt (ASN1CTXT* ctxt_p, ASN1OpenType* pOpenType);

EXTERN int pd_ConsInteger (
   ASN1CTXT* ctxt_p, ASN1INT* pvalue, ASN1INT lower, ASN1INT upper);

EXTERN int pd_ConsUnsigned (
   ASN1CTXT* ctxt_p, ASN1UINT* pvalue, ASN1UINT lower, ASN1UINT upper);

EXTERN int pd_ConsWholeNumber (
   ASN1CTXT* ctxt_p, ASN1UINT* padjusted_value, ASN1UINT range_value);

EXTERN int pd_ConstrainedString (
   ASN1CTXT* ctxt_p, char** string, Asn1CharSet* pCharSet);

EXTERN int pd_16BitConstrainedString (
   ASN1CTXT* ctxt_p, Asn116BitCharString* pString, Asn116BitCharSet* pCharSet);

EXTERN int pd_DynBitString (ASN1CTXT* ctxt_p, ASN1DynBitStr* pBitStr);

EXTERN int pd_DynOctetString (ASN1CTXT* ctxt_p, ASN1DynOctStr* pOctStr);

EXTERN int pd_GeneralString (
   ASN1CTXT* ctxt_p, ASN1GeneralString* pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pd_GetComponentLength (ASN1CTXT* ctxt_p, ASN1UINT itemBits);

EXTERN int pd_IA5String (
   ASN1CTXT* ctxt_p, ASN1IA5String* pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pd_IncrBitIdx (ASN1CTXT* ctxt_p);

EXTERN int pd_Length (ASN1CTXT* ctxt_p, ASN1UINT* pvalue);

EXTERN int pd_moveBitCursor (ASN1CTXT* ctxt_p, int bitOffset);

EXTERN int pd_NumericString (
   ASN1CTXT* ctxt_p, ASN1NumericString* pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pd_ObjectIdentifier (ASN1CTXT* ctxt_p, ASN1OBJID* pvalue);

EXTERN int pd_OctetString (
   ASN1CTXT* ctxt_p, ASN1UINT* numocts_p, ASN1ConstOctetPtr buffer,
   ASN1UINT bufsiz);

EXTERN int pd_OpenType (ASN1CTXT* ctxt_p, ASN1OpenType* pOpenType);
EXTERN int pd_OpenTypeExt (ASN1CTXT* ctxt_p, ASN1OpenType* pOpenType);

EXTERN int pd_PrintableString (ASN1CTXT* ctxt_p, 
   ASN1PrintableString* pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pd_Real (ASN1CTXT* ctxt_p, ASN1REAL* pvalue);
EXTERN int pd_SmallNonNegWholeNumber (ASN1CTXT* ctxt_p, ASN1UINT* pvalue);

EXTERN int pd_T61String (
   ASN1CTXT* ctxt_p, ASN1T61String* pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pd_UnconsInteger (ASN1CTXT* ctxt_p, ASN1INT* pvalue);
EXTERN int pd_UnconsUnsigned (ASN1CTXT* ctxt_p, ASN1UINT* pvalue);

EXTERN int pd_VisibleString (
   ASN1CTXT* ctxt_p, ASN1VisibleString* pvalue, ASN1ConstCharPtr permCharSet);

/* Encode functions */

EXTERN int pe_16BitConstrainedString (
   ASN1CTXT* ctxt_p, Asn116BitCharString value, Asn116BitCharSet* pCharSet);

EXTERN int pe_2sCompBinInt (ASN1CTXT* ctxt_p, ASN1INT value);

EXTERN int pe_aligned_octets (
   ASN1CTXT* ctxt_p, ASN1OCTET* pvalue, ASN1UINT nocts);

EXTERN int pe_BigInteger (ASN1CTXT* ctxt_p, ASN1ConstCharPtr pvalue);

EXTERN int pe_bit (ASN1CTXT* ctxt_p, ASN1BOOL value);
EXTERN int pe_bits (ASN1CTXT* ctxt_p, ASN1UINT value, ASN1UINT nbits);

EXTERN int pe_BitString (
   ASN1CTXT* ctxt_p, ASN1UINT numocts, ASN1ConstOctetPtr data);

EXTERN int pe_BMPString (
   ASN1CTXT* ctxt_p, ASN1BMPString value, Asn116BitCharSet* permCharSet);

EXTERN int pe_byte_align (ASN1CTXT* ctxt_p);
EXTERN int pe_CheckBuffer (ASN1CTXT* ctxt_p, ASN1UINT nbytes);

EXTERN int pe_ConsInteger (
   ASN1CTXT* ctxt_p, ASN1INT value, ASN1INT lower, ASN1INT upper);

EXTERN int pe_ConstrainedString (
   ASN1CTXT* ctxt_p, ASN1ConstCharPtr string, Asn1CharSet* pCharSet );

EXTERN int pe_ConsUnsigned (
   ASN1CTXT* ctxt_p, ASN1UINT value, ASN1UINT lower, ASN1UINT upper);

EXTERN int pe_ConsWholeNumber (
   ASN1CTXT* ctxt_p, ASN1UINT adjusted_value, ASN1UINT range_value);

EXTERN int pe_ExpandBuffer (ASN1CTXT* ctxt_p, ASN1UINT nbytes);

EXTERN int pe_GeneralString (
   ASN1CTXT* ctxt_p, ASN1GeneralString value, ASN1ConstCharPtr permCharSet );

EXTERN ASN1UINT pe_GetIntLen (ASN1UINT value);
EXTERN int pe_GetMsgBitCnt (ASN1CTXT* ctxt_p);
EXTERN ASN1OCTET* pe_GetMsgPtr (ASN1CTXT* ctxt_p, int* pLength);

EXTERN int pe_IA5String (
   ASN1CTXT* ctxt_p, ASN1IA5String value, ASN1ConstCharPtr permCharSet );

EXTERN int pe_Length (ASN1CTXT* ctxt_p, ASN1UINT value);
EXTERN int pe_NonNegBinInt (ASN1CTXT* ctxt_p, ASN1UINT value);

EXTERN int pe_NumericString (
   ASN1CTXT* ctxt_p, ASN1NumericString value, ASN1ConstCharPtr permCharSet );

EXTERN int pe_ObjectIdentifier (ASN1CTXT* ctxt_p, ASN1OBJID* pvalue);
EXTERN int pe_octets (ASN1CTXT* ctxt_p, ASN1OCTET* pvalue, ASN1UINT nbits);

EXTERN int pe_OctetString (
   ASN1CTXT* ctxt_p, ASN1UINT numocts, ASN1ConstOctetPtr data);

EXTERN int pe_OpenType (ASN1CTXT* ctxt_p, ASN1OpenType* pOpenType);
EXTERN int pe_OpenTypeExt (ASN1CTXT* ctxt_p, ASN1OpenType* pOpenType);

EXTERN int pe_PrintableString (
   ASN1CTXT* ctxt_p, ASN1PrintableString value, ASN1ConstCharPtr permCharSet );

EXTERN int pe_Real (ASN1CTXT* ctxt_p, ASN1REAL value);
EXTERN int pe_SmallNonNegWholeNumber (ASN1CTXT* ctxt_p, ASN1UINT value);

EXTERN int pe_T61String (
   ASN1CTXT* ctxt_p, ASN1T61String pvalue, ASN1ConstCharPtr permCharSet);

EXTERN int pe_UnconsLength (ASN1CTXT* ctxt_p, ASN1UINT value);
EXTERN int pe_UnconsInteger (ASN1CTXT* ctxt_p, ASN1INT value);
EXTERN int pe_UnconsUnsigned (ASN1CTXT* ctxt_p, ASN1UINT value);

EXTERN int pe_VisibleString (
   ASN1CTXT* ctxt_p, ASN1VisibleString value, ASN1ConstCharPtr permCharSet );

/* Utility functions */

EXTERN int  pu_addSizeConstraint (ASN1CTXT* ctxt_p, Asn1SizeCnst* pSize);
EXTERN void pu_bindump (ASN1CTXT* ctxt_p, ASN1ConstCharPtr varname);
EXTERN ASN1UINT pu_bitcnt (ASN1UINT value);
EXTERN ASN1ConstCharPtr pu_BMPStringToCString (
   ASN1BMPString* pBMPString, ASN1ConstCharPtr cstring, ASN1UINT cstrsize);
EXTERN ASN1ConstCharPtr pu_BMPStringToNewCString (ASN1BMPString* pBMPString);
EXTERN ASN1BMPString* pu_CStringToBMPString (ASN1CTXT* ctxt_p,
   ASN1ConstCharPtr cstring, ASN1BMPString* pBMPString, Asn116BitCharSet* pCharSet);
EXTERN Asn1SizeCnst* pu_checkSize (
   Asn1SizeCnst* pSizeList, ASN1UINT value, ASN1BOOL* pExtendable);
EXTERN void pu_freeContext (ASN1CTXT* ctxt_p);
EXTERN int  pu_getBitOffset (ASN1CTXT* ctxt_p);
EXTERN int  pu_getMaskAndIndex (int bitOffset, unsigned char* pMask);
EXTERN int  pu_getMsgLen (ASN1CTXT* ctxt_p);
EXTERN void pu_hexdump (ASN1CTXT* ctxt_p);

EXTERN int pu_initContext (
   ASN1CTXT* ctxt_p, ASN1OCTET* bufaddr, ASN1UINT bufsiz, ASN1BOOL aligned);

EXTERN int pu_initContextBuffer (ASN1CTXT* pTarget, ASN1CTXT* pSource);

EXTERN void pu_dumpField (ASN1CTXT* ctxt_p, PERField* pField, 
   ASN1ConstCharPtr varname, int nextBitOffset, BinDumpBuffer* pbuf);

EXTERN ASN1ConstCharPtr pu_getFullName (ASN1CTXT* ctxt_p, ASN1ConstCharPtr suffix);
EXTERN Asn1SizeCnst* pu_getSizeConstraint (ASN1CTXT* ctxt_p, ASN1BOOL extbit);

EXTERN void pu_init16BitCharSet (Asn116BitCharSet* pCharSet, 
   ASN116BITCHAR first, ASN116BITCHAR last, ASN1UINT abits, ASN1UINT ubits);

EXTERN void pu_insLenField (ASN1CTXT* ctxt_p);

EXTERN ASN1BOOL pu_isExtendableSize (Asn1SizeCnst* pSizeList);

EXTERN ASN1BOOL pu_isFixedSize (Asn1SizeCnst* pSizeList);

EXTERN ASN1BOOL pu_isIn16BitCharSet (
   ASN116BITCHAR ch, Asn116BitCharSet* pCharSet);

EXTERN ASN1CTXT* pu_newContext (
   ASN1OCTET* bufaddr, ASN1UINT bufsiz, ASN1BOOL aligned);

EXTERN PERField* pu_newField (ASN1CTXT* ctxt_p, ASN1ConstCharPtr nameSuffix);
EXTERN void pu_popName (ASN1CTXT* ctxt_p);
EXTERN void pu_pushElemName (ASN1CTXT* ctxt_p, int index);
EXTERN void pu_pushName (ASN1CTXT* ctxt_p, ASN1ConstCharPtr name);
EXTERN void pu_setBitOffset (ASN1CTXT* ctxt_p, int bitOffset);
EXTERN void pu_setCharSet (Asn1CharSet* pCharSet, ASN1ConstCharPtr permSet);
EXTERN void pu_set16BitCharSet (
   ASN1CTXT* ctxt_p, Asn116BitCharSet* pCharSet, Asn116BitCharSet* pAlphabet);
EXTERN void pu_setFldBitCount (ASN1CTXT* ctxt_p);
EXTERN void pu_setFldBitOffset (ASN1CTXT* ctxt_p);
EXTERN void pu_setTrace (ASN1CTXT* pCtxt, ASN1BOOL value);

#ifdef __cplusplus
}
#endif

/* Macros */

#define pe_ISO646String      pe_IA5String
#define pe_GeneralizedTime   pe_IA5String
#define pe_TeletexString     pe_TeletexString
#define pe_UTCTime           pe_GeneralizedTime
#define pe_VideotexString    pe_GeneralString
#define pe_GraphicString     pe_GeneralString
#define pe_UniversalString   pe_GeneralString
#define pe_UTF8String        pe_UniversalString
#define pe_ObjectDescriptor  pe_GraphicString

#define pd_ISO646String      pd_IA5String
#define pd_GeneralizedTime   pd_IA5String
#define pd_TeletexString     pd_TeletexString
#define pd_UTCTime           pd_GeneralizedTime
#define pd_VideotexString    pd_GeneralString
#define pd_GraphicString     pd_GeneralString
#define pd_UniversalString   pd_GeneralString
#define pd_UTF8String        pd_UniversalString
#define pd_ObjectDescriptor  pd_GraphicString

#define pe_GetMsgLen         pu_getMsgLen

#endif
