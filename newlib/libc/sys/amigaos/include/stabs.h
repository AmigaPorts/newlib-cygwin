#ifndef _HEADERS_STABS_H
#define _HEADERS_STABS_H

/* The lists are sections, which the linker script collects and sorts by
   name: .list_* in the code, .dlist_* in the data. */

#define __PASTE(a,b) a##b

#define ADD2LIST(a,b,c) \
	__PASTE(ADD2LIST,c)(_##a,b)

#define ADD2LIST2(a,b,c) \
	__PASTE(ADD2LIST,c)(a,b)

/* add symbol a to list b */
#define ADD2LIST22(a,b) \
	__asm__(".section\t.list_" #b ",\"aw\""); \
	__asm__("\t.long " #a); \
	__asm__(".text");

#define ADD2LIST24(a,b) \
	__asm__(".section\t.dlist_" #b ",\"aw\""); \
	__asm__("\t.long " #a); \
	__asm__(".text");

/* Install private constructors and destructors pri MUST be -127<=pri<=127 */
#define ADD2INIT(a,pri) ADD2LIST(a,__INIT_LIST__,22); \
                        ADD2LIST2(pri+128,__INIT_LIST__,22)
#define ADD2EXIT(a,pri) ADD2LIST(a,__EXIT_LIST__,22); \
                        ADD2LIST2(pri+128,__EXIT_LIST__,22)

#define ADDTABL_END() ADD2LIST2(-1,__FuncTable__,22)

/* These are some macros for handling of symboltable information
 */

/* linker can use symbol b for symbol a if a is not defined */
#define ALIAS(a,b) __asm__(".stabs \"_" #a "\",11,0,0,0;.stabs \"_" #b "\",1,0,0,0")


/* Add to library list */
#define ADD2LIB(a) ADD2LIST(a,__LIB_LIST__,24)

/* This one does not really handle symbol tables
 * it's just pointless to write a header file for one macro
 *
 * define a as a label for an absolute address b
 */
#define ABSDEF(a,b) __asm__("_" #a "=" #b ";.globl _" #a )

/* Generate assembler stub for a shared library entry
 * and add it to the jump table
 * ADDTABL_X(name,...)	means function with X arguments
 * ADDTABL_END()	ends the list
 * Usage: ADDTABL_2(AddHead,a0,a1);
 * No more than 4 arguments supported, use structures!
 */
 
#define _ADDTABL_START(name)		\
__asm__(".globl ___" #name);		\
__asm__("___" #name ":\tmovel a4,sp@-")

#define _ADDTABL_ARG(arg)		\
__asm__("\tmovel " #arg ",sp@-")

#define _ADDTABL_CALL(name)		\
__asm__("\tmovel a6@(40:W),a4");		\
__asm__("\tbsr _" #name)

#define _ADDTABL_END0(name,numargs)	\
__asm__("\tmovel sp@+,a4");			\
__asm__("\trts");				\
ADD2LIST(__##name,__FuncTable__,22)

#define _ADDTABL_END2(name,numargs)	\
__asm__("\taddqw #4*" #numargs ",sp");	\
__asm__("\tmovel sp@+,a4");			\
__asm__("\trts");				\
ADD2LIST(__##name,__FuncTable__,22)

#define _ADDTABL_ENDN(name,numargs)	\
__asm__("\taddaw #4*" #numargs ",sp");	\
__asm__("\tmovel sp@+,a4");			\
__asm__("\trts");				\
ADD2LIST(__##name,__FuncTable__,22)

#define ADDTABL_0(name)			\
_ADDTABL_START(name);			\
_ADDTABL_CALL(name);			\
_ADDTABL_END0(name,0)

#define ADDTABL_1(name,arg1)		\
_ADDTABL_START(name);			\
_ADDTABL_ARG(arg1);			\
_ADDTABL_CALL(name);			\
_ADDTABL_END2(name,1)

#define ADDTABL_2(name,arg1,arg2)	\
_ADDTABL_START(name);			\
_ADDTABL_ARG(arg2);			\
_ADDTABL_ARG(arg1);			\
_ADDTABL_CALL(name);			\
_ADDTABL_END2(name,2)

#define ADDTABL_3(name,arg1,arg2,arg3)	\
_ADDTABL_START(name);			\
_ADDTABL_ARG(arg3);			\
_ADDTABL_ARG(arg2);			\
_ADDTABL_ARG(arg1);			\
_ADDTABL_CALL(name);			\
_ADDTABL_ENDN(name,3)

#define ADDTABL_4(name,arg1,arg2,arg3,arg4) \
_ADDTABL_START(name);			\
_ADDTABL_ARG(arg4);			\
_ADDTABL_ARG(arg3);			\
_ADDTABL_ARG(arg2);			\
_ADDTABL_ARG(arg1);			\
_ADDTABL_CALL(name);			\
_ADDTABL_ENDN(name,4)



#endif /* _HEADERS_STABS_H */
