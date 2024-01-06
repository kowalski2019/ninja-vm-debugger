/**
 * @file csmk_db.c
 * @author Claude Stephane Kouame (stephane.kouame@africasgeeks.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "csmk_db.h"

csmk_db_util_object db_object;
csmk_db_init_object db_init_object;

bool csmk_db_on = true;


void csmk_db_vars_init(void *stack, void *sda, void *ret_reg,
                       unsigned int *prog, int *sp, int *fp, int *pc,
                       int prog_length, int sda_length, char *file_name,
                       bool *debug, int vm_version, void (*runner)(void), int *br_point) {
  db_init_object.stack = stack;
  db_init_object.sda = sda;
  db_init_object.ret_reg = ret_reg;

  db_init_object.program_mem = prog;
  db_init_object.sp = sp;
  db_init_object.fp = fp;
  db_init_object.pc = pc;
  db_init_object.prog_length = prog_length;
  db_init_object.sda_length = sda_length;
  db_init_object.code_file_name = file_name;
  db_init_object.vm_version = vm_version;
  db_init_object.br_point = br_point;

  db_object.debug = debug;
  db_object.bp = -1;
  db_object.halt = true;
  db_object.vm_runner = runner;
}

void csmk_db_run_debugger(void) {
  printf("CSMK DEBUG: file '%s' loaded (prog size [%d] | sda size [%d]) \n",
         db_init_object.code_file_name, db_init_object.prog_length,
         db_init_object.sda_length);
  char input_buff[32];
  while (csmk_db_on) {
    *db_object.debug = true;
    printf("\nActual Instruction to run: ");
    csmk_db_print_inst(*db_init_object.pc,
                       db_init_object.program_mem[*db_init_object.pc]);
    printf("\n");
    printf("CSMK_DEBUG: pc (%d) | sp (%d) | fp (%d) \n", *db_init_object.pc,
           *db_init_object.sp, *db_init_object.fp);
    csmk_db_show_ret_reg();
    printf(
        "\t=> inspect[i], list[l], breakpoint[b], step[s], run[r], quit[q]?\n");
    fgets(input_buff, 32, stdin);
    if (strcmp(input_buff, "inspect") == 0 || input_buff[0] == 'i') {
      char i_buf[16];
      printf("DEBUG [inspect]: stack, data?\n");
      fgets(i_buf, 16, stdin);
      if (strcmp(i_buf, "stack") == 0 || i_buf[0] == 's') {
        csmk_db_print_stack();
      } else if (strcmp(i_buf, "data") == 0 || i_buf[0] == 'd') {
        if (db_init_object.sda_length == 0) {
          printf("SDA is empty\n");
        } else {
          csmk_db_print_sda();
        }
      } else {
        printf("Unknown Inspect Option\n");
      }
    } else if (strcmp(input_buff, "list") == 0 || input_buff[0] == 'l') {
      int i = 0;
      while ((i++) < db_init_object.prog_length) {
        csmk_db_print_inst(i, db_init_object.program_mem[i]);
      }
    } else if (strcmp(input_buff, "breakpoint") == 0 || input_buff[0] == 'b') {
      csmk_db_bp_handler();
    } else if (strcmp(input_buff, "step") == 0 || input_buff[0] == 's') {
      db_object.vm_runner();
    } else if (strcmp(input_buff, "run") == 0 || input_buff[0] == 'r') {
      //#undef CSMK_DBG_ON
      //#define CSMK_DBG_ON 0
      //
      db_object.halt = false;
      *db_object.debug = false;
      db_object.vm_runner();
      /*if (db_object.bp == -1) {
        *db_object.debug = false;
        csmk_db_on = false;
      }*/
      db_object.halt = true;
      break;
    } else if (strcmp(input_buff, "quit") == 0 || input_buff[0] == 'q') {
      printf("Happy Hacking\n\t bye\n");
      break;
    } else {
      printf("Unknown Option\n");
    }
  }
}
bool csmk_db_is_integer(char *buf) {
  // bool neg_flag = false;
  int i = 0;
  if (buf[0] == '-') {
    // neg_flag = true;
    i += 1;
    if (strlen(buf) == 1)
      return false;
  }
  for (; i < strlen(buf) - 1; i++) {
    if (buf[i] < '0' || buf[i] > '9') {
      return false;
    }
  }
  return true;
}
void csmk_db_bp_handler(void) {
  char buf[10];
  bool finished = false;
  if (db_object.bp == -1) {
    printf("[BREAKPOINT]: -1 => no value actually\n");
  } else {
    printf("[BREAKPOINT]: %d => actual value\n", db_object.bp);
  }
  while (!finished) {
    printf("\t\tenter a value to set the bp, [c] to clear or [q] for no "
           "updates\n");
    fgets(buf, 10, stdin);
    bool res = csmk_db_is_integer(buf);
    // printf("was a valid ? %s \n",res ? "yes" : "no");
    if (res) {
      int new_bp = atoi(buf);
      if (new_bp < 0) {
        printf("Invalid breakpoint value\n");
      } else {
        db_object.bp = new_bp;
        printf("Breakpoint succesfully set\n");
        finished = true;
      }
    } else {
      if (buf[0] == 'q')
        finished = true;
      else if (buf[0] == 'c') {
        db_object.bp = -1;
        printf("Breakpoint succesfully cleared\n");
        finished = true;
      } else {
        printf("Unknown option or invalid breakpoint value\n");
      }
    }
  }
  *db_init_object.br_point = db_object.bp;
}
void csmk_db_print_stack(void) {
  int sp = *db_init_object.sp;
  bool print_space = true;
  while (sp > -1) {
    print_space = true;
    if (sp == *db_init_object.sp && sp == *db_init_object.fp) {
      printf("sp, fp ====> ");
      print_space = false;
    }
    if (sp == *db_init_object.sp && sp != *db_init_object.fp) {
      printf("sp  =====> ");
      print_space = false;
    }
    if (sp == *db_init_object.fp && sp != *db_init_object.sp) {
      printf("fp  =====> ");
      print_space = false;
    }
    if (print_space)
      printf("           ");
    printf("[%04d]:\t", sp);
    printf("[ ");
    if (sp == *db_init_object.sp) {
      printf("xxxxxxxxxxx");
    } else {
      if (db_init_object.vm_version < 5) {
        int *db_stack = (int*)db_init_object.stack;
        printf("%d", db_stack[sp]);
      } else {
        StackSlot *db_stack = (StackSlot*)db_init_object.stack;
        if (db_stack[sp].isObjRef) {
          #ifdef BIGINT_SUPPORTED
          if (db_stack[sp].u.objRef != NULL) {
            // handle object here
            //printf("Handle Object \n");
            csmk_db_print_object(db_stack[sp].u.objRef);
          } else {
            printf(" (nil) Object");
          }
          #endif
        } else {
          printf("%d", db_stack[sp].u.number);
        }
      }
    }
    printf(" ]\n");
    sp -= 1;
  }
}

#ifdef BIGINT_SUPPORTED
void csmk_db_print_object(void *ob) {
  ObjRef obj = (ObjRef)ob;
  if (obj == NULL) {
    printf("nil ");
    return;
  }
  bip.op1 = obj;
  printf("0x%p", (void *)obj);
  if (IS_PRIM(obj)) {
    printf("(");
  } else {
    printf("{ ");
    for (int i = 0; i < GET_SIZE(obj); i++) {
      csmk_db_print_object(GET_REFS(obj)[i]);
    }
  }

  if (IS_PRIM(obj)) {
    bip.op1 = obj;
    bigPrint(stdout);

    printf(") ");
  } else {
    printf(" }");
  }
}
#endif

void csmk_db_print_sda(void) {
  int i = db_init_object.sda_length - 1;
  while (i > -1) {
    printf("[%04d]:\t", i);
    printf("[ ");
    if (db_init_object.vm_version < 5) {
        int *db_sda = (int*)db_init_object.sda;
      printf("%d", db_sda[i]);
    } else {
      #ifdef BIGINT_SUPPORTED
      ObjRef *db_sda = (ObjRef*)db_init_object.sda;
      csmk_db_print_object(db_sda[i]);
      #endif
    }
    printf(" ]\n");
    i -= 1;
  }
}
void csmk_db_show_ret_reg(void) {
  printf("\t=> ret_reg [ ");
  printf("%p", db_init_object.ret_reg);
  printf(" (");
  if (db_init_object.vm_version < 5) {
    int *ret_reg = (int*)db_init_object.ret_reg;
    printf("%d", *ret_reg);
  } else {
    #ifdef BIGINT_SUPPORTED
    ObjRef *obj = (ObjRef*)db_init_object.ret_reg;
    if (*obj != NULL) {
      bip.op1 = *obj;
      bigPrint(stdout);
    }
    #endif /* BIGINT_SUPPORTED */
  }
  printf(") ]\n");
}
void csmk_db_print_inst(int pc, unsigned int ir) {
  printf("[%04d]\t", pc);
  switch ((ir) >> 24) {
  case DB_HALT:
    printf("halt");
    break;
  case DB_PUSHC:
    printf("pushc\t%d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_ADD:
    printf("add");
    break;
  case DB_SUB:
    printf("sub");
    break;
  case DB_MUL:
    printf("mul");
    break;
  case DB_DIV:
    printf("div");
    break;
  case DB_MOD:
    printf("mod");
    break;
  case DB_RDINT:
    printf("rdint");
    break;
  case DB_WRINT:
    printf("wrint");
    break;
  case DB_RDCHR:
    printf("rdchr");
    break;
  case DB_WRCHR:
    printf("wrchr");
    break;
  case DB_PUSHG:
    printf("pushg\t%d", IMMEDIATE(ir));
    break;
  case DB_POPG:
    printf("popg\t%d", IMMEDIATE(ir));
    break;
  case DB_ASF:
    printf("asf\t%d", IMMEDIATE(ir));
    break;
  case DB_RSF:
    printf("rsf");
    break;
  case DB_PUSHL:
    printf("pushl\t%d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_POPL:
    printf("popl\t%d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_EQ:
    printf("eq");
    break;
  case DB_NE:
    printf("ne");
    break;
  case DB_LT:
    printf("lt");
    break;
  case DB_LE:
    printf("le");
    break;
  case DB_GT:
    printf("gt");
    break;
  case DB_GE:
    printf("ge");
    break;
  case DB_JMP:
    printf("jmp\t%d", IMMEDIATE(ir));
    break;
  case DB_BRF:
    printf("brf\t%d", IMMEDIATE(ir));
    break;
  case DB_BRT:
    printf("brt\t%d", IMMEDIATE(ir));
    break;
  case DB_CALL:
    printf("call\t%d", IMMEDIATE(ir));
    break;
  case DB_RET:
    printf("ret");
    break;
  case DB_DROP:
    printf("drop\t%d", IMMEDIATE(ir));
    break;
  case DB_PUSHR:
    printf("pushr");
    break;
  case DB_POPR:
    printf("popr");
    break;
  case DB_DUP:
    printf("dup");
    break;
  case DB_NEW:
    printf("new\t%d", IMMEDIATE(ir));
    break;
  case DB_GETF:
    printf("getf\t%d", IMMEDIATE(ir));
    break;
  case DB_PUTF:
    printf("putf\t%d", IMMEDIATE(ir));
    break;
  case DB_NEWA:
    printf("newa");
    break;
  case DB_GETFA:
    printf("getfa");
    break;
  case DB_PUTFA:
    printf("putfa");
    break;
  case DB_GETSZ:
    printf("getsz");
    break;
  case DB_PUSHN:
    printf("pushn");
    break;
  case DB_REFEQ:
    printf("refeq");
    break;
  case DB_REFNE:
    printf("refne");
    break;
  }
  printf("\n");
}
