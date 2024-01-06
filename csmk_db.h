/**
 * @file csmk_db.h
 * @author Claude Stephane Kouame (stephane.kouame@africasgeeks.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _CSMK_H_
#define _CSMK_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIGINT_SUPPORTED

#ifdef BIGINT_SUPPORTED
#include <bigint.h>
#include <support.h>
#endif


typedef struct {
  void (*vm_runner)(void);
  bool *debug;
  bool halt;
  int bp;
} csmk_db_util_object;


typedef struct {
    void *stack; // can be StackSlot or int
    void *sda;   // can be ObjRef or int*
    void *ret_reg; // can be ObjRef or int
    unsigned int *program_mem;
    int *sp;
    int *fp;
    int *pc;
    int prog_length;
    int sda_length;
    char *code_file_name;
    int vm_version;
    int *br_point;

} csmk_db_init_object;

extern csmk_db_util_object db_object;
extern csmk_db_init_object db_init_object;

void csmk_db_vars_init(void *stack, void *sda, void *ret_reg,
                         unsigned int *prog, int *sp, int *fp, int *pc,
                         int prog_length, int sda_length, char *file_name,
                         bool *debug, int vm_version, void (*runner)(void), int *br_point);
void csmk_db_run_debugger(void);
void csmk_db_print_inst(int pc, unsigned int inst);
void csmk_db_print_stack(void);
void csmk_db_print_sda(void);
void csmk_db_show_ret_reg(void);
void csmk_db_bp_handler(void);
bool csmk_db_is_integer(char *buf);
void csmk_db_print_object(void* ob);

//#define CSMK_DBG_ON 1

#define NJVM_UTILS_MACRO
#ifdef NJVM_UTILS_MACRO

typedef struct {
  unsigned int size;     /* byte count of payload data */
  unsigned char data[1]; /* payload data , size as needed */
} * ObjRef;

typedef struct {
  bool isObjRef; /* slot used for object reference ? */
  union {
    ObjRef objRef; /* used if isObjRef=TRUE */
    int number;    /* used if isObjRef=FALSE */

  } u;
} StackSlot;

#endif /* NJVM_UTILS_MACRO */

#define IMMEDIATE(x) ((x)&0x00FFFFFF)
#define SIGN_EXTEND(i) ((i)&0x00800000 ? (i) | 0xFF000000 : (i))

#define GET_OBJ_VALUE(obj) (*(int *)obj->data)

#define MSB                                                                    \
  (1 << (8 * sizeof(unsigned int) - 1)) // 10000000 00000000 00000000 00000000
#define IS_PRIM(objRef) (((objRef)->size & MSB) == 0)
#define GET_SIZE(objRef)                                                       \
  ((objRef)->size & ~MSB) // ~MSB = 01111111 11111111 11111111 11111111
#define GET_REFS(objRef) ((ObjRef *)(objRef)->data)



// Instruction
#define DB_HALT 0

#define DB_PUSHC 1

#define DB_ADD 2
#define DB_SUB 3
#define DB_MUL 4
#define DB_DIV 5
#define DB_MOD 6

#define DB_RDINT 7
#define DB_WRINT 8
#define DB_RDCHR 9
#define DB_WRCHR 10

#define DB_PUSHG 11
#define DB_POPG 12

#define DB_ASF 13
#define DB_RSF 14
#define DB_PUSHL 15
#define DB_POPL 16

#define DB_EQ 17
#define DB_NE 18
#define DB_LT 19
#define DB_LE 20
#define DB_GT 21
#define DB_GE 22

#define DB_JMP 23
#define DB_BRF 24
#define DB_BRT 25

#define DB_CALL 26
#define DB_RET 27
#define DB_DROP 28
#define DB_PUSHR 29
#define DB_POPR 30

#define DB_DUP 31

#define DB_NEW 32
#define DB_GETF 33
#define DB_PUTF 34

#define DB_NEWA 35
#define DB_GETFA 36
#define DB_PUTFA 37

#define DB_GETSZ 38

#define DB_PUSHN 39
#define DB_REFEQ 40
#define DB_REFNE 41


#endif /* _CSMK_H_ */
