#ifndef _CSMK_H_
#define _CSMK_H_

#include <bigint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support.h>


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
    uint32_t *pc;
    int prog_length;
    int sda_length;
    char *code_file_name;
    int vm_version;
    int *br_point;

} csmk_db_init_object;

extern csmk_db_util_object db_object;
extern csmk_db_init_object db_init_object;

void csmk_db_vars_init(void *stack, void *sda, void *ret_reg,
                         unsigned int *prog, int *sp, int *fp, uint32_t *pc,
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

#ifndef NJVM_UTILS_MACRO_DEFINES

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
#define HALT 0

#define PUSHC 1

#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6

#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10

#define PUSHG 11
#define POPG 12

#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16

#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22

#define JMP 23
#define BRF 24
#define BRT 25

#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30

#define DUP 31

#define NEW 32
#define GETF 33
#define PUTF 34

#define NEWA 35
#define GETFA 36
#define PUTFA 37

#define GETSZ 38

#define PUSHN 39
#define REFEQ 40
#define REFNE 41
#endif /* NJVM_UTILS_MACRO_DEFINES*/

#endif /* _CSMK_H_ */
