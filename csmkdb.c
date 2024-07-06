/**
 * @file csmkdb.c
 * @author Claude Stephane Kouame (stephane.kouame@africasgeeks.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "csmkdb.h"

csmkdb_util_object db_object;
csmkdb_init_object db_init_object;

bool csmkdb_on = true;
const char *logfile_name = "csmkdb.log";
FILE *logfile;


void csmkdb_vars_init(void *stack, void *sda, void *ret_reg,
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
  db_object.run = false;
  db_object.vm_runner = runner;

  signal(SIGSEGV, csmkdb_signal_handler);
}

void csmkdb_run_debugger(void) {
  printf("CSMK DEBUG: file '%s' loaded (prog size [%d] | sda size [%d]) \n",
         db_init_object.code_file_name, db_init_object.prog_length,
         db_init_object.sda_length);
  char input_buff[32];
  char buffer[50];
  while (csmkdb_on) {
    *db_object.debug = true;
    printf("\nActual Instruction to run: ");
    csmkdb_print_inst(*db_init_object.pc,
                       db_init_object.program_mem[*db_init_object.pc]);
    printf("\n");
    printf("CSMK_DEBUG: pc (%d) | sp (%d) | fp (%d) \n", *db_init_object.pc,
           *db_init_object.sp, *db_init_object.fp);
    csmkdb_show_ret_reg();
    printf(
        "\t=> inspect[i], list[l], breakpoint[b], step[s], run[r], quit[q]?\n");
    fgets(input_buff, 32, stdin);
    if (strcmp(input_buff, "inspect") == 0 || input_buff[0] == 'i') {
      char i_buf[16];
      printf("DEBUG [inspect]: stack, data?\n");
      fgets(i_buf, 16, stdin);
      if (strcmp(i_buf, "stack") == 0 || i_buf[0] == 's') {
        csmkdb_print_stack();
      } else if (strcmp(i_buf, "data") == 0 || i_buf[0] == 'd') {
        if (db_init_object.sda_length == 0) {
          printf("SDA is empty\n");
        } else {
          csmkdb_print_sda();
        }
      } else {
        printf("Unknown Inspect Option\n");
      }
    } else if (strcmp(input_buff, "list") == 0 || input_buff[0] == 'l') {
      int i = 0;
      while ((i++) < db_init_object.prog_length) {
        csmkdb_print_inst(i, db_init_object.program_mem[i]);
      }
    } else if (strcmp(input_buff, "breakpoint") == 0 || input_buff[0] == 'b') {
      csmkdb_bp_handler();
    } else if (strcmp(input_buff, "step") == 0 || input_buff[0] == 's') {
      sprintf(buffer, "[%04d]\t%s", *db_init_object.pc, csmkdb_format_inst(*db_init_object.pc, db_init_object.program_mem[*db_init_object.pc]));
      csmkdb_log_message(logfile_name, buffer);
      db_object.vm_runner();
    } else if (strcmp(input_buff, "run") == 0 || input_buff[0] == 'r') {
      if (db_object.bp == -1) {
        //*db_object.debug = false;
         csmkdb_on = false;
        db_object.run = true;
        while(db_object.run) {
          sprintf(buffer, "[%04d]\t%s", *db_init_object.pc, csmkdb_format_inst(*db_init_object.pc, db_init_object.program_mem[*db_init_object.pc]));
          csmkdb_log_message(logfile_name, buffer);
          db_object.vm_runner();
          if (DB_HALT == db_init_object.program_mem[*db_init_object.pc]) {
            csmkdb_on = false;
            break;
          }
        }
        db_object.run = false;
        //break;
      } else {
        db_object.run = true;
        while(db_object.run) {
          sprintf(buffer, "[%04d]\t%s", *db_init_object.pc, csmkdb_format_inst(*db_init_object.pc, db_init_object.program_mem[*db_init_object.pc]));
          csmkdb_log_message(logfile_name, buffer);
          db_object.vm_runner();
          if (db_object.bp == *db_init_object.pc) {
            db_object.run = false;
          }
          if (DB_HALT == db_init_object.program_mem[*db_init_object.pc]) {
            csmkdb_on = false;
            break;
          }
        }
      }
    } else if (strcmp(input_buff, "quit") == 0 || input_buff[0] == 'q') {
      printf("Happy Hacking\n\t bye\n");
      break;
    } else {
      printf("Unknown Option\n");
    }
  }
  fclose(logfile);
}
bool csmkdb_is_integer(char *buf) {
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
void csmkdb_bp_handler(void) {
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
    bool res = csmkdb_is_integer(buf);
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
void csmkdb_print_stack(void) {
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
            csmkdb_print_object(db_stack[sp].u.objRef);
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
void csmkdb_print_object(void *ob) {
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
      csmkdb_print_object(GET_REFS(obj)[i]);
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

void csmkdb_print_sda(void) {
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
      csmkdb_print_object(db_sda[i]);
      #endif
    }
    printf(" ]\n");
    i -= 1;
  }
}
void csmkdb_show_ret_reg(void) {
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
char *csmkdb_format_inst(int pc, unsigned int ir) {
  char *res = malloc(sizeof(char)*10);
  switch ((ir) >> 24) {
  case DB_HALT:
    sprintf(res, "halt");
    break;
  case DB_PUSHC:
    sprintf(res, "pushc %d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_ADD:
    sprintf(res, "add");
    break;
  case DB_SUB:
    sprintf(res, "sub");
    break;
  case DB_MUL:
    sprintf(res, "mul");
    break;
  case DB_DIV:
    sprintf(res, "div");
    break;
  case DB_MOD:
    sprintf(res, "mod");
    break;
  case DB_RDINT:
    sprintf(res, "rdint");
    break;
  case DB_WRINT:
    sprintf(res, "wrint");
    break;
  case DB_RDCHR:
    sprintf(res, "rdchr");
    break;
  case DB_WRCHR:
    sprintf(res, "wrchr");
    break;
  case DB_PUSHG:
    sprintf(res, "pushg\t%d", IMMEDIATE(ir));
    break;
  case DB_POPG:
   sprintf(res, "popg\t%d", IMMEDIATE(ir));
    break;
  case DB_ASF:
    sprintf(res, "asf\t%d", IMMEDIATE(ir));
    break;
  case DB_RSF:
    sprintf(res, "rsf");
    break;
  case DB_PUSHL:
    sprintf(res, "pushl\t%d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_POPL:
    sprintf(res, "popl\t%d", SIGN_EXTEND(IMMEDIATE(ir)));
    break;
  case DB_EQ:
    sprintf(res, "eq");
    break;
  case DB_NE:
    sprintf(res, "ne");
    break;
  case DB_LT:
    sprintf(res, "lt");
    break;
  case DB_LE:
    sprintf(res, "le");
    break;
  case DB_GT:
    sprintf(res, "gt");
    break;
  case DB_GE:
    sprintf(res, "ge");
    break;
  case DB_JMP:
    sprintf(res, "jmp\t%d", IMMEDIATE(ir));
    break;
  case DB_BRF:
    sprintf(res, "brf\t%d", IMMEDIATE(ir));
    break;
  case DB_BRT:
    sprintf(res, "brt\t%d", IMMEDIATE(ir));
    break;
  case DB_CALL:
    sprintf(res, "call\t%d", IMMEDIATE(ir));
    break;
  case DB_RET:
    sprintf(res, "ret");
    break;
  case DB_DROP:
    sprintf(res, "drop\t%d", IMMEDIATE(ir));
    break;
  case DB_PUSHR:
    sprintf(res, "pushr");
    break;
  case DB_POPR:
    sprintf(res, "popr");
    break;
  case DB_DUP:
    sprintf(res, "dup");
    break;
  case DB_NEW:
    sprintf(res, "new\t%d", IMMEDIATE(ir));
    break;
  case DB_GETF:
    sprintf(res, "getf\t%d", IMMEDIATE(ir));
    break;
  case DB_PUTF:
    sprintf(res, "putf\t%d", IMMEDIATE(ir));
    break;
  case DB_NEWA:
    sprintf(res, "newa");
    break;
  case DB_GETFA:
    sprintf(res, "getfa");
    break;
  case DB_PUTFA:
    sprintf(res, "putfa");
    break;
  case DB_GETSZ:
    sprintf(res, "getsz");
    break;
  case DB_PUSHN:
    sprintf(res, "pushn");
    break;
  case DB_REFEQ:
    sprintf(res, "refeq");
    break;
  case DB_REFNE:
    sprintf(res, "refne");
    break;
  default:
    sprintf(res, "unknown");
  }
  return res;
}

void csmkdb_print_inst(int pc, unsigned int ir) {
  printf("[%04d]\t", pc);
  printf("%s\n", csmkdb_format_inst(pc, ir));
}

char* csmkdb_get_timestamp(void) {
    time_t rawtime;
    struct tm * timeinfo;
    char *buffer = malloc(20 * sizeof(char));

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", timeinfo);

    return buffer; 
}
void csmkdb_log_message(const char* filename, const char* message) {
    FILE *logfile = fopen(filename, "a");
    if (logfile == NULL) {
        perror("Failed to open log file");
        return;
    }

    char *timestamp = csmkdb_get_timestamp();
    fprintf(logfile, "[%s] %s\n", timestamp, message);
    free(timestamp);
    fclose(logfile);
}

void csmkdb_signal_handler(int signal) {
    void *array[10];
    size_t size;

    // Get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // Print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", signal);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    fclose(logfile);
    exit(0);
}