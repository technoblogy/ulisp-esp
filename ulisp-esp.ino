/* uLisp ESP Version 3.6 - www.ulisp.com
   David Johnson-Davies - www.technoblogy.com - 4th April 2021

   Licensed under the MIT license: https://opensource.org/licenses/MIT
*/

// Lisp Library
const char LispLibrary[] PROGMEM = "";

// Compile options

// #define resetautorun
#define printfreespace
// #define printgcs
// #define sdcardsupport
// #define gfxsupport
// #define lisplibrary
// #define lineeditor
// #define vt100

// Includes

// #include "LispLibrary.h"
#include <setjmp.h>
#include <SPI.h>
#include <Wire.h>
#include <limits.h>
#include <EEPROM.h>
#if defined (ESP8266)
  #include <ESP8266WiFi.h>
#elif defined (ESP32)
  #include <WiFi.h>
#endif

#if defined(gfxsupport)
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_SSD1306.h>
#define COLOR_WHITE 1
#define COLOR_BLACK 0
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4
Adafruit_SSD1306 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#endif

#if defined(sdcardsupport)
  #include <SD.h>
  #define SDSIZE 172
#else
  #define SDSIZE 0
#endif

// Platform specific settings

#define WORDALIGNED __attribute__((aligned (4)))
#define BUFFERSIZE 34  // Number of bits+2

#if defined(ESP8266)
  #define WORKSPACESIZE (4000-SDSIZE)     /* Cells (8*bytes) */
  #define EEPROMSIZE 4096                 /* Bytes available for EEPROM */
  #define SYMBOLTABLESIZE 512             /* Bytes */
  #define SDCARD_SS_PIN 10

#elif defined(ESP32)
  #define WORKSPACESIZE (8000-SDSIZE)     /* Cells (8*bytes) */
  #define EEPROMSIZE 4096                 /* Bytes available for EEPROM */
  #define SYMBOLTABLESIZE 1024            /* Bytes */
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13

#else
#error "Board not supported!"
#endif

// C Macros

#define nil                NULL
#define car(x)             (((object *) (x))->car)
#define cdr(x)             (((object *) (x))->cdr)

#define first(x)           (((object *) (x))->car)
#define second(x)          (car(cdr(x)))
#define cddr(x)            (cdr(cdr(x)))
#define third(x)           (car(cdr(cdr(x))))

#define push(x, y)         ((y) = cons((x),(y)))
#define pop(y)             ((y) = cdr(y))

#define integerp(x)        ((x) != NULL && (x)->type == NUMBER)
#define floatp(x)          ((x) != NULL && (x)->type == FLOAT)
#define symbolp(x)         ((x) != NULL && (x)->type == SYMBOL)
#define stringp(x)         ((x) != NULL && (x)->type == STRING)
#define characterp(x)      ((x) != NULL && (x)->type == CHARACTER)
#define arrayp(x)          ((x) != NULL && (x)->type == ARRAY)
#define streamp(x)         ((x) != NULL && (x)->type == STREAM)

#define mark(x)            (car(x) = (object *)(((uintptr_t)(car(x))) | MARKBIT))
#define unmark(x)          (car(x) = (object *)(((uintptr_t)(car(x))) & ~MARKBIT))
#define marked(x)          ((((uintptr_t)(car(x))) & MARKBIT) != 0)
#define MARKBIT            1

#define setflag(x)         (Flags = Flags | 1<<(x))
#define clrflag(x)         (Flags = Flags & ~(1<<(x)))
#define tstflag(x)         (Flags & 1<<(x))

#define issp(x)            (x == ' ' || x == '\n' || x == '\r' || x == '\t')

// Constants

const int TRACEMAX = 3; // Number of traced functions
enum type { ZZERO=0, SYMBOL=2, CODE=4, NUMBER=6, STREAM=8, CHARACTER=10, FLOAT=12, ARRAY=14, STRING=16, PAIR=18 };  // ARRAY STRING and PAIR must be last
enum token { UNUSED, BRA, KET, QUO, DOT };
enum stream { SERIALSTREAM, I2CSTREAM, SPISTREAM, SDSTREAM, WIFISTREAM, STRINGSTREAM, GFXSTREAM };

// Stream names used by printobject
const char serialstream[] PROGMEM = "serial";
const char i2cstream[] PROGMEM = "i2c";
const char spistream[] PROGMEM = "spi";
const char sdstream[] PROGMEM = "sd";
const char wifistream[] PROGMEM = "wifi";
const char stringstream[] PROGMEM = "string";
const char gfxstream[] PROGMEM = "gfx";
PGM_P const streamname[] PROGMEM = {serialstream, i2cstream, spistream, sdstream, wifistream, stringstream, gfxstream};

// Typedefs

typedef unsigned int symbol_t;

typedef struct sobject {
  union {
    struct {
      sobject *car;
      sobject *cdr;
    };
    struct {
      unsigned int type;
      union {
        symbol_t name;
        int integer;
        int chars; // For strings
        float single_float;
      };
    };
  };
} object;

typedef object *(*fn_ptr_type)(object *, object *);
typedef void (*mapfun_t)(object *, object **);

typedef struct {
  PGM_P string;
  fn_ptr_type fptr;
  uint8_t minmax;
} tbl_entry_t;

typedef int (*gfun_t)();
typedef void (*pfun_t)(char);
typedef int PinMode;
typedef int BitOrder;

enum function { NIL, TEE, NOTHING, OPTIONAL, INITIALELEMENT, ELEMENTTYPE, BIT, AMPREST, LAMBDA, LET,
LETSTAR, CLOSURE, SPECIAL_FORMS, QUOTE, OR, DEFUN, DEFVAR, SETQ, LOOP, RETURN, PUSH, POP, INCF, DECF,
SETF, DOLIST, DOTIMES, TRACE, UNTRACE, FORMILLIS, WITHOUTPUTTOSTRING, WITHSERIAL, WITHI2C, WITHSPI,
WITHSDCARD, WITHGFX, WITHCLIENT, TAIL_FORMS, PROGN, IF, COND, WHEN, UNLESS, CASE, AND, FUNCTIONS, NOT,
NULLFN, CONS, ATOM, LISTP, CONSP, SYMBOLP, ARRAYP, BOUNDP, SETFN, STREAMP, EQ, CAR, FIRST, CDR, REST,
CAAR, CADR, SECOND, CDAR, CDDR, CAAAR, CAADR, CADAR, CADDR, THIRD, CDAAR, CDADR, CDDAR, CDDDR, LENGTH,
ARRAYDIMENSIONS, LIST, MAKEARRAY, REVERSE, NTH, AREF, ASSOC, MEMBER, APPLY, FUNCALL, APPEND, MAPC, MAPCAR,
MAPCAN, ADD, SUBTRACT, MULTIPLY, DIVIDE, MOD, ONEPLUS, ONEMINUS, ABS, RANDOM, MAXFN, MINFN, NOTEQ, NUMEQ,
LESS, LESSEQ, GREATER, GREATEREQ, PLUSP, MINUSP, ZEROP, ODDP, EVENP, INTEGERP, NUMBERP, FLOATFN, FLOATP,
SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, EXP, SQRT, LOG, EXPT, CEILING, FLOOR, TRUNCATE, ROUND,
CHAR, CHARCODE, CODECHAR, CHARACTERP, STRINGP, STRINGEQ, STRINGLESS, STRINGGREATER, SORT, STRINGFN,
CONCATENATE, SUBSEQ, READFROMSTRING, PRINCTOSTRING, PRIN1TOSTRING, LOGAND, LOGIOR, LOGXOR, LOGNOT, ASH,
LOGBITP, EVAL, GLOBALS, LOCALS, MAKUNBOUND, BREAK, READ, PRIN1, PRINT, PRINC, TERPRI, READBYTE, READLINE,
WRITEBYTE, WRITESTRING, WRITELINE, RESTARTI2C, GC, ROOM, SAVEIMAGE, LOADIMAGE, CLS, PINMODE, DIGITALREAD,
DIGITALWRITE, ANALOGREAD, ANALOGREADRESOLUTION, ANALOGWRITE, DELAY, MILLIS, SLEEP, NOTE, EDIT, PPRINT,
PPRINTALL, FORMAT, REQUIRE, LISTLIBRARY, AVAILABLE, WIFISERVER, WIFISOFTAP, CONNECTED, WIFILOCALIP,
WIFICONNECT, DRAWPIXEL, DRAWLINE, DRAWRECT, FILLRECT, DRAWCIRCLE, FILLCIRCLE, DRAWROUNDRECT,
FILLROUNDRECT, DRAWTRIANGLE, FILLTRIANGLE, DRAWCHAR, SETCURSOR, SETTEXTCOLOR, SETTEXTSIZE, SETTEXTWRAP,
FILLSCREEN, SETROTATION, INVERTDISPLAY, KEYWORDS, 
K_HIGH, K_LOW,
#if defined(ESP8266)
K_INPUT, K_INPUT_PULLUP, K_OUTPUT,
#elif defined(ESP32)
K_INPUT, K_INPUT_PULLUP, K_INPUT_PULLDOWN, K_OUTPUT,
#endif
USERFUNCTIONS, ENDFUNCTIONS };

// Global variables

object Workspace[WORKSPACESIZE] WORDALIGNED;
char SymbolTable[SYMBOLTABLESIZE];

jmp_buf exception;
unsigned int Freespace = 0;
object *Freelist;
char *SymbolTop = SymbolTable;
unsigned int I2CCount;
unsigned int TraceFn[TRACEMAX];
unsigned int TraceDepth[TRACEMAX];

object *GlobalEnv;
object *GCStack = NULL;
object *GlobalString;
int GlobalStringIndex = 0;
uint8_t PrintCount = 0;
uint8_t BreakLevel = 0;
char LastChar = 0;
char LastPrint = 0;

// Flags
enum flag { PRINTREADABLY, RETURNFLAG, ESCAPE, EXITEDITOR, LIBRARYLOADED, NOESC, NOECHO };
volatile uint8_t Flags = 0b00001; // PRINTREADABLY set by default

// Forward references
object *tee;
object *tf_progn (object *form, object *env);
object *eval (object *form, object *env);
object *read (gfun_t gfun);
void repl (object *env);
void printobject (object *form, pfun_t pfun);
char *lookupbuiltin (symbol_t name);
intptr_t lookupfn (symbol_t name);
int builtin (char* n);
char *symbolname (symbol_t x);
object *apply (symbol_t name, object *function, object *args, object *env);
char *lookupsymbol (symbol_t name);
char *cstring (object *form, char *buffer, int buflen);
object *edit (object *fun);

// Error handling

void errorsub (symbol_t fname, PGM_P string) {
  pfl(pserial); pfstring(PSTR("Error: "), pserial);
  if (fname) {
    pserial('\'');
    pstring(symbolname(fname), pserial);
    pserial('\''); pserial(' ');
  }
  pfstring(string, pserial);
}

void error (symbol_t fname, PGM_P string, object *symbol) {
  errorsub(fname, string);
  pserial(':'); pserial(' ');
  printobject(symbol, pserial);
  pln(pserial);
  GCStack = NULL;
  longjmp(exception, 1);
}

void error2 (symbol_t fname, PGM_P string) {
  errorsub(fname, string);
  pln(pserial);
  GCStack = NULL;
  longjmp(exception, 1);
}

// Save space as these are used multiple times
const char notanumber[] PROGMEM = "argument is not a number";
const char notaninteger[] PROGMEM = "argument is not an integer";
const char notastring[] PROGMEM = "argument is not a string";
const char notalist[] PROGMEM = "argument is not a list";
const char notasymbol[] PROGMEM = "argument is not a symbol";
const char notproper[] PROGMEM = "argument is not a proper list";
const char toomanyargs[] PROGMEM = "too many arguments";
const char toofewargs[] PROGMEM = "too few arguments";
const char noargument[] PROGMEM = "missing argument";
const char nostream[] PROGMEM = "missing stream argument";
const char overflow[] PROGMEM = "arithmetic overflow";
const char indexnegative[] PROGMEM = "index can't be negative";
const char invalidarg[] PROGMEM = "invalid argument";
const char invalidkey[] PROGMEM = "invalid keyword";
const char invalidpin[] PROGMEM = "invalid pin";
const char oddargs[] PROGMEM = "odd number of arguments";

// Set up workspace

void initworkspace () {
  Freelist = NULL;
  for (int i=WORKSPACESIZE-1; i>=0; i--) {
    object *obj = &Workspace[i];
    car(obj) = NULL;
    cdr(obj) = Freelist;
    Freelist = obj;
    Freespace++;
  }
}

object *myalloc () {
  if (Freespace == 0) error2(0, PSTR("no room"));
  object *temp = Freelist;
  Freelist = cdr(Freelist);
  Freespace--;
  return temp;
}

inline void myfree (object *obj) {
  car(obj) = NULL;
  cdr(obj) = Freelist;
  Freelist = obj;
  Freespace++;
}

// Make each type of object

object *number (int n) {
  object *ptr = myalloc();
  ptr->type = NUMBER;
  ptr->integer = n;
  return ptr;
}

object *makefloat (float f) {
  object *ptr = myalloc();
  ptr->type = FLOAT;
  ptr->single_float = f;
  return ptr;
}

object *character (uint8_t c) {
  object *ptr = myalloc();
  ptr->type = CHARACTER;
  ptr->chars = c;
  return ptr;
}

object *cons (object *arg1, object *arg2) {
  object *ptr = myalloc();
  ptr->car = arg1;
  ptr->cdr = arg2;
  return ptr;
}

object *symbol (symbol_t name) {
  object *ptr = myalloc();
  ptr->type = SYMBOL;
  ptr->name = name;
  return ptr;
}

object *newsymbol (symbol_t name) {
  for (int i=0; i<WORKSPACESIZE; i++) {
    object *obj = &Workspace[i];
    if (symbolp(obj) && obj->name == name) return obj;
  }
  return symbol(name);
}

object *stream (uint8_t streamtype, uint8_t address) {
  object *ptr = myalloc();
  ptr->type = STREAM;
  ptr->integer = streamtype<<8 | address;
  return ptr;
}

// Garbage collection

void markobject (object *obj) {
  MARK:
  if (obj == NULL) return;
  if (marked(obj)) return;

  object* arg = car(obj);
  unsigned int type = obj->type;
  mark(obj);

  if (type >= PAIR || type == ZZERO) { // cons
    markobject(arg);
    obj = cdr(obj);
    goto MARK;
  }

  if (type == ARRAY) {
    obj = cdr(obj);
    goto MARK;
  }

  if (type == STRING) {
    obj = cdr(obj);
    while (obj != NULL) {
      arg = car(obj);
      mark(obj);
      obj = arg;
    }
  }
}

void sweep () {
  Freelist = NULL;
  Freespace = 0;
  for (int i=WORKSPACESIZE-1; i>=0; i--) {
    object *obj = &Workspace[i];
    if (!marked(obj)) myfree(obj); else unmark(obj);
  }
}

void gc (object *form, object *env) {
  #if defined(printgcs)
  int start = Freespace;
  #endif
  markobject(tee);
  markobject(GlobalEnv);
  markobject(GCStack);
  markobject(form);
  markobject(env);
  sweep();
  #if defined(printgcs)
  pfl(pserial); pserial('{'); pint(Freespace - start, pserial); pserial('}');
  #endif
}

// Compact image

void movepointer (object *from, object *to) {
  for (int i=0; i<WORKSPACESIZE; i++) {
    object *obj = &Workspace[i];
    unsigned int type = (obj->type) & ~MARKBIT;
    if (marked(obj) && (type >= ARRAY || type==ZZERO)) {
      if (car(obj) == (object *)((uintptr_t)from | MARKBIT))
        car(obj) = (object *)((uintptr_t)to | MARKBIT);
      if (cdr(obj) == from) cdr(obj) = to;
    }
  }
  // Fix strings
  for (int i=0; i<WORKSPACESIZE; i++) {
    object *obj = &Workspace[i];
    if (marked(obj) && ((obj->type) & ~MARKBIT) == STRING) {
      obj = cdr(obj);
      while (obj != NULL) {
        if (cdr(obj) == to) cdr(obj) = from;
        obj = (object *)((uintptr_t)(car(obj)) & ~MARKBIT);
      }
    }
  }
}

uintptr_t compactimage (object **arg) {
  markobject(tee);
  markobject(GlobalEnv);
  markobject(GCStack);
  object *firstfree = Workspace;
  while (marked(firstfree)) firstfree++;
  object *obj = &Workspace[WORKSPACESIZE-1];
  while (firstfree < obj) {
    if (marked(obj)) {
      car(firstfree) = car(obj);
      cdr(firstfree) = cdr(obj);
      unmark(obj);
      movepointer(obj, firstfree);
      if (GlobalEnv == obj) GlobalEnv = firstfree;
      if (GCStack == obj) GCStack = firstfree;
      if (*arg == obj) *arg = firstfree;
      while (marked(firstfree)) firstfree++;
    }
    obj--;
  }
  sweep();
  return firstfree - Workspace;
}

// Make SD card filename

char *MakeFilename (object *arg) {
  char *buffer = SymbolTop;
  int max = maxbuffer(buffer);
  buffer[0]='/';
  int i = 1;
  do {
    char c = nthchar(arg, i-1);
    if (c == '\0') break;
    buffer[i++] = c;
  } while (i<max);
  buffer[i] = '\0';
  return buffer;
}

// Save-image and load-image

#if defined(sdcardsupport)
void SDWriteInt (File file, int data) {
  file.write(data & 0xFF); file.write(data>>8 & 0xFF);
  file.write(data>>16 & 0xFF); file.write(data>>24 & 0xFF);
}
#else
void EpromWriteInt(int *addr, uintptr_t data) {
  EEPROM.write((*addr)++, data & 0xFF); EEPROM.write((*addr)++, data>>8 & 0xFF);
  EEPROM.write((*addr)++, data>>16 & 0xFF); EEPROM.write((*addr)++, data>>24 & 0xFF);
}
#endif

unsigned int saveimage (object *arg) {
  unsigned int imagesize = compactimage(&arg);
#if defined(sdcardsupport)
  SD.begin(SDCARD_SS_PIN);
  File file;
  if (stringp(arg)) {
    file = SD.open(MakeFilename(arg), FILE_WRITE);
    arg = NULL;
  } else if (arg == NULL || listp(arg)) file = SD.open("/ULISP.IMG", FILE_WRITE);
  else error(SAVEIMAGE, PSTR("illegal argument"), arg);
  if (!file) error2(SAVEIMAGE, PSTR("problem saving to SD card"));
  SDWriteInt(file, (uintptr_t)arg);
  SDWriteInt(file, imagesize);
  SDWriteInt(file, (uintptr_t)GlobalEnv);
  SDWriteInt(file, (uintptr_t)GCStack);
  #if SYMBOLTABLESIZE > BUFFERSIZE
  SDWriteInt(file, (uintptr_t)SymbolTop);
  for (int i=0; i<SYMBOLTABLESIZE; i++) file.write(SymbolTable[i]);
  #endif
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    SDWriteInt(file, (uintptr_t)car(obj));
    SDWriteInt(file, (uintptr_t)cdr(obj));
  }
  file.close();
  return imagesize;
#else
  if (!(arg == NULL || listp(arg))) error(SAVEIMAGE, PSTR("illegal argument"), arg);
  int bytesneeded = imagesize*8 + SYMBOLTABLESIZE + 36;
  if (bytesneeded > EEPROMSIZE) error(SAVEIMAGE, PSTR("image too large"), number(imagesize));
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  EpromWriteInt(&addr, (uintptr_t)arg);
  EpromWriteInt(&addr, imagesize);
  EpromWriteInt(&addr, (uintptr_t)GlobalEnv);
  EpromWriteInt(&addr, (uintptr_t)GCStack);
  #if SYMBOLTABLESIZE > BUFFERSIZE
  EpromWriteInt(&addr, (uintptr_t)SymbolTop);
  for (int i=0; i<SYMBOLTABLESIZE; i++) EEPROM.write(addr++, SymbolTable[i]);
  #endif
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    EpromWriteInt(&addr, (uintptr_t)car(obj));
    EpromWriteInt(&addr, (uintptr_t)cdr(obj));
  }
  EEPROM.commit();
  return imagesize;
#endif
}

#if defined(sdcardsupport)
int SDReadInt (File file) {
  uintptr_t b0 = file.read(); uintptr_t b1 = file.read();
  uintptr_t b2 = file.read(); uintptr_t b3 = file.read();
  return b0 | b1<<8 | b2<<16 | b3<<24;
}
#else
int EpromReadInt (int *addr) {
  uint8_t b0 = EEPROM.read((*addr)++); uint8_t b1 = EEPROM.read((*addr)++);
  uint8_t b2 = EEPROM.read((*addr)++); uint8_t b3 = EEPROM.read((*addr)++);
  return b0 | b1<<8 | b2<<16 | b3<<24;
}
#endif

unsigned int loadimage (object *arg) {
#if defined(sdcardsupport)
  SD.begin(SDCARD_SS_PIN);
  File file;
  if (stringp(arg)) file = SD.open(MakeFilename(arg));
  else if (arg == NULL) file = SD.open("/ULISP.IMG");
  else error(LOADIMAGE, PSTR("illegal argument"), arg);
  if (!file) error2(LOADIMAGE, PSTR("problem loading from SD card"));
  SDReadInt(file);
  unsigned int imagesize = SDReadInt(file);
  GlobalEnv = (object *)SDReadInt(file);
  GCStack = (object *)SDReadInt(file);
  #if SYMBOLTABLESIZE > BUFFERSIZE
  SymbolTop = (char *)SDReadInt(file);
  for (int i=0; i<SYMBOLTABLESIZE; i++) SymbolTable[i] = file.read();
  #endif
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    car(obj) = (object *)SDReadInt(file);
    cdr(obj) = (object *)SDReadInt(file);
  }
  file.close();
  gc(NULL, NULL);
  return imagesize;
#else
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  EpromReadInt(&addr); // Skip eval address
  unsigned int imagesize = EpromReadInt(&addr);
  if (imagesize == 0 || imagesize == 0xFFFFFFFF) error2(LOADIMAGE, PSTR("no saved image"));
  GlobalEnv = (object *)EpromReadInt(&addr);
  GCStack = (object *)EpromReadInt(&addr);
  #if SYMBOLTABLESIZE > BUFFERSIZE
  SymbolTop = (char *)EpromReadInt(&addr);
  for (int i=0; i<SYMBOLTABLESIZE; i++) SymbolTable[i] = EEPROM.read(addr++);
  #endif
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    car(obj) = (object *)EpromReadInt(&addr);
    cdr(obj) = (object *)EpromReadInt(&addr);
  }
  gc(NULL, NULL);
  return imagesize;
#endif
}

void autorunimage () {
#if defined(sdcardsupport)
  SD.begin(SDCARD_SS_PIN);
  File file = SD.open("/ULISP.IMG");
  if (!file) error2(0, PSTR("problem autorunning from SD card"));
  object *autorun = (object *)SDReadInt(file);
  file.close();
  if (autorun != NULL) {
    loadimage(NULL);
    apply(0, autorun, NULL, NULL);
  }
#else
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  object *autorun = (object *)EpromReadInt(&addr);
  if (autorun != NULL && (unsigned int)autorun != 0xFFFF) {
    loadimage(NULL);
    apply(0, autorun, NULL, NULL);
  }
#endif
}

// Tracing

bool tracing (symbol_t name) {
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == name) return i+1;
    i++;
  }
  return 0;
}

void trace (symbol_t name) {
  if (tracing(name)) error(TRACE, PSTR("already being traced"), symbol(name));
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == 0) { TraceFn[i] = name; TraceDepth[i] = 0; return; }
    i++;
  }
  error2(TRACE, PSTR("already tracing 3 functions"));
}

void untrace (symbol_t name) {
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == name) { TraceFn[i] = 0; return; }
    i++;
  }
  error(UNTRACE, PSTR("not tracing"), symbol(name));
}

// Helper functions

bool consp (object *x) {
  if (x == NULL) return false;
  unsigned int type = x->type;
  return type >= PAIR || type == ZZERO;
}

#define atom(x) (!consp(x))

bool listp (object *x) {
  if (x == NULL) return true;
  unsigned int type = x->type;
  return type >= PAIR || type == ZZERO;
}

#define improperp(x) (!listp(x))

object *quote (object *arg) {
  return cons(symbol(QUOTE), cons(arg,NULL));
}

// Radix 40 encoding

#define MAXSYMBOL 4096000000

int toradix40 (char ch) {
  if (ch == 0) return 0;
  if (ch >= '0' && ch <= '9') return ch-'0'+30;
  if (ch == '$') return 27; if (ch == '*') return 28; if (ch == '-') return 29;
  ch = ch | 0x20;
  if (ch >= 'a' && ch <= 'z') return ch-'a'+1;
  return -1; // Invalid
}

int fromradix40 (int n) {
  if (n >= 1 && n <= 26) return 'a'+n-1;
  if (n == 27) return '$'; if (n == 28) return '*'; if (n == 29) return '-';
  if (n >= 30 && n <= 39) return '0'+n-30;
  return 0;
}

int pack40 (char *buffer) {
  int x = 0;
  for (int i=0; i<6; i++) x = x * 40 + toradix40(buffer[i]);
  return x;
}

bool valid40 (char *buffer) {
  for (int i=0; i<6; i++) if (toradix40(buffer[i]) == -1) return false;
  return true;
}

char *symbolname (symbol_t x) {
  if (x < ENDFUNCTIONS) return lookupbuiltin(x);
  else if (x >= MAXSYMBOL) return lookupsymbol(x);
  char *buffer = SymbolTop;
  buffer[3] = '\0'; buffer[4] = '\0'; buffer[5] = '\0'; buffer[6] = '\0';
  for (int n=5; n>=0; n--) {
    buffer[n] = fromradix40(x % 40);
    x = x / 40;
  }
  return buffer;
}

int digitvalue (char d) {
  if (d>='0' && d<='9') return d-'0';
  d = d | 0x20;
  if (d>='a' && d<='f') return d-'a'+10;
  return 16;
}

int checkinteger (symbol_t name, object *obj) {
  if (!integerp(obj)) error(name, notaninteger, obj);
  return obj->integer;
}

int checkbitvalue (symbol_t name, object *obj) {
  if (!integerp(obj)) error(name, notaninteger, obj);
  int n = obj->integer;
  if (n & ~1) error(name, PSTR("argument is not a bit value"), obj);
  return n;
}

float checkintfloat (symbol_t name, object *obj){
  if (integerp(obj)) return obj->integer;
  if (!floatp(obj)) error(name, notanumber, obj);
  return obj->single_float;
}

int checkchar (symbol_t name, object *obj) {
  if (!characterp(obj)) error(name, PSTR("argument is not a character"), obj);
  return obj->chars;
}

int isstream (object *obj){
  if (!streamp(obj)) error(0, PSTR("not a stream"), obj);
  return obj->integer;
}

int issymbol (object *obj, symbol_t n) {
  return symbolp(obj) && obj->name == n;
}

int keywordp (object *obj) {
  if (!symbolp(obj)) return false;
  symbol_t name = obj->name;
  return ((name > KEYWORDS) && (name < USERFUNCTIONS));
}

int checkkeyword (symbol_t name, object *obj) {
  if (!keywordp(obj)) error(name, PSTR("argument is not a keyword"), obj);
  symbol_t kname = obj->name;
  uint8_t context = getminmax(kname);
  if (context != 0 && context != name) error(name, invalidkey, obj);
  return ((int)lookupfn(kname));
}

void checkargs (symbol_t name, object *args) {
  int nargs = listlength(name, args);
  if (name >= ENDFUNCTIONS) error(0, PSTR("not valid here"), symbol(name));
  checkminmax(name, nargs);
}

int eq (object *arg1, object *arg2) {
  if (arg1 == arg2) return true;  // Same object
  if ((arg1 == nil) || (arg2 == nil)) return false;  // Not both values
  if (arg1->cdr != arg2->cdr) return false;  // Different values
  if (symbolp(arg1) && symbolp(arg2)) return true;  // Same symbol
  if (integerp(arg1) && integerp(arg2)) return true;  // Same integer
  if (floatp(arg1) && floatp(arg2)) return true; // Same float
  if (characterp(arg1) && characterp(arg2)) return true;  // Same character
  return false;
}

int listlength (symbol_t name, object *list) {
  int length = 0;
  while (list != NULL) {
    if (improperp(list)) error2(name, notproper);
    list = cdr(list);
    length++;
  }
  return length;
}

// Association lists

object *assoc (object *key, object *list) {
  while (list != NULL) {
    if (improperp(list)) error(ASSOC, notproper, list);
    object *pair = first(list);
    if (!listp(pair)) error(ASSOC, PSTR("element is not a list"), pair);
    if (pair != NULL && eq(key,car(pair))) return pair;
    list = cdr(list);
  }
  return nil;
}

object *delassoc (object *key, object **alist) {
  object *list = *alist;
  object *prev = NULL;
  while (list != NULL) {
    object *pair = first(list);
    if (eq(key,car(pair))) {
      if (prev == NULL) *alist = cdr(list);
      else cdr(prev) = cdr(list);
      return key;
    }
    prev = list;
    list = cdr(list);
  }
  return nil;
}

// Array utilities

int nextpower2 (int n) {
  n--; n |= n >> 1; n |= n >> 2; n |= n >> 4;
  n |= n >> 8; n |= n >> 16; n++;
  return n<2 ? 2 : n;
}

object *buildarray (int n, int s, object *def) {
  int s2 = s>>1;
  if (s2 == 1) {
    if (n == 2) return cons(def, def);
    else if (n == 1) return cons(def, NULL);
    else return NULL;
  } else if (n >= s2) return cons(buildarray(s2, s2, def), buildarray(n - s2, s2, def));
  else return cons(buildarray(n, s2, def), nil);
}

object *makearray (symbol_t name, object *dims, object *def, bool bitp) {
  int size = 1;
  object *dimensions = dims;
  while (dims != NULL) {
    int d = car(dims)->integer;
    if (d < 0) error2(MAKEARRAY, PSTR("dimension can't be negative"));
    size = size * d;
    dims = cdr(dims);
  }
  // Bit array identified by making first dimension negative
  if (bitp) { size = (size + 31)/32; car(dimensions) = number(-(car(dimensions)->integer)); }
  object *ptr = myalloc();
  ptr->type = ARRAY;
  object *tree = nil;
  if (size != 0) tree = buildarray(size, nextpower2(size), def);
  ptr->cdr = cons(tree, dimensions);
  return ptr;
}

object **arrayref (object *array, int index, int size) {
  int mask = nextpower2(size)>>1;
  object **p = &car(cdr(array));
  while (mask) {
    if ((index & mask) == 0) p = &(car(*p)); else p = &(cdr(*p));
    mask = mask>>1;
  }
  return p;
}

object **getarray (symbol_t name, object *array, object *subs, object *env, int *bit) {
  int index = 0, size = 1, s;
  *bit = -1;
  bool bitp = false;
  object *dims = cddr(array);
  while (dims != NULL && subs != NULL) {
    int d = car(dims)->integer;
    if (d < 0) { d = -d; bitp = true; }
    if (env) s = checkinteger(name, eval(car(subs), env)); else s = checkinteger(name, car(subs));
    if (s < 0 || s >= d) error(name, PSTR("subscript out of range"), car(subs));
    size = size * d;
    index = index * d + s;
    dims = cdr(dims); subs = cdr(subs);
  }
  if (dims != NULL) error2(name, PSTR("too few subscripts"));
  if (subs != NULL) error2(name, PSTR("too many subscripts"));
  if (bitp) {
    size = (size + 31)/32;
    *bit = index & 0x1F; index = index>>5;
  }
  return arrayref(array, index, size);
}

void rslice (object *array, int size, int slice, object *dims, object *args) {
  int d = first(dims)->integer;
  for (int i = 0; i < d; i++) {
    int index = slice * d + i;
    if (!consp(args)) error2(0, PSTR("initial contents don't match array type"));
    if (cdr(dims) == NULL) {
      object **p = arrayref(array, index, size);
      *p = car(args);
    } else rslice(array, size, index, cdr(dims), car(args));
    args = cdr(args);
  }
}

object *readarray (int d, object *args) {
  object *list = args;
  object *dims = NULL; object *head = NULL;
  int size = 1;
  for (int i = 0; i < d; i++) {
    if (!listp(list)) error2(0, PSTR("initial contents don't match array type"));
    int l = listlength(0, list);
    if (dims == NULL) { dims = cons(number(l), NULL); head = dims; }
    else { cdr(dims) = cons(number(l), NULL); dims = cdr(dims); }
    size = size * l;
    if (list != NULL) list = car(list); 
  }
  object *array = makearray(0, head, NULL, false);
  rslice(array, size, 0, head, args);
  return array;
}

object *readbitarray (gfun_t gfun) {
  char ch = gfun();
  object *head = NULL;
  object *tail = NULL;
  while (!issp(ch) && ch != ')' && ch != '(') {
    if (ch != '0' && ch != '1') error2(0, PSTR("illegal character in bit array"));
    object *cell = cons(number(ch - '0'), NULL);
    if (head == NULL) head = cell;
    else tail->cdr = cell;
    tail = cell;
    ch = gfun();
  }
  LastChar = ch;
  int size = listlength(0, head);
  object *array = makearray(0, cons(number(size), NULL), 0, true);
  size = (size + 31) / 32;
  int index = 0;
  while (head != NULL) {
    object **loc = arrayref(array, index>>5, size);
    int bit = index & 0x1F;
    *loc = number((((*loc)->integer) & ~(1<<bit)) | (car(head)->integer)<<bit);
    index++;
    head = cdr(head);
  }
  return array;
}

void pslice (object *array, int size, int slice, object *dims, pfun_t pfun, bool bitp) {
  bool spaces = true;
  if (slice == -1) { spaces = false; slice = 0; }
  int d = first(dims)->integer;
  if (d < 0) d = -d;
  for (int i = 0; i < d; i++) {
    if (i && spaces) pfun(' ');
    int index = slice * d + i;
    if (cdr(dims) == NULL) {
      if (bitp) pint(((*arrayref(array, index>>5, size))->integer)>>(index & 0x1f) & 1, pfun);
      else printobject(*arrayref(array, index, size), pfun);
    } else { pfun('('); pslice(array, size, index, cdr(dims), pfun, bitp); pfun(')'); }
  }
}

void printarray (object *array, pfun_t pfun) {
  object *dimensions = cddr(array);
  object *dims = dimensions;
  bool bitp = false;
  int size = 1, n = 0;
  while (dims != NULL) {
    int d = car(dims)->integer;
    if (d < 0) { bitp = true; d = -d; }
    size = size * d;
    dims = cdr(dims); n++;
  }
  if (bitp) size = (size+31)/32;
  pfun('#');
  if (n == 1 && bitp) { pfun('*'); pslice(array, size, -1, dimensions, pfun, bitp); }
  else {
    if (n > 1) { pint(n, pfun); pfun('A'); }
    pfun('('); pslice(array, size, 0, dimensions, pfun, bitp); pfun(')');
  }
}

// String utilities

void indent (uint8_t spaces, char ch, pfun_t pfun) {
  for (uint8_t i=0; i<spaces; i++) pfun(ch);
}

object *startstring (symbol_t name) {
  object *string = myalloc();
  string->type = STRING;
  GlobalString = NULL;
  GlobalStringIndex = 0;
  return string;
}

void buildstring (uint8_t ch, int *chars, object **head) {
  static object* tail;
  static uint8_t shift;
  if (*chars == 0) {
    shift = (sizeof(int)-1)*8;
    *chars = ch<<shift;
    object *cell = myalloc();
    if (*head == NULL) *head = cell; else tail->car = cell;
    cell->car = NULL;
    cell->chars = *chars;
    tail = cell;
  } else {
    shift = shift - 8;
    *chars = *chars | ch<<shift;
    tail->chars = *chars;
    if (shift == 0) *chars = 0;
  }
}

object *readstring (uint8_t delim, gfun_t gfun) {
  object *obj = myalloc();
  obj->type = STRING;
  int ch = gfun();
  if (ch == -1) return nil;
  object *head = NULL;
  int chars = 0;
  while ((ch != delim) && (ch != -1)) {
    if (ch == '\\') ch = gfun();
    buildstring(ch, &chars, &head);
    ch = gfun();
  }
  obj->cdr = head;
  return obj;
}

int stringlength (object *form) {
  int length = 0;
  form = cdr(form);
  while (form != NULL) {
    int chars = form->chars;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      if (chars>>i & 0xFF) length++;
    }
    form = car(form);
  }
  return length;
}

uint8_t nthchar (object *string, int n) {
  object *arg = cdr(string);
  int top;
  if (sizeof(int) == 4) { top = n>>2; n = 3 - (n&3); }
  else { top = n>>1; n = 1 - (n&1); }
  for (int i=0; i<top; i++) {
    if (arg == NULL) return 0;
    arg = car(arg);
  }
  if (arg == NULL) return 0;
  return (arg->chars)>>(n*8) & 0xFF;
}

int gstr () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
  char c = nthchar(GlobalString, GlobalStringIndex++);
  if (c != 0) return c;
  return '\n'; // -1?
}

void pstr (char c) {
  buildstring(c, &GlobalStringIndex, &GlobalString);
}

char *cstringbuf (object *arg) {
  cstring(arg, SymbolTop, SYMBOLTABLESIZE-(SymbolTop-SymbolTable));
  return SymbolTop;
}

char *cstring (object *form, char *buffer, int buflen) {
  int index = 0;
  form = cdr(form);
  while (form != NULL) {
    int chars = form->integer;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      char ch = chars>>i & 0xFF;
      if (ch) {
        if (index >= buflen-1) error2(0, PSTR("no room for string"));
        buffer[index++] = ch;
      }
    }
    form = car(form);
  }
  buffer[index] = '\0';
  return buffer;
}

object *lispstring (char *s) {
  object *obj = myalloc();
  obj->type = STRING;
  char ch = *s++;
  object *head = NULL;
  int chars = 0;
  while (ch) {
    if (ch == '\\') ch = *s++;
    buildstring(ch, &chars, &head);
    ch = *s++;
  }
  obj->cdr = head;
  return obj;
}

// Lookup variable in environment

object *value (symbol_t n, object *env) {
  while (env != NULL) {
    object *pair = car(env);
    if (pair != NULL && car(pair)->name == n) return pair;
    env = cdr(env);
  }
  return nil;
}

bool boundp (object *var, object *env) {
  symbol_t varname = var->name;
  if (value(varname, env) != NULL) return true;
  if (value(varname, GlobalEnv) != NULL) return true;
  return false;
}

object *findvalue (object *var, object *env) {
  symbol_t varname = var->name;
  object *pair = value(varname, env);
  if (pair == NULL) pair = value(varname, GlobalEnv);
  if (pair == NULL) error(0, PSTR("unknown variable"), var);
  return pair;
}

// Handling closures

object *closure (int tc, symbol_t name, object *state, object *function, object *args, object **env) {
  int trace = 0;
  if (name) trace = tracing(name);
  if (trace) {
    indent(TraceDepth[trace-1]<<1, ' ', pserial);
    pint(TraceDepth[trace-1]++, pserial);
    pserial(':'); pserial(' '); pserial('('); pstring(symbolname(name), pserial);
  }
  object *params = first(function);
  if (!listp(params)) error(name, notalist, params);
  function = cdr(function);
  // Dropframe
  if (tc) {
    if (*env != NULL && car(*env) == NULL) {
      pop(*env);
      while (*env != NULL && car(*env) != NULL) pop(*env);
    } else push(nil, *env);
  }
  // Push state
  while (state != NULL) {
    object *pair = first(state);
    push(pair, *env);
    state = cdr(state);
  }
  // Add arguments to environment
  bool optional = false;
  while (params != NULL) {
    object *value;
    object *var = first(params);
    if (symbolp(var) && var->name == OPTIONAL) optional = true;
    else {
      if (consp(var)) {
        if (!optional) error(name, PSTR("invalid default value"), var);
        if (args == NULL) value = eval(second(var), *env);
        else { value = first(args); args = cdr(args); }
        var = first(var);
        if (!symbolp(var)) error(name, PSTR("illegal optional parameter"), var);
      } else if (!symbolp(var)) {
        error2(name, PSTR("illegal function parameter"));
      } else if (var->name == AMPREST) {
        params = cdr(params);
        var = first(params);
        value = args;
        args = NULL;
      } else {
        if (args == NULL) {
          if (optional) value = nil; 
          else error2(name, toofewargs);
        } else { value = first(args); args = cdr(args); }
      }
      push(cons(var,value), *env);
      if (trace) { pserial(' '); printobject(value, pserial); }
    }
    params = cdr(params);
  }
  if (args != NULL) error2(name, toomanyargs);
  if (trace) { pserial(')'); pln(pserial); }
  // Do an implicit progn
  if (tc) push(nil, *env);
  return tf_progn(function, *env);
}

object *apply (symbol_t name, object *function, object *args, object *env) {
  if (symbolp(function)) {
    symbol_t fname = function->name;
    if ((fname > FUNCTIONS) && (fname < KEYWORDS)) {
      checkargs(fname, args);
      return ((fn_ptr_type)lookupfn(fname))(args, env);
    } else function = eval(function, env);
  }
  if (consp(function) && issymbol(car(function), LAMBDA)) {
    function = cdr(function);
    object *result = closure(0, name, NULL, function, args, &env);
    return eval(result, env);
  }
  if (consp(function) && issymbol(car(function), CLOSURE)) {
    function = cdr(function);
    object *result = closure(0, name, car(function), cdr(function), args, &env);
    return eval(result, env);
  }
  error(name, PSTR("illegal function"), function);
  return NULL;
}

// In-place operations

object **place (symbol_t name, object *args, object *env, int *bit) {
  *bit = -1;
  if (atom(args)) return &cdr(findvalue(args, env));
  object* function = first(args);
  if (symbolp(function)) {
    symbol_t fname = function->name;
    if (fname == CAR || fname == FIRST) {
      object *value = eval(second(args), env);
      if (!listp(value)) error(name, PSTR("can't take car"), value);
      return &car(value);
    }
    if (fname == CDR || fname == REST) {
      object *value = eval(second(args), env);
      if (!listp(value)) error(name, PSTR("can't take cdr"), value);
      return &cdr(value);
    }
    if (fname == NTH) {
      int index = checkinteger(NTH, eval(second(args), env));
      object *list = eval(third(args), env);
      if (atom(list)) error(name, PSTR("second argument to nth is not a list"), list);
      while (index > 0) {
        list = cdr(list);
        if (list == NULL) error2(name, PSTR("index to nth is out of range"));
        index--;
      }
      return &car(list);
    }
    if (fname == AREF) {
      object *array = eval(second(args), env);
      if (!arrayp(array)) error(AREF, PSTR("first argument is not an array"), array);
      return getarray(AREF, array, cddr(args), env, bit);
    }
  }
  error2(name, PSTR("illegal place"));
  return nil;
}

// Checked car and cdr

object *carx (object *arg) {
  if (!listp(arg)) error(0, PSTR("can't take car"), arg);
  if (arg == nil) return nil;
  return car(arg);
}

object *cdrx (object *arg) {
  if (!listp(arg)) error(0, PSTR("can't take cdr"), arg);
  if (arg == nil) return nil;
  return cdr(arg);
}

// I2C interface

void I2Cinit (bool enablePullup) {
  (void) enablePullup;
  Wire.begin();
}

int I2Cread () {
  return Wire.read();
}

void I2Cwrite (uint8_t data) {
  Wire.write(data);
}

bool I2Cstart (uint8_t address, uint8_t read) {
 int ok = true;
 if (read == 0) {
   Wire.beginTransmission(address);
   ok = (Wire.endTransmission(true) == 0);
   Wire.beginTransmission(address);
 }
 else Wire.requestFrom(address, I2CCount);
 return ok;
}

bool I2Crestart (uint8_t address, uint8_t read) {
  int error = (Wire.endTransmission(false) != 0);
  if (read == 0) Wire.beginTransmission(address);
  else Wire.requestFrom(address, I2CCount);
  return error ? false : true;
}

void I2Cstop (uint8_t read) {
  if (read == 0) Wire.endTransmission(); // Check for error?
}

// Streams

inline int spiread () { return SPI.transfer(0); }
inline int serial1read () { while (!Serial1.available()) testescape(); return Serial1.read(); }
#if defined(sdcardsupport)
File SDpfile, SDgfile;
inline int SDread () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
  return SDgfile.read();
}
#endif

WiFiClient client;
WiFiServer server(80);

inline int WiFiread () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
  return client.read();
}

void serialbegin (int address, int baud) {
  if (address == 1) Serial1.begin((long)baud*100);
  else error(WITHSERIAL, PSTR("port not supported"), number(address));
}

void serialend (int address) {
  if (address == 1) {Serial1.flush(); Serial1.end(); }
}

gfun_t gstreamfun (object *args) {
  int streamtype = SERIALSTREAM;
  int address = 0;
  gfun_t gfun = gserial;
  if (args != NULL) {
    int stream = isstream(first(args));
    streamtype = stream>>8; address = stream & 0xFF;
  }
  if (streamtype == I2CSTREAM) gfun = (gfun_t)I2Cread;
  else if (streamtype == SPISTREAM) gfun = spiread;
  else if (streamtype == SERIALSTREAM) {
    if (address == 0) gfun = gserial;
    else if (address == 1) gfun = serial1read;
  }
  #if defined(sdcardsupport)
  else if (streamtype == SDSTREAM) gfun = (gfun_t)SDread;
  #endif
  else if (streamtype == WIFISTREAM) gfun = (gfun_t)WiFiread;
  else error2(0, PSTR("unknown stream type"));
  return gfun;
}

inline void spiwrite (char c) { SPI.transfer(c); }
inline void serial1write (char c) { Serial1.write(c); }
inline void WiFiwrite (char c) { client.write(c); }
#if defined(sdcardsupport)
inline void SDwrite (char c) { SDpfile.write(c); }
#endif
#if defined(gfxsupport)
inline void gfxwrite (char c) { tft.write(c); tft.display(); }
#endif

pfun_t pstreamfun (object *args) {
  int streamtype = SERIALSTREAM;
  int address = 0;
  pfun_t pfun = pserial;
  if (args != NULL && first(args) != NULL) {
    int stream = isstream(first(args));
    streamtype = stream>>8; address = stream & 0xFF;
  }
  if (streamtype == I2CSTREAM) pfun = (pfun_t)I2Cwrite;
  else if (streamtype == SPISTREAM) pfun = spiwrite;
  else if (streamtype == SERIALSTREAM) {
    if (address == 0) pfun = pserial;
    else if (address == 1) pfun = serial1write;
  }
  else if (streamtype == STRINGSTREAM) {
    pfun = pstr;
  }
  #if defined(sdcardsupport)
  else if (streamtype == SDSTREAM) pfun = (pfun_t)SDwrite;
  #endif
  #if defined(gfxsupport)
  else if (streamtype == GFXSTREAM) pfun = (pfun_t)gfxwrite;
  #endif
  else if (streamtype == WIFISTREAM) pfun = (pfun_t)WiFiwrite;
  else error2(0, PSTR("unknown stream type"));
  return pfun;
}

// Check pins

void checkanalogread (int pin) {
#if defined(ESP8266)
  if (pin!=17) error(ANALOGREAD, PSTR("invalid pin"), number(pin));
#elif defined(ESP32)
  if (!(pin==0 || pin==2 || pin==4 || (pin>=12 && pin<=15) || (pin>=25 && pin<=27) || (pin>=32 && pin<=36) || pin==39))
    error(ANALOGREAD, PSTR("invalid pin"), number(pin));
#endif
}

void checkanalogwrite (int pin) {
#if defined(ESP8266)
  if (!(pin>=0 && pin<=16)) error(ANALOGWRITE, PSTR("invalid pin"), number(pin));
#elif defined(ESP32)
  if (!(pin>=25 && pin<=26)) error(ANALOGWRITE, PSTR("invalid pin"), number(pin));
#endif
}

// Note

void tone (int pin, int note) {
  (void) pin, (void) note;
}

void noTone (int pin) {
  (void) pin;
}

const int scale[] PROGMEM = {4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902};

void playnote (int pin, int note, int octave) {
  int prescaler = 8 - octave - note/12;
  if (prescaler<0 || prescaler>8) error(NOTE, PSTR("octave out of range"), number(prescaler));
  tone(pin, pgm_read_word(&scale[note%12])>>prescaler);
}

void nonote (int pin) {
  noTone(pin);
}

// Sleep

void initsleep () { }

void sleep (int secs) {
  delay(1000 * secs);
}

// Prettyprint

const int PPINDENT = 2;
const int PPWIDTH = 80;
const int GFXPPWIDTH = 52; // 320 pixel wide screen
int ppwidth = PPWIDTH;

void pcount (char c) {
  if (c == '\n') PrintCount++;
  PrintCount++;
}

uint8_t atomwidth (object *obj) {
  PrintCount = 0;
  printobject(obj, pcount);
  return PrintCount;
}

uint8_t basewidth (object *obj, uint8_t power2) {
  PrintCount = 0;
  pintbase(obj->integer, power2, pcount);
  return PrintCount;
}

boolean quoted (object *obj) {
  return (consp(obj) && car(obj) != NULL && car(obj)->name == QUOTE && consp(cdr(obj)) && cddr(obj) == NULL);
}

int subwidth (object *obj, int w) {
  if (atom(obj)) return w - atomwidth(obj);
  if (quoted(obj)) return subwidthlist(car(cdr(obj)), w - 1);
  return subwidthlist(obj, w - 1);
}

int subwidthlist (object *form, int w) {
  while (form != NULL && w >= 0) {
    if (atom(form)) return w - (2 + atomwidth(form));
    w = subwidth(car(form), w - 1);
    form = cdr(form);
  }
  return w;
}

void superprint (object *form, int lm, pfun_t pfun) {
  if (atom(form)) {
    if (symbolp(form) && form->name == NOTHING) pstring(symbolname(form->name), pfun);
    else printobject(form, pfun);
  }
  else if (quoted(form)) { pfun('\''); superprint(car(cdr(form)), lm + 1, pfun); }
  else if (subwidth(form, ppwidth - lm) >= 0) supersub(form, lm + PPINDENT, 0, pfun);
  else supersub(form, lm + PPINDENT, 1, pfun);
}

const int ppspecials = 16;
const char ppspecial[ppspecials] PROGMEM =
  { DOTIMES, DOLIST, IF, SETQ, TEE, LET, LETSTAR, LAMBDA, WHEN, UNLESS, WITHI2C, WITHSERIAL, WITHSPI, WITHSDCARD, FORMILLIS, WITHCLIENT };

void supersub (object *form, int lm, int super, pfun_t pfun) {
  int special = 0, separate = 1;
  object *arg = car(form);
  if (symbolp(arg)) {
    int name = arg->name;
    if (name == DEFUN) special = 2;
    else for (int i=0; i<ppspecials; i++) {
      if (name == pgm_read_byte(&ppspecial[i])) { special = 1; break; }
    } 
  }
  while (form != NULL) {
    if (atom(form)) { pfstring(PSTR(" . "), pfun); printobject(form, pfun); pfun(')'); return; }
    else if (separate) { pfun('('); separate = 0; }
    else if (special) { pfun(' '); special--; }
    else if (!super) pfun(' ');
    else { pln(pfun); indent(lm, ' ', pfun); }
    superprint(car(form), lm, pfun);
    form = cdr(form);
  }
  pfun(')'); return;
}

// Special forms

object *sp_quote (object *args, object *env) {
  (void) env;
  checkargs(QUOTE, args);
  return first(args);
}

object *sp_or (object *args, object *env) {
  while (args != NULL) {
    object *val = eval(car(args), env);
    if (val != NULL) return val;
    args = cdr(args);
  }
  return nil;
}

object *sp_defun (object *args, object *env) {
  (void) env;
  checkargs(DEFUN, args);
  object *var = first(args);
  if (!symbolp(var)) error(DEFUN, notasymbol, var);
  object *val = cons(symbol(LAMBDA), cdr(args));
  object *pair = value(var->name,GlobalEnv);
  if (pair != NULL) cdr(pair) = val;
  else push(cons(var, val), GlobalEnv);
  return var;
}

object *sp_defvar (object *args, object *env) {
  checkargs(DEFVAR, args);
  object *var = first(args);
  if (!symbolp(var)) error(DEFVAR, notasymbol, var);
  object *val = NULL;
  args = cdr(args);
  if (args != NULL) { setflag(NOESC); val = eval(first(args), env); clrflag(NOESC); }
  object *pair = value(var->name, GlobalEnv);
  if (pair != NULL) cdr(pair) = val;
  else push(cons(var, val), GlobalEnv);
  return var;
}

object *sp_setq (object *args, object *env) {
  object *arg = nil;
  while (args != NULL) {
    if (cdr(args) == NULL) error2(SETQ, oddargs);
    object *pair = findvalue(first(args), env);
    arg = eval(second(args), env);
    cdr(pair) = arg;
    args = cddr(args);
  }
  return arg;
}

object *sp_loop (object *args, object *env) {
  object *start = args;
  for (;;) {
    yield();
    args = start;
    while (args != NULL) {
      object *result = eval(car(args),env);
      if (tstflag(RETURNFLAG)) {
        clrflag(RETURNFLAG);
        return result;
      }
      args = cdr(args);
    }
  }
}

object *sp_return (object *args, object *env) {
  object *result = eval(tf_progn(args,env), env);
  setflag(RETURNFLAG);
  return result;
}

object *sp_push (object *args, object *env) {
  int bit;
  checkargs(PUSH, args); 
  object *item = eval(first(args), env);
  object **loc = place(PUSH, second(args), env, &bit);
  push(item, *loc);
  return *loc;
}

object *sp_pop (object *args, object *env) {
  int bit;
  checkargs(POP, args); 
  object **loc = place(POP, first(args), env, &bit);
  object *result = car(*loc);
  pop(*loc);
  return result;
}

// Accessors

object *sp_incf (object *args, object *env) {
  int bit;
  checkargs(INCF, args); 
  object **loc = place(INCF, first(args), env, &bit);
  args = cdr(args);
  
  object *x = *loc;
  object *inc = (args != NULL) ? eval(first(args), env) : NULL;

  if (bit != -1) {
    int increment;
    if (inc == NULL) increment = 1; else increment = checkbitvalue(INCF, inc);
    int newvalue = (((*loc)->integer)>>bit & 1) + increment;

    if (newvalue & ~1) error2(INCF, PSTR("result is not a bit value"));
    *loc = number((((*loc)->integer) & ~(1<<bit)) | newvalue<<bit);
    return number(newvalue);
  }

  if (floatp(x) || floatp(inc)) {
    float increment;
    float value = checkintfloat(INCF, x);

    if (inc == NULL) increment = 1.0; else increment = checkintfloat(INCF, inc);

    *loc = makefloat(value + increment);
  } else if (integerp(x) && (integerp(inc) || inc == NULL)) {
    int increment;
    int value = x->integer;

    if (inc == NULL) increment = 1; else increment = inc->integer;

    if (increment < 1) {
      if (INT_MIN - increment > value) *loc = makefloat((float)value + (float)increment);
      else *loc = number(value + increment);
    } else {
      if (INT_MAX - increment < value) *loc = makefloat((float)value + (float)increment);
      else *loc = number(value + increment);
    }
  } else error2(INCF, notanumber);
  return *loc;
}

object *sp_decf (object *args, object *env) {
  int bit;
  checkargs(DECF, args);
  object **loc = place(DECF, first(args), env, &bit);
  args = cdr(args);
  
  object *x = *loc;
  object *dec = (args != NULL) ? eval(first(args), env) : NULL;

  if (bit != -1) {
    int decrement;
    if (dec == NULL) decrement = 1; else decrement = checkbitvalue(DECF, dec);
    int newvalue = (((*loc)->integer)>>bit & 1) - decrement;

    if (newvalue & ~1) error2(INCF, PSTR("result is not a bit value"));
    *loc = number((((*loc)->integer) & ~(1<<bit)) | newvalue<<bit);
    return number(newvalue);
  }
  
  if (floatp(x) || floatp(dec)) {
    float decrement;
    float value = checkintfloat(DECF, x);

    if (dec == NULL) decrement = 1.0; else decrement = checkintfloat(DECF, dec);

    *loc = makefloat(value - decrement);
  } if (integerp(x) && (integerp(dec) || dec == NULL)) {
    int decrement;
    int value = x->integer;

    if (dec == NULL) decrement = 1; else decrement = dec->integer;

    if (decrement < 1) {
      if (INT_MAX + decrement < value) *loc = makefloat((float)value - (float)decrement);
      else *loc = number(value - decrement);
    } else {
      if (INT_MIN + decrement > value) *loc = makefloat((float)value - (float)decrement);
      else *loc = number(value - decrement);
    }
  } else error2(DECF, notanumber);
  return *loc;
}

object *sp_setf (object *args, object *env) {
  int bit;
  object *arg = nil;
  while (args != NULL) {
    if (cdr(args) == NULL) error2(SETF, oddargs);
    object **loc = place(SETF, first(args), env, &bit);
    arg = eval(second(args), env);
    if (bit == -1) *loc = arg;
    else *loc = number((checkinteger(SETF,*loc) & ~(1<<bit)) | checkbitvalue(SETF,arg)<<bit);
    args = cddr(args);
  }
  return arg;
}

// Other special forms

object *sp_dolist (object *args, object *env) {
  if (args == NULL || listlength(DOLIST, first(args)) < 2) error2(DOLIST, noargument);
  object *params = first(args);
  object *var = first(params);
  object *list = eval(second(params), env);
  push(list, GCStack); // Don't GC the list
  object *pair = cons(var,nil);
  push(pair,env);
  params = cdr(cdr(params));
  args = cdr(args);
  while (list != NULL) {
    if (improperp(list)) error(DOLIST, notproper, list);
    cdr(pair) = first(list);
    object *forms = args;
    while (forms != NULL) {
      object *result = eval(car(forms), env);
      if (tstflag(RETURNFLAG)) {
        clrflag(RETURNFLAG);
        pop(GCStack);
        return result;
      }
      forms = cdr(forms);
    }
    list = cdr(list);
  }
  cdr(pair) = nil;
  pop(GCStack);
  if (params == NULL) return nil;
  return eval(car(params), env);
}

object *sp_dotimes (object *args, object *env) {
  if (args == NULL || listlength(DOTIMES, first(args)) < 2) error2(DOTIMES, noargument);
  object *params = first(args);
  object *var = first(params);
  int count = checkinteger(DOTIMES, eval(second(params), env));
  int index = 0;
  params = cdr(cdr(params));
  object *pair = cons(var,number(0));
  push(pair,env);
  args = cdr(args);
  while (index < count) {
    cdr(pair) = number(index);
    object *forms = args;
    while (forms != NULL) {
      object *result = eval(car(forms), env);
      if (tstflag(RETURNFLAG)) {
        clrflag(RETURNFLAG);
        return result;
      }
      forms = cdr(forms);
    }
    index++;
  }
  cdr(pair) = number(index);
  if (params == NULL) return nil;
  return eval(car(params), env);
}

object *sp_trace (object *args, object *env) {
  (void) env;
  while (args != NULL) {
    object *var = first(args);
    if (!symbolp(var)) error(TRACE, notasymbol, var);
    trace(var->name);
    args = cdr(args);
  }
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] != 0) args = cons(symbol(TraceFn[i]), args);
    i++;
  }
  return args;
}

object *sp_untrace (object *args, object *env) {
  (void) env;
  if (args == NULL) {
    int i = 0;
    while (i < TRACEMAX) {
      if (TraceFn[i] != 0) args = cons(symbol(TraceFn[i]), args);
      TraceFn[i] = 0;
      i++;
    }
  } else {
    while (args != NULL) {
      object *var = first(args);
      if (!symbolp(var)) error(UNTRACE, notasymbol, var);
      untrace(var->name);
      args = cdr(args);
    }
  }
  return args;
}

object *sp_formillis (object *args, object *env) {
  if (args == NULL) error2(FORMILLIS, noargument);
  object *param = first(args);
  unsigned long start = millis();
  unsigned long now, total = 0;
  if (param != NULL) total = checkinteger(FORMILLIS, eval(first(param), env));
  eval(tf_progn(cdr(args),env), env);
  do {
    now = millis() - start;
    testescape();
  } while (now < total);
  if (now <= INT_MAX) return number(now);
  return nil;
}

object *sp_withoutputtostring (object *args, object *env) {
  if (args == NULL) error2(WITHOUTPUTTOSTRING, noargument);
  object *params = first(args);
  if (params == NULL) error2(WITHOUTPUTTOSTRING, nostream);
  object *var = first(params);
  object *pair = cons(var, stream(STRINGSTREAM, 0));
  push(pair,env);
  object *string = startstring(WITHOUTPUTTOSTRING);
  object *forms = cdr(args);
  eval(tf_progn(forms,env), env);
  string->cdr = GlobalString;
  GlobalString = NULL;
  return string;
}

object *sp_withserial (object *args, object *env) {
  object *params = first(args);
  if (params == NULL) error2(WITHSERIAL, nostream);
  object *var = first(params);
  int address = checkinteger(WITHSERIAL, eval(second(params), env));
  params = cddr(params);
  int baud = 96;
  if (params != NULL) baud = checkinteger(WITHSERIAL, eval(first(params), env));
  object *pair = cons(var, stream(SERIALSTREAM, address));
  push(pair,env);
  serialbegin(address, baud);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  serialend(address);
  return result;
}

object *sp_withi2c (object *args, object *env) {
  object *params = first(args);
  if (params == NULL) error2(WITHI2C, nostream);
  object *var = first(params);
  int address = checkinteger(WITHI2C, eval(second(params), env));
  params = cddr(params);
  if (address == 0) params = cdr(params); // Ignore port
  int read = 0; // Write
  I2CCount = 0;
  if (params != NULL) {
    object *rw = eval(first(params), env);
    if (integerp(rw)) I2CCount = rw->integer;
    read = (rw != NULL);
  }
  I2Cinit(1); // Pullups
  object *pair = cons(var, (I2Cstart(address, read)) ? stream(I2CSTREAM, address) : nil);
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  I2Cstop(read);
  return result;
}

object *sp_withspi (object *args, object *env) {
  object *params = first(args);
  if (params == NULL) error2(WITHSPI, nostream);
  object *var = first(params);
  params = cdr(params);
  if (params == NULL) error2(WITHSPI, nostream);
  int pin = checkinteger(WITHSPI, eval(car(params), env));
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  params = cdr(params);
  int clock = 4000, mode = SPI_MODE0; // Defaults
  BitOrder bitorder = MSBFIRST;
  if (params != NULL) {
    clock = checkinteger(WITHSPI, eval(car(params), env));
    params = cdr(params);
    if (params != NULL) {
      bitorder = (checkinteger(WITHSPI, eval(car(params), env)) == 0) ? LSBFIRST : MSBFIRST;
      params = cdr(params);
      if (params != NULL) {
        int modeval = checkinteger(WITHSPI, eval(car(params), env));
        mode = (modeval == 3) ? SPI_MODE3 : (modeval == 2) ? SPI_MODE2 : (modeval == 1) ? SPI_MODE1 : SPI_MODE0;
      }
    }
  }
  object *pair = cons(var, stream(SPISTREAM, pin));
  push(pair,env);
  SPI.begin();
  SPI.beginTransaction(SPISettings(((unsigned long)clock * 1000), bitorder, mode));
  digitalWrite(pin, LOW);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  digitalWrite(pin, HIGH);
  SPI.endTransaction();
  return result;
}

object *sp_withsdcard (object *args, object *env) {
#if defined(sdcardsupport)
  object *params = first(args);
  object *var = first(params);
  object *filename = eval(second(params), env);
  params = cddr(params);
  SD.begin();
  int mode = 0;
  if (params != NULL && first(params) != NULL) mode = checkinteger(WITHSDCARD, first(params));
  const char *oflag = FILE_READ;
  if (mode == 1) oflag = FILE_APPEND; else if (mode == 2) oflag = FILE_WRITE;
  if (mode >= 1) {
    SDpfile = SD.open(MakeFilename(filename), oflag);
    if (!SDpfile) error2(WITHSDCARD, PSTR("problem writing to SD card"));
  } else {
    SDgfile = SD.open(MakeFilename(filename), oflag);
    if (!SDgfile) error2(WITHSDCARD, PSTR("problem reading from SD card"));
  }
  object *pair = cons(var, stream(SDSTREAM, 1));
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  if (mode >= 1) SDpfile.close(); else SDgfile.close();
  return result;
#else
  (void) args, (void) env;
  error2(WITHSDCARD, PSTR("not supported"));
  return nil;
#endif
}

object *sp_withgfx (object *args, object *env) {
#if defined(gfxsupport)
  object *params = first(args);
  object *var = first(params);
  object *pair = cons(var, stream(GFXSTREAM, 1));
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  return result;
#else
  (void) args, (void) env;
  error2(WITHGFX, PSTR("not supported"));
  return nil;
#endif
}

object *sp_withclient (object *args, object *env) {
  object *params = first(args);
  object *var = first(params);
  params = cdr(params);
  int n;
  if (params == NULL) {
    client = server.available();
    if (!client) return nil;
    n = 2;
  } else {
    object *address = eval(first(params), env);
    object *port = eval(second(params), env);
    int success;
    if (stringp(address)) success = client.connect(cstringbuf(address), checkinteger(WITHCLIENT, port));
    else if (integerp(address)) success = client.connect(address->integer, checkinteger(WITHCLIENT, port));
    else error2(WITHCLIENT, PSTR("invalid address"));
    if (!success) return nil;
    n = 1;
  }
  object *pair = cons(var, stream(WIFISTREAM, n));
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  client.stop();
  return result;
}

// Tail-recursive forms

object *tf_progn (object *args, object *env) {
  if (args == NULL) return nil;
  object *more = cdr(args);
  while (more != NULL) {
    object *result = eval(car(args),env);
    if (tstflag(RETURNFLAG)) return result;
    args = more;
    more = cdr(args);
  }
  return car(args);
}

object *tf_if (object *args, object *env) {
  if (args == NULL || cdr(args) == NULL) error2(IF, PSTR("missing argument(s)"));
  if (eval(first(args), env) != nil) return second(args);
  args = cddr(args);
  return (args != NULL) ? first(args) : nil;
}

object *tf_cond (object *args, object *env) {
  while (args != NULL) {
    object *clause = first(args);
    if (!consp(clause)) error(COND, PSTR("illegal clause"), clause);
    object *test = eval(first(clause), env);
    object *forms = cdr(clause);
    if (test != nil) {
      if (forms == NULL) return quote(test); else return tf_progn(forms, env);
    }
    args = cdr(args);
  }
  return nil;
}

object *tf_when (object *args, object *env) {
  if (args == NULL) error2(WHEN, noargument);
  if (eval(first(args), env) != nil) return tf_progn(cdr(args),env);
  else return nil;
}

object *tf_unless (object *args, object *env) {
  if (args == NULL) error2(UNLESS, noargument);
  if (eval(first(args), env) != nil) return nil;
  else return tf_progn(cdr(args),env);
}

object *tf_case (object *args, object *env) {
  object *test = eval(first(args), env);
  args = cdr(args);
  while (args != NULL) {
    object *clause = first(args);
    if (!consp(clause)) error(CASE, PSTR("illegal clause"), clause);
    object *key = car(clause);
    object *forms = cdr(clause);
    if (consp(key)) {
      while (key != NULL) {
        if (eq(test,car(key))) return tf_progn(forms, env);
        key = cdr(key);
      }
    } else if (eq(test,key) || eq(key,tee)) return tf_progn(forms, env);
    args = cdr(args);
  }
  return nil;
}

object *tf_and (object *args, object *env) {
  if (args == NULL) return tee;
  object *more = cdr(args);
  while (more != NULL) {
    if (eval(car(args), env) == NULL) return nil;
    args = more;
    more = cdr(args);
  }
  return car(args);
}

// Core functions

object *fn_not (object *args, object *env) {
  (void) env;
  return (first(args) == nil) ? tee : nil;
}

object *fn_cons (object *args, object *env) {
  (void) env;
  return cons(first(args), second(args));
}

object *fn_atom (object *args, object *env) {
  (void) env;
  return atom(first(args)) ? tee : nil;
}

object *fn_listp (object *args, object *env) {
  (void) env;
  return listp(first(args)) ? tee : nil;
}

object *fn_consp (object *args, object *env) {
  (void) env;
  return consp(first(args)) ? tee : nil;
}

object *fn_symbolp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  return (arg == NULL || symbolp(arg)) ? tee : nil;
}

object *fn_arrayp (object *args, object *env) {
  (void) env;
  return arrayp(first(args)) ? tee : nil;
}

object *fn_boundp (object *args, object *env) {
  (void) env;
  object *var = first(args);
  if (!symbolp(var)) error(BOUNDP, notasymbol, var);
  return boundp(var, env) ? tee : nil;
}

object *fn_setfn (object *args, object *env) {
  object *arg = nil;
  while (args != NULL) {
    if (cdr(args) == NULL) error2(SETFN, oddargs);
    object *pair = findvalue(first(args), env);
    arg = second(args);
    cdr(pair) = arg;
    args = cddr(args);
  }
  return arg;
}

object *fn_streamp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  return streamp(arg) ? tee : nil;
}

object *fn_eq (object *args, object *env) {
  (void) env;
  return eq(first(args), second(args)) ? tee : nil;
}

// List functions

object *fn_car (object *args, object *env) {
  (void) env;
  return carx(first(args));
}

object *fn_cdr (object *args, object *env) {
  (void) env;
  return cdrx(first(args));
}

object *fn_caar (object *args, object *env) {
  (void) env;
  return carx(carx(first(args)));
}

object *fn_cadr (object *args, object *env) {
  (void) env;
  return carx(cdrx(first(args)));
}

object *fn_cdar (object *args, object *env) {
  (void) env;
  return cdrx(carx(first(args)));
}

object *fn_cddr (object *args, object *env) {
  (void) env;
  return cdrx(cdrx(first(args)));
}

object *fn_caaar (object *args, object *env) {
  (void) env;
  return carx(carx(carx(first(args))));
}

object *fn_caadr (object *args, object *env) {
  (void) env;
  return carx(carx(cdrx(first(args))));
}

object *fn_cadar (object *args, object *env) {
  (void) env;
  return carx(cdrx(carx(first(args))));
}

object *fn_caddr (object *args, object *env) {
  (void) env;
  return carx(cdrx(cdrx(first(args))));
}

object *fn_cdaar (object *args, object *env) {
  (void) env;
  return cdrx(carx(carx(first(args))));
}

object *fn_cdadr (object *args, object *env) {
  (void) env;
  return cdrx(carx(cdrx(first(args))));
}

object *fn_cddar (object *args, object *env) {
  (void) env;
  return cdrx(cdrx(carx(first(args))));
}

object *fn_cdddr (object *args, object *env) {
  (void) env;
  return cdrx(cdrx(cdrx(first(args))));
}

object *fn_length (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (listp(arg)) return number(listlength(LENGTH, arg));
  if (stringp(arg)) return number(stringlength(arg));
  if (!(arrayp(arg) && cdr(cddr(arg)) == NULL)) error(LENGTH, PSTR("argument is not a list, 1d array, or string"), arg);
  return number(abs(first(cddr(arg))->integer));
}

object *fn_arraydimensions (object *args, object *env) {
  object *array = first(args);
  if (!arrayp(array)) error(ARRAYDIMENSIONS, PSTR("argument is not an array"), array);
  object *dimensions = cddr(array);
  return (first(dimensions)->integer < 0) ? cons(number(-(first(dimensions)->integer)), cdr(dimensions)) : dimensions;
}

object *fn_list (object *args, object *env) {
  (void) env;
  return args;
}

object *fn_makearray (object *args, object *env) {
  (void) env;
  object *def = nil;
  bool bitp = false;
  object *dims = first(args);
  if (dims == NULL) error2(MAKEARRAY, PSTR("dimensions can't be nil"));
  else if (atom(dims)) dims = cons(dims, NULL);
  args = cdr(args);
  while (args != NULL && cdr(args) != NULL) {
    object *var = first(args);
    if (issymbol(first(args), INITIALELEMENT)) def = second(args);
    else if (issymbol(first(args), ELEMENTTYPE) && issymbol(second(args), BIT)) bitp = true;
    else error(MAKEARRAY, PSTR("argument not recognised"), var); 
    args = cddr(args);
  }
  if (bitp) {
    if (def == nil) def = number(0);
    else def = number(-checkbitvalue(MAKEARRAY, def)); // 1 becomes all ones
  }
  return makearray(MAKEARRAY, dims, def, bitp);
}

object *fn_reverse (object *args, object *env) {
  (void) env;
  object *list = first(args);
  object *result = NULL;
  while (list != NULL) {
    if (improperp(list)) error(REVERSE, notproper, list);
    push(first(list),result);
    list = cdr(list);
  }
  return result;
}

object *fn_nth (object *args, object *env) {
  (void) env;
  int n = checkinteger(NTH, first(args));
  if (n < 0) error(NTH, indexnegative, first(args));
  object *list = second(args);
  while (list != NULL) {
    if (improperp(list)) error(NTH, notproper, list);
    if (n == 0) return car(list);
    list = cdr(list);
    n--;
  }
  return nil;
}

object *fn_aref (object *args, object *env) {
  int bit;
  object *array = first(args);
  if (!arrayp(array)) error(AREF, PSTR("first argument is not an array"), array);
  object *loc = *getarray(AREF, array, cdr(args), 0, &bit);
  if (bit == -1) return loc;
  else return number((loc->integer)>>bit & 1);
}

object *fn_assoc (object *args, object *env) {
  (void) env;
  object *key = first(args);
  object *list = second(args);
  return assoc(key,list);
}

object *fn_member (object *args, object *env) {
  (void) env;
  object *item = first(args);
  object *list = second(args);
  while (list != NULL) {
    if (improperp(list)) error(MEMBER, notproper, list);
    if (eq(item,car(list))) return list;
    list = cdr(list);
  }
  return nil;
}

object *fn_apply (object *args, object *env) {
  object *previous = NULL;
  object *last = args;
  while (cdr(last) != NULL) {
    previous = last;
    last = cdr(last);
  }
  object *arg = car(last);
  if (!listp(arg)) error(APPLY, notalist, arg);
  cdr(previous) = arg;
  return apply(APPLY, first(args), cdr(args), env);
}

object *fn_funcall (object *args, object *env) {
  return apply(FUNCALL, first(args), cdr(args), env);
}

object *fn_append (object *args, object *env) {
  (void) env;
  object *head = NULL;
  object *tail;
  while (args != NULL) {
    object *list = first(args);
    if (!listp(list)) error(APPEND, notalist, list);
    while (consp(list)) {
      object *obj = cons(car(list), cdr(list));
      if (head == NULL) head = obj;
      else cdr(tail) = obj;
      tail = obj;
      list = cdr(list);
      if (cdr(args) != NULL && improperp(list)) error(APPEND, notproper, first(args));
    }
    args = cdr(args);
  }
  return head;
}

object *fn_mapc (object *args, object *env) {
  object *function = first(args);
  args = cdr(args);
  object *result = first(args);
  object *params = cons(NULL, NULL);
  push(params,GCStack);
  // Make parameters
  while (true) {
    object *tailp = params;
    object *lists = args;
    while (lists != NULL) {
      object *list = car(lists);
      if (list == NULL) {
         pop(GCStack);
         return result;
      }
      if (improperp(list)) error(MAPC, notproper, list);
      object *obj = cons(first(list),NULL);
      car(lists) = cdr(list);
      cdr(tailp) = obj; tailp = obj;
      lists = cdr(lists);
    }
    apply(MAPC, function, cdr(params), env);
  }
}

void mapcarfun (object *result, object **tail) {
  object *obj = cons(result,NULL);
  cdr(*tail) = obj; *tail = obj;
}

void mapcanfun (object *result, object **tail) {
  if (cdr(*tail) != NULL) error(MAPCAN, notproper, *tail);
  while (consp(result)) {
    cdr(*tail) = result; *tail = result;
    result = cdr(result);
  }
}

object *mapcarcan (symbol_t name, object *args, object *env, mapfun_t fun) {
  object *function = first(args);
  args = cdr(args);
  object *params = cons(NULL, NULL);
  push(params,GCStack);
  object *head = cons(NULL, NULL); 
  push(head,GCStack);
  object *tail = head;
  // Make parameters
  while (true) {
    object *tailp = params;
    object *lists = args;
    while (lists != NULL) {
      object *list = car(lists);
      if (list == NULL) {
         pop(GCStack);
         pop(GCStack);
         return cdr(head);
      }
      if (improperp(list)) error(name, notproper, list);
      object *obj = cons(first(list),NULL);
      car(lists) = cdr(list);
      cdr(tailp) = obj; tailp = obj;
      lists = cdr(lists);
    }
    object *result = apply(name, function, cdr(params), env);
    fun(result, &tail);
  }
}

object *fn_mapcar (object *args, object *env) {
  return mapcarcan(MAPCAR, args, env, mapcarfun);
}

object *fn_mapcan (object *args, object *env) {
  return mapcarcan(MAPCAN, args, env, mapcanfun);
}

// Arithmetic functions

object *add_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    fresult = fresult + checkintfloat(ADD, arg);
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *fn_add (object *args, object *env) {
  (void) env;
  int result = 0;
  while (args != NULL) {
    object *arg = car(args);
    if (floatp(arg)) return add_floats(args, (float)result);
    else if (integerp(arg)) {
      int val = arg->integer;
      if (val < 1) { if (INT_MIN - val > result) return add_floats(args, (float)result); }
      else { if (INT_MAX - val < result) return add_floats(args, (float)result); }
      result = result + val;
    } else error(ADD, notanumber, arg);
    args = cdr(args);
  }
  return number(result);
}

object *subtract_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    fresult = fresult - checkintfloat(SUBTRACT, arg);
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *negate (object *arg) {
  if (integerp(arg)) {
    int result = arg->integer;
    if (result == INT_MIN) return makefloat(-result);
    else return number(-result);
  } else if (floatp(arg)) return makefloat(-(arg->single_float));
  else error(SUBTRACT, notanumber, arg);
  return nil;
}

object *fn_subtract (object *args, object *env) {
  (void) env;
  object *arg = car(args);
  args = cdr(args);
  if (args == NULL) return negate(arg);
  else if (floatp(arg)) return subtract_floats(args, arg->single_float);
  else if (integerp(arg)) {
    int result = arg->integer;
    while (args != NULL) {
      arg = car(args);
      if (floatp(arg)) return subtract_floats(args, result);
      else if (integerp(arg)) {
        int val = (car(args))->integer;
        if (val < 1) { if (INT_MAX + val < result) return subtract_floats(args, result); }
        else { if (INT_MIN + val > result) return subtract_floats(args, result); }
        result = result - val;
      } else error(SUBTRACT, notanumber, arg);
      args = cdr(args);
    }
    return number(result);
  } else error(SUBTRACT, notanumber, arg);
  return nil;
}

object *multiply_floats (object *args, float fresult) {
  while (args != NULL) {
   object *arg = car(args);
    fresult = fresult * checkintfloat(MULTIPLY, arg);
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *fn_multiply (object *args, object *env) {
  (void) env;
  int result = 1;
  while (args != NULL){
    object *arg = car(args);
    if (floatp(arg)) return multiply_floats(args, result);
    else if (integerp(arg)) {
      int64_t val = result * (int64_t)(arg->integer);
      if ((val > INT_MAX) || (val < INT_MIN)) return multiply_floats(args, result);
      result = val;
    } else error(MULTIPLY, notanumber, arg);
    args = cdr(args);
  }
  return number(result);
}

object *divide_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    float f = checkintfloat(DIVIDE, arg);
    if (f == 0.0) error2(DIVIDE, PSTR("division by zero"));
    fresult = fresult / f;
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *fn_divide (object *args, object *env) {
  (void) env;
  object* arg = first(args);
  args = cdr(args);
  // One argument
  if (args == NULL) {
    if (floatp(arg)) {
      float f = arg->single_float;
      if (f == 0.0) error2(DIVIDE, PSTR("division by zero"));
      return makefloat(1.0 / f);
    } else if (integerp(arg)) {
      int i = arg->integer;
      if (i == 0) error2(DIVIDE, PSTR("division by zero"));
      else if (i == 1) return number(1);
      else return makefloat(1.0 / i);
    } else error(DIVIDE, notanumber, arg);
  }
  // Multiple arguments
  if (floatp(arg)) return divide_floats(args, arg->single_float);
  else if (integerp(arg)) {
    int result = arg->integer;
    while (args != NULL) {
      arg = car(args);
      if (floatp(arg)) {
        return divide_floats(args, result);
      } else if (integerp(arg)) {
        int i = arg->integer;
        if (i == 0) error2(DIVIDE, PSTR("division by zero"));
        if ((result % i) != 0) return divide_floats(args, result);
        if ((result == INT_MIN) && (i == -1)) return divide_floats(args, result);
        result = result / i;
        args = cdr(args);
      } else error(DIVIDE, notanumber, arg);
    }
    return number(result);
  } else error(DIVIDE, notanumber, arg);
  return nil;
}

object *fn_mod (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  object *arg2 = second(args);
  if (integerp(arg1) && integerp(arg2)) {
    int divisor = arg2->integer;
    if (divisor == 0) error2(MOD, PSTR("division by zero"));
    int dividend = arg1->integer;
    int remainder = dividend % divisor;
    if ((dividend<0) != (divisor<0)) remainder = remainder + divisor;
    return number(remainder);
  } else {
    float fdivisor = checkintfloat(MOD, arg2);
    if (fdivisor == 0.0) error2(MOD, PSTR("division by zero"));
    float fdividend = checkintfloat(MOD, arg1);
    float fremainder = fmod(fdividend , fdivisor);
    if ((fdividend<0) != (fdivisor<0)) fremainder = fremainder + fdivisor;
    return makefloat(fremainder);
  }
}

object *fn_oneplus (object *args, object *env) {
  (void) env;
  object* arg = first(args);
  if (floatp(arg)) return makefloat((arg->single_float) + 1.0);
  else if (integerp(arg)) {
    int result = arg->integer;
    if (result == INT_MAX) return makefloat((arg->integer) + 1.0);
    else return number(result + 1);
  } else error(ONEPLUS, notanumber, arg);
  return nil;
}

object *fn_oneminus (object *args, object *env) {
  (void) env;
  object* arg = first(args);
  if (floatp(arg)) return makefloat((arg->single_float) - 1.0);
  else if (integerp(arg)) {
    int result = arg->integer;
    if (result == INT_MIN) return makefloat((arg->integer) - 1.0);
    else return number(result - 1);
  } else error(ONEMINUS, notanumber, arg);
  return nil;
}

object *fn_abs (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return makefloat(abs(arg->single_float));
  else if (integerp(arg)) {
    int result = arg->integer;
    if (result == INT_MIN) return makefloat(abs((float)result));
    else return number(abs(result));
  } else error(ABS, notanumber, arg);
  return nil;
}

object *fn_random (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (integerp(arg)) return number(random(arg->integer));
  else if (floatp(arg)) return makefloat((float)rand()/(float)(RAND_MAX/(arg->single_float)));
  else error(RANDOM, notanumber, arg);
  return nil;
}

object *fn_maxfn (object *args, object *env) {
  (void) env;
  object* result = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg = car(args);
    if (integerp(result) && integerp(arg)) {
      if ((arg->integer) > (result->integer)) result = arg;
    } else if ((checkintfloat(MAXFN, arg) > checkintfloat(MAXFN, result))) result = arg;
    args = cdr(args);
  }
  return result;
}

object *fn_minfn (object *args, object *env) {
  (void) env;
  object* result = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg = car(args);
    if (integerp(result) && integerp(arg)) {
      if ((arg->integer) < (result->integer)) result = arg;
    } else if ((checkintfloat(MINFN, arg) < checkintfloat(MINFN, result))) result = arg;
    args = cdr(args);
  }
  return result;
}

// Arithmetic comparisons

object *fn_noteq (object *args, object *env) {
  (void) env;
  while (args != NULL) {
    object *nargs = args;
    object *arg1 = first(nargs);
    nargs = cdr(nargs);
    while (nargs != NULL) {
      object *arg2 = first(nargs);
      if (integerp(arg1) && integerp(arg2)) {
        if ((arg1->integer) == (arg2->integer)) return nil;
      } else if ((checkintfloat(NOTEQ, arg1) == checkintfloat(NOTEQ, arg2))) return nil;
      nargs = cdr(nargs);
    }
    args = cdr(args);
  }
  return tee;
}

object *fn_numeq (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!((arg1->integer) == (arg2->integer))) return nil;
    } else if (!(checkintfloat(NUMEQ, arg1) == checkintfloat(NUMEQ, arg2))) return nil;
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
}

object *fn_less (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!((arg1->integer) < (arg2->integer))) return nil;
    } else if (!(checkintfloat(LESS, arg1) < checkintfloat(LESS, arg2))) return nil;
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
}

object *fn_lesseq (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!((arg1->integer) <= (arg2->integer))) return nil;
    } else if (!(checkintfloat(LESSEQ, arg1) <= checkintfloat(LESSEQ, arg2))) return nil;
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
}

object *fn_greater (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!((arg1->integer) > (arg2->integer))) return nil;
    } else if (!(checkintfloat(GREATER, arg1) > checkintfloat(GREATER, arg2))) return nil;
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
}

object *fn_greatereq (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!((arg1->integer) >= (arg2->integer))) return nil;
    } else if (!(checkintfloat(GREATEREQ, arg1) >= checkintfloat(GREATEREQ, arg2))) return nil;
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
}

object *fn_plusp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) > 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) > 0) ? tee : nil;
  else error(PLUSP, notanumber, arg);
  return nil;
}

object *fn_minusp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) < 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) < 0) ? tee : nil;
  else error(MINUSP, notanumber, arg);
  return nil;
}

object *fn_zerop (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) == 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) == 0) ? tee : nil;
  else error(ZEROP, notanumber, arg);
  return nil;
}

object *fn_oddp (object *args, object *env) {
  (void) env;
  int arg = checkinteger(ODDP, first(args));
  return ((arg & 1) == 1) ? tee : nil;
}

object *fn_evenp (object *args, object *env) {
  (void) env;
  int arg = checkinteger(EVENP, first(args));
  return ((arg & 1) == 0) ? tee : nil;
}

// Number functions

object *fn_integerp (object *args, object *env) {
  (void) env;
  return integerp(first(args)) ? tee : nil;
}

object *fn_numberp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  return (integerp(arg) || floatp(arg)) ? tee : nil;
}

// Floating-point functions

object *fn_floatfn (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  return (floatp(arg)) ? arg : makefloat((float)(arg->integer));
}

object *fn_floatp (object *args, object *env) {
  (void) env;
  return floatp(first(args)) ? tee : nil;
}

object *fn_sin (object *args, object *env) {
  (void) env;
  return makefloat(sin(checkintfloat(SIN, first(args))));
}

object *fn_cos (object *args, object *env) {
  (void) env;
  return makefloat(cos(checkintfloat(COS, first(args))));
}

object *fn_tan (object *args, object *env) {
  (void) env;
  return makefloat(tan(checkintfloat(TAN, first(args))));
}

object *fn_asin (object *args, object *env) {
  (void) env;
  return makefloat(asin(checkintfloat(ASIN, first(args))));
}

object *fn_acos (object *args, object *env) {
  (void) env;
  return makefloat(acos(checkintfloat(ACOS, first(args))));
}

object *fn_atan (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  float div = 1.0;
  args = cdr(args);
  if (args != NULL) div = checkintfloat(ATAN, first(args));
  return makefloat(atan2(checkintfloat(ATAN, arg), div));
}

object *fn_sinh (object *args, object *env) {
  (void) env;
  return makefloat(sinh(checkintfloat(SINH, first(args))));
}

object *fn_cosh (object *args, object *env) {
  (void) env;
  return makefloat(cosh(checkintfloat(COSH, first(args))));
}

object *fn_tanh (object *args, object *env) {
  (void) env;
  return makefloat(tanh(checkintfloat(TANH, first(args))));
}

object *fn_exp (object *args, object *env) {
  (void) env;
  return makefloat(exp(checkintfloat(EXP, first(args))));
}

object *fn_sqrt (object *args, object *env) {
  (void) env;
  return makefloat(sqrt(checkintfloat(SQRT, first(args))));
}

object *fn_log (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  float fresult = log(checkintfloat(LOG, arg));
  args = cdr(args);
  if (args == NULL) return makefloat(fresult);
  else return makefloat(fresult / log(checkintfloat(LOG, first(args))));
}

int intpower (int base, int exp) {
  int result = 1;
  while (exp) {
    if (exp & 1) result = result * base;
    exp = exp / 2;
    base = base * base;
  }
  return result;
}

object *fn_expt (object *args, object *env) {
  (void) env;
  object *arg1 = first(args); object *arg2 = second(args);
  float float1 = checkintfloat(EXPT, arg1);
  float value = log(abs(float1)) * checkintfloat(EXPT, arg2);
  if (integerp(arg1) && integerp(arg2) && ((arg2->integer) > 0) && (abs(value) < 21.4875))
    return number(intpower(arg1->integer, arg2->integer));
  if (float1 < 0) {
    if (integerp(arg2)) return makefloat((arg2->integer & 1) ? -exp(value) : exp(value));
    else error2(EXPT, PSTR("invalid result"));
  }
  return makefloat(exp(value));
}

object *fn_ceiling (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) return number(ceil(checkintfloat(CEILING, arg) / checkintfloat(CEILING, first(args))));
  else return number(ceil(checkintfloat(CEILING, arg)));
}

object *fn_floor (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) return number(floor(checkintfloat(FLOOR, arg) / checkintfloat(FLOOR, first(args))));
  else return number(floor(checkintfloat(FLOOR, arg)));
}

object *fn_truncate (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) return number((int)(checkintfloat(TRUNCATE, arg) / checkintfloat(TRUNCATE, first(args))));
  else return number((int)(checkintfloat(TRUNCATE, arg)));
}

int myround (float number) {
  return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

object *fn_round (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) return number(myround(checkintfloat(ROUND, arg) / checkintfloat(ROUND, first(args))));
  else return number(myround(checkintfloat(ROUND, arg)));
}

// Characters

object *fn_char (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!stringp(arg)) error(CHAR, notastring, arg);
  char c = nthchar(arg, checkinteger(CHAR, second(args)));
  if (c == 0) error2(CHAR, PSTR("index out of range"));
  return character(c);
}

object *fn_charcode (object *args, object *env) {
  (void) env;
  return number(checkchar(CHARCODE, first(args)));
}

object *fn_codechar (object *args, object *env) {
  (void) env;
  return character(checkinteger(CODECHAR, first(args)));
}

object *fn_characterp (object *args, object *env) {
  (void) env;
  return characterp(first(args)) ? tee : nil;
}

// Strings

object *fn_stringp (object *args, object *env) {
  (void) env;
  return stringp(first(args)) ? tee : nil;
}

bool stringcompare (symbol_t name, object *args, bool lt, bool gt, bool eq) {
  object *arg1 = first(args); if (!stringp(arg1)) error(name, notastring, arg1);
  object *arg2 = second(args); if (!stringp(arg2)) error(name, notastring, arg2);
  arg1 = cdr(arg1);
  arg2 = cdr(arg2);
  while ((arg1 != NULL) || (arg2 != NULL)) {
    if (arg1 == NULL) return lt;
    if (arg2 == NULL) return gt;
    if (arg1->chars < arg2->chars) return lt;
    if (arg1->chars > arg2->chars) return gt;
    arg1 = car(arg1);
    arg2 = car(arg2);
  }
  return eq;
}

object *fn_stringeq (object *args, object *env) {
  (void) env;
  return stringcompare(STRINGEQ, args, false, false, true) ? tee : nil;
}

object *fn_stringless (object *args, object *env) {
  (void) env;
  return stringcompare(STRINGLESS, args, true, false, false) ? tee : nil;
}

object *fn_stringgreater (object *args, object *env) {
  (void) env;
  return stringcompare(STRINGGREATER, args, false, true, false) ? tee : nil;
}

object *fn_sort (object *args, object *env) {
  if (first(args) == NULL) return nil;
  object *list = cons(nil,first(args));
  push(list,GCStack);
  object *predicate = second(args);
  object *compare = cons(NULL, cons(NULL, NULL));
  push(compare,GCStack);
  object *ptr = cdr(list);
  while (cdr(ptr) != NULL) {
    object *go = list;
    while (go != ptr) {
      car(compare) = car(cdr(ptr));
      car(cdr(compare)) = car(cdr(go));
      if (apply(SORT, predicate, compare, env)) break;
      go = cdr(go);
    }
    if (go != ptr) {
      object *obj = cdr(ptr);
      cdr(ptr) = cdr(obj);
      cdr(obj) = cdr(go);
      cdr(go) = obj;
    } else ptr = cdr(ptr);
  }
  pop(GCStack); pop(GCStack);
  return cdr(list);
}

object *fn_stringfn (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  int type = arg->type;
  if (type == STRING) return arg;
  object *obj = myalloc();
  obj->type = STRING;
  if (type == CHARACTER) {
    object *cell = myalloc();
    cell->car = NULL;
    uint8_t shift = (sizeof(int)-1)*8;
    cell->chars = (arg->chars)<<shift;
    obj->cdr = cell;
  } else if (type == SYMBOL) {
    char *s = symbolname(arg->name);
    char ch = *s++;
    object *head = NULL;
    int chars = 0;
    while (ch) {
      if (ch == '\\') ch = *s++;
      buildstring(ch, &chars, &head);
      ch = *s++;
    }
    obj->cdr = head;
  } else error(STRINGFN, PSTR("can't convert to string"), arg);
  return obj;
}

object *fn_concatenate (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (arg->name != STRINGFN) error2(CONCATENATE, PSTR("only supports strings"));
  args = cdr(args);
  object *result = myalloc();
  result->type = STRING;
  object *head = NULL;
  int chars = 0;
  while (args != NULL) {
    object *obj = first(args);
    if (!stringp(obj)) error(CONCATENATE, notastring, obj);
    obj = cdr(obj);
    while (obj != NULL) {
      int quad = obj->chars;
      while (quad != 0) {
         char ch = quad>>((sizeof(int)-1)*8) & 0xFF;
         buildstring(ch, &chars, &head);
         quad = quad<<8;
      }
      obj = car(obj);
    }
    args = cdr(args);
  }
  result->cdr = head;
  return result;
}

object *fn_subseq (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!stringp(arg)) error(SUBSEQ, notastring, arg);
  int start = checkinteger(SUBSEQ, second(args));
  if (start < 0) error(SUBSEQ, indexnegative, second(args));
  int end;
  args = cddr(args);
  if (args != NULL) end = checkinteger(SUBSEQ, car(args)); else end = stringlength(arg);
  object *result = myalloc();
  result->type = STRING;
  object *head = NULL;
  int chars = 0;
  for (int i=start; i<end; i++) {
    char ch = nthchar(arg, i);
    if (ch == 0) error2(SUBSEQ, PSTR("index out of range"));
    buildstring(ch, &chars, &head);
  }
  result->cdr = head;
  return result;
}

object *fn_readfromstring (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!stringp(arg)) error(READFROMSTRING, notastring, arg);
  GlobalString = arg;
  GlobalStringIndex = 0;
  return read(gstr);
}

object *fn_princtostring (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  object *obj = startstring(PRINCTOSTRING);
  prin1object(arg, pstr);
  obj->cdr = GlobalString;
  return obj;
}

object *fn_prin1tostring (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  object *obj = startstring(PRIN1TOSTRING);
  printobject(arg, pstr);
  obj->cdr = GlobalString;
  return obj;
}

// Bitwise operators

object *fn_logand (object *args, object *env) {
  (void) env;
  int result = -1;
  while (args != NULL) {
    result = result & checkinteger(LOGAND, first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_logior (object *args, object *env) {
  (void) env;
  int result = 0;
  while (args != NULL) {
    result = result | checkinteger(LOGIOR, first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_logxor (object *args, object *env) {
  (void) env;
  int result = 0;
  while (args != NULL) {
    result = result ^ checkinteger(LOGXOR, first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_lognot (object *args, object *env) {
  (void) env;
  int result = checkinteger(LOGNOT, car(args));
  return number(~result);
}

object *fn_ash (object *args, object *env) {
  (void) env;
  int value = checkinteger(ASH, first(args));
  int count = checkinteger(ASH, second(args));
  if (count >= 0) return number(value << count);
  else return number(value >> abs(count));
}

object *fn_logbitp (object *args, object *env) {
  (void) env;
  int index = checkinteger(LOGBITP, first(args));
  int value = checkinteger(LOGBITP, second(args));
  return (bitRead(value, index) == 1) ? tee : nil;
}

// System functions

object *fn_eval (object *args, object *env) {
  return eval(first(args), env);
}

object *fn_globals (object *args, object *env) {
  (void) args;
  if (GlobalEnv == NULL) return nil;
  return fn_mapcar(cons(symbol(CAR),cons(GlobalEnv,nil)), env);
}

object *fn_locals (object *args, object *env) {
  (void) args;
  return env;
}

object *fn_makunbound (object *args, object *env) {
  (void) env;
  object *var = first(args);
  if (!symbolp(var)) error(MAKUNBOUND, notasymbol, var);
  delassoc(var, &GlobalEnv);
  return var;
}

object *fn_break (object *args, object *env) {
  (void) args;
  pfstring(PSTR("\nBreak!\n"), pserial);
  BreakLevel++;
  repl(env);
  BreakLevel--;
  return nil;
}

object *fn_read (object *args, object *env) {
  (void) env;
  gfun_t gfun = gstreamfun(args);
  return read(gfun);
}

object *fn_prin1 (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  printobject(obj, pfun);
  return obj;
}

object *fn_print (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  pln(pfun);
  printobject(obj, pfun);
  pfun(' ');
  return obj;
}

object *fn_princ (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  prin1object(obj, pfun);
  return obj;
}

object *fn_terpri (object *args, object *env) {
  (void) env;
  pfun_t pfun = pstreamfun(args);
  pln(pfun);
  return nil;
}

object *fn_readbyte (object *args, object *env) {
  (void) env;
  gfun_t gfun = gstreamfun(args);
  int c = gfun();
  return (c == -1) ? nil : number(c);
}

object *fn_readline (object *args, object *env) {
  (void) env;
  gfun_t gfun = gstreamfun(args);
  return readstring('\n', gfun);
}

object *fn_writebyte (object *args, object *env) {
  (void) env;
  int value = checkinteger(WRITEBYTE, first(args));
  pfun_t pfun = pstreamfun(cdr(args));
  (pfun)(value);
  return nil;
}

object *fn_writestring (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  char temp = Flags;
  clrflag(PRINTREADABLY);
  printstring(obj, pfun);
  Flags = temp;
  return nil;
}

object *fn_writeline (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  char temp = Flags;
  clrflag(PRINTREADABLY);
  printstring(obj, pfun);
  pln(pfun);
  Flags = temp;
  return nil;
}

object *fn_restarti2c (object *args, object *env) {
  (void) env;
  int stream = first(args)->integer;
  args = cdr(args);
  int read = 0; // Write
  I2CCount = 0;
  if (args != NULL) {
    object *rw = first(args);
    if (integerp(rw)) I2CCount = rw->integer;
    read = (rw != NULL);
  }
  int address = stream & 0xFF;
  if (stream>>8 != I2CSTREAM) error2(RESTARTI2C, PSTR("not an i2c stream"));
  return I2Crestart(address, read) ? tee : nil;
}

object *fn_gc (object *obj, object *env) {
  int initial = Freespace;
  unsigned long start = micros();
  gc(obj, env);
  unsigned long elapsed = micros() - start;
  pfstring(PSTR("Space: "), pserial);
  pint(Freespace - initial, pserial);
  pfstring(PSTR(" bytes, Time: "), pserial);
  pint(elapsed, pserial);
  pfstring(PSTR(" us\n"), pserial);
  return nil;
}

object *fn_room (object *args, object *env) {
  (void) args, (void) env;
  return number(Freespace);
}

object *fn_saveimage (object *args, object *env) {
  if (args != NULL) args = eval(first(args), env);
  return number(saveimage(args));
}

object *fn_loadimage (object *args, object *env) {
  (void) env;
  if (args != NULL) args = first(args);
  return number(loadimage(args));
}

object *fn_cls (object *args, object *env) {
  (void) args, (void) env;
  pserial(12);
  return nil;
}

// Arduino procedures

object *fn_pinmode (object *args, object *env) {
  (void) env;
  int pin = checkinteger(PINMODE, first(args));
  PinMode pm = INPUT;
  object *arg = second(args);
  if (keywordp(arg)) pm = (PinMode)checkkeyword(PINMODE, arg);
  else if (integerp(arg)) {
    int mode = arg->integer;
    if (mode == 1) pm = OUTPUT; else if (mode == 2) pm = INPUT_PULLUP;
    #if defined(INPUT_PULLDOWN)
    else if (mode == 4) pm = INPUT_PULLDOWN;
    #endif
  } else if (arg != nil) pm = OUTPUT;
  pinMode(pin, pm);
  return nil;
}

object *fn_digitalread (object *args, object *env) {
  (void) env;
  int pin = checkinteger(DIGITALREAD, first(args));
  if (digitalRead(pin) != 0) return tee; else return nil;
}

object *fn_digitalwrite (object *args, object *env) {
  (void) env;
  int pin = checkinteger(DIGITALWRITE, first(args));
  object *arg = second(args);
  int mode;
  if (keywordp(arg)) mode = checkkeyword(DIGITALWRITE, arg);
  else if (integerp(arg)) mode = arg->integer ? HIGH : LOW;
  else mode = (arg != nil) ? HIGH : LOW;
  digitalWrite(pin, mode);
  return arg;
}

object *fn_analogread (object *args, object *env) {
  (void) env;
  int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(ANALOGREAD, arg);
  else {
    pin = checkinteger(ANALOGREAD, arg);
    checkanalogread(pin);
  }
  return number(analogRead(pin));
}

object *fn_analogreadresolution (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  #if defined(ESP32)
  analogReadResolution(checkinteger(ANALOGREADRESOLUTION, arg));
  #else
  error2(ANALOGREADRESOLUTION, PSTR("not supported"));
  #endif
  return arg;
}

object *fn_analogwrite (object *args, object *env) {
  (void) env;
  int pin = checkinteger(ANALOGWRITE, first(args));
  checkanalogwrite(pin);
  object *value = second(args);
  analogWrite(pin, checkinteger(ANALOGWRITE, value));
  return value;
}

object *fn_delay (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  delay(checkinteger(DELAY, arg1));
  return arg1;
}

object *fn_millis (object *args, object *env) {
  (void) args, (void) env;
  return number(millis());
}

object *fn_sleep (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  sleep(checkinteger(SLEEP, arg1));
  return arg1;
}

object *fn_note (object *args, object *env) {
  (void) env;
  static int pin = 255;
  if (args != NULL) {
    pin = checkinteger(NOTE, first(args));
    int note = 0;
    if (cddr(args) != NULL) note = checkinteger(NOTE, second(args));
    int octave = 0;
    if (cddr(args) != NULL) octave = checkinteger(NOTE, third(args));
    playnote(pin, note, octave);
  } else nonote(pin);
  return nil;
}

// Tree Editor

object *fn_edit (object *args, object *env) {
  object *fun = first(args);
  object *pair = findvalue(fun, env);
  clrflag(EXITEDITOR);
  object *arg = edit(eval(fun, env));
  cdr(pair) = arg;
  return arg;
}

object *edit (object *fun) {
  while (1) {
    if (tstflag(EXITEDITOR)) return fun;
    char c = gserial();
    if (c == 'q') setflag(EXITEDITOR);
    else if (c == 'b') return fun;
    else if (c == 'r') fun = read(gserial);
    else if (c == '\n') { pfl(pserial); superprint(fun, 0, pserial); pln(pserial); }
    else if (c == 'c') fun = cons(read(gserial), fun);
    else if (atom(fun)) pserial('!');
    else if (c == 'd') fun = cons(car(fun), edit(cdr(fun)));
    else if (c == 'a') fun = cons(edit(car(fun)), cdr(fun));
    else if (c == 'x') fun = cdr(fun);
    else pserial('?');
  }
}

// Pretty printer

object *fn_pprint (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  #if defined(gfxsupport)
  if (pfun == gfxwrite) ppwidth = GFXPPWIDTH;
  #endif
  pln(pfun);
  superprint(obj, 0, pfun);
  ppwidth = PPWIDTH;
  return symbol(NOTHING);
}

object *fn_pprintall (object *args, object *env) {
  (void) env;
  pfun_t pfun = pstreamfun(args);
  #if defined(gfxsupport)
  if (pfun == gfxwrite) ppwidth = GFXPPWIDTH;
  #endif
  object *globals = GlobalEnv;
  while (globals != NULL) {
    object *pair = first(globals);
    object *var = car(pair);
    object *val = cdr(pair);
    pln(pfun);
    if (consp(val) && symbolp(car(val)) && car(val)->name == LAMBDA) {
      superprint(cons(symbol(DEFUN), cons(var, cdr(val))), 0, pfun);
    } else {
      superprint(cons(symbol(DEFVAR), cons(var, cons(quote(val), NULL))), 0, pserial);
    }
    pln(pfun);
    testescape();
    globals = cdr(globals);
  }
  ppwidth = PPWIDTH;
  return symbol(NOTHING);
}

// Format

void formaterr (object *formatstr, PGM_P string, uint8_t p) {
  pln(pserial); indent(4, ' ', pserial); printstring(formatstr, pserial); pln(pserial);
  indent(p+5, ' ', pserial); pserial('^');
  errorsub(FORMAT, string);
  pln(pserial);
  GCStack = NULL;
  longjmp(exception, 1);
}

object *fn_format (object *args, object *env) {
  (void) env;
  pfun_t pfun = pserial;
  object *output = first(args);
  object *obj;
  if (output == nil) { obj = startstring(FORMAT); pfun = pstr; }
  else if (output != tee) pfun = pstreamfun(args);
  object *formatstr = second(args);
  if (!stringp(formatstr)) error(FORMAT, notastring, formatstr);
  object *save = NULL;
  args = cddr(args);
  int len = stringlength(formatstr);
  uint8_t n = 0, width = 0, w, bra = 0;
  char pad = ' ';
  bool tilde = false, mute = false, comma, quote;
  while (n < len) {
    char ch = nthchar(formatstr, n);
    char ch2 = ch & ~0x20; // force to upper case
    if (tilde) {
     if (ch == '}') {
        if (save == NULL) formaterr(formatstr, PSTR("no matching ~{"), n);
        if (args == NULL) { args = cdr(save); save = NULL; } else n = bra; 
        mute = false; tilde = false;
      }      
      else if (!mute) {
        if (comma && quote) { pad = ch; comma = false, quote = false; }
        else if (ch == '\'') {
          if (comma) quote = true; 
          else formaterr(formatstr, PSTR("quote not valid"), n);
        }
        else if (ch == '~') { pfun('~'); tilde = false; }
        else if (ch >= '0' && ch <= '9') width = width*10 + ch - '0';
        else if (ch == ',') comma = true;
        else if (ch == '%') { pln(pfun); tilde = false; }
        else if (ch == '&') { pfl(pfun); tilde = false; }
        else if (ch == '^') {
          if (save != NULL && args == NULL) mute = true;
          tilde = false;
        }
        else if (ch == '{') {
          if (save != NULL) formaterr(formatstr, PSTR("can't nest ~{"), n);
          if (args == NULL) formaterr(formatstr, noargument, n);
          if (!listp(first(args))) formaterr(formatstr, notalist, n);
          save = args; args = first(args); bra = n; tilde = false;
          if (args == NULL) mute = true;
        }
        else if (ch2 == 'A' || ch2 == 'S' || ch2 == 'D' || ch2 == 'G' || ch2 == 'X' || ch2 == 'B') {
          if (args == NULL) formaterr(formatstr, noargument, n);
          object *arg = first(args); args = cdr(args);
          uint8_t aw = atomwidth(arg);
          if (width < aw) w = 0; else w = width-aw;
          tilde = false;
          if (ch2 == 'A') { prin1object(arg, pfun); indent(w, pad, pfun); }
          else if (ch2 == 'S') { printobject(arg, pfun); indent(w, pad, pfun); }
          else if (ch2 == 'D' || ch2 == 'G') { indent(w, pad, pfun); prin1object(arg, pfun); }
          else if (ch2 == 'X' || ch2 == 'B') {
            if (integerp(arg)) {
              uint8_t power2 = (ch2 == 'B') ? 1 : 4;
              uint8_t hw = basewidth(arg, power2); if (width < hw) w = 0; else w = width-hw;
              indent(w, pad, pfun); pintbase(arg->integer, power2, pfun);
            } else {
              indent(w, pad, pfun); prin1object(arg, pfun);
            }
          }
          tilde = false;
        } else formaterr(formatstr, PSTR("invalid directive"), n);
      }
    } else {
      if (ch == '~') { tilde = true; pad = ' '; width = 0; comma = false; quote = false; }
      else if (!mute) pfun(ch);
    }
    n++;
  }
  if (output == nil) { obj->cdr = GlobalString; return obj; }
  else return nil;
}

// LispLibrary

object *fn_require (object *args, object *env) {
  object *arg = first(args);
  object *globals = GlobalEnv;
  if (!symbolp(arg)) error(REQUIRE, notasymbol, arg);
  while (globals != NULL) {
    object *pair = first(globals);
    object *var = car(pair);
    if (symbolp(var) && var == arg) return nil;
    globals = cdr(globals);
  }
  GlobalStringIndex = 0;
  object *line = read(glibrary);
  while (line != NULL) {
    // Is this the definition we want
    int fname = first(line)->name;
    if ((fname == DEFUN || fname == DEFVAR) && symbolp(second(line)) && second(line)->name == arg->name) {
      eval(line, env);
      return tee;
    }
    line = read(glibrary);
  }
  return nil;
}

object *fn_listlibrary (object *args, object *env) {
  (void) args, (void) env;
  GlobalStringIndex = 0;
  object *line = read(glibrary);
  while (line != NULL) {
    int fname = first(line)->name;
    if (fname == DEFUN || fname == DEFVAR) {
      pstring(symbolname(second(line)->name), pserial); pserial(' ');
    }
    line = read(glibrary);
  }
  return symbol(NOTHING);
}

// Wi-fi

object *fn_available (object *args, object *env) {
  (void) env;
  if (isstream(first(args))>>8 != WIFISTREAM) error2(AVAILABLE, PSTR("invalid stream"));
  return number(client.available());
}

object *fn_wifiserver (object *args, object *env) {
  (void) args, (void) env;
  server.begin();
  return nil;
}

object *fn_wifisoftap (object *args, object *env) {
  (void) env;
  char ssid[33], pass[65];
  if (args == NULL) return WiFi.softAPdisconnect(true) ? tee : nil;
  object *first = first(args); args = cdr(args);
  if (args == NULL) WiFi.softAP(cstring(first, ssid, 33));
  else {
    object *second = first(args);
    args = cdr(args);
    int channel = 1;
    boolean hidden = false;
    if (args != NULL) {
      channel = checkinteger(WIFISOFTAP, first(args));
      args = cdr(args);
      if (args != NULL) hidden = (first(args) != nil);
    }
    WiFi.softAP(cstring(first, ssid, 33), cstring(second, pass, 65), channel, hidden);
  }
  return lispstring((char*)WiFi.softAPIP().toString().c_str());
}

object *fn_connected (object *args, object *env) {
  (void) env;
  if (isstream(first(args))>>8 != WIFISTREAM) error2(CONNECTED, PSTR("invalid stream"));
  return client.connected() ? tee : nil;
}

object *fn_wifilocalip (object *args, object *env) {
  (void) args, (void) env;
  return lispstring((char*)WiFi.localIP().toString().c_str());
}

object *fn_wificonnect (object *args, object *env) {
  (void) env;
  char ssid[33], pass[65];
  if (args == NULL) { WiFi.disconnect(true); return nil; }
  if (cdr(args) == NULL) WiFi.begin(cstring(first(args), ssid, 33));
  else WiFi.begin(cstring(first(args), ssid, 33), cstring(second(args), pass, 65));
  int result = WiFi.waitForConnectResult();
  if (result == WL_CONNECTED) return lispstring((char*)WiFi.localIP().toString().c_str());
  else if (result == WL_NO_SSID_AVAIL) error2(WIFICONNECT, PSTR("network not found"));
  else if (result == WL_CONNECT_FAILED) error2(WIFICONNECT, PSTR("connection failed"));
  else error2(WIFICONNECT, PSTR("unable to connect"));
  return nil;
}

// Graphics functions

object *fn_drawpixel (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t colour = COLOR_WHITE;
  if (cddr(args) != NULL) colour = checkinteger(DRAWPIXEL, third(args));
  tft.drawPixel(checkinteger(DRAWPIXEL, first(args)), checkinteger(DRAWPIXEL, second(args)), colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawline (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(DRAWLINE, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(DRAWLINE, car(args));
  tft.drawLine(params[0], params[1], params[2], params[3], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawrect (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(DRAWRECT, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(DRAWRECT, car(args));
  tft.drawRect(params[0], params[1], params[2], params[3], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_fillrect (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(FILLRECT, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(FILLRECT, car(args));
  tft.fillRect(params[0], params[1], params[2], params[3], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawcircle (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[3], colour = COLOR_WHITE;
  for (int i=0; i<3; i++) { params[i] = checkinteger(DRAWCIRCLE, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(DRAWCIRCLE, car(args));
  tft.drawCircle(params[0], params[1], params[2], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_fillcircle (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[3], colour = COLOR_WHITE;
  for (int i=0; i<3; i++) { params[i] = checkinteger(FILLCIRCLE, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(FILLCIRCLE, car(args));
  tft.fillCircle(params[0], params[1], params[2], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawroundrect (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[5], colour = COLOR_WHITE;
  for (int i=0; i<5; i++) { params[i] = checkinteger(DRAWROUNDRECT, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(DRAWROUNDRECT, car(args));
  tft.drawRoundRect(params[0], params[1], params[2], params[3], params[4], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_fillroundrect (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[5], colour = COLOR_WHITE;
  for (int i=0; i<5; i++) { params[i] = checkinteger(FILLROUNDRECT, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(FILLROUNDRECT, car(args));
  tft.fillRoundRect(params[0], params[1], params[2], params[3], params[4], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawtriangle (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[6], colour = COLOR_WHITE;
  for (int i=0; i<6; i++) { params[i] = checkinteger(DRAWTRIANGLE, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(DRAWTRIANGLE, car(args));
  tft.drawTriangle(params[0], params[1], params[2], params[3], params[4], params[5], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_filltriangle (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t params[6], colour = COLOR_WHITE;
  for (int i=0; i<6; i++) { params[i] = checkinteger(FILLTRIANGLE, car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(FILLTRIANGLE, car(args));
  tft.fillTriangle(params[0], params[1], params[2], params[3], params[4], params[5], colour);
  tft.display();
  #endif
  return nil;
}

object *fn_drawchar (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t colour = COLOR_WHITE, bg = COLOR_BLACK, size = 1;
  object *more = cdr(cddr(args));
  if (more != NULL) {
    colour = checkinteger(DRAWCHAR, car(more));
    more = cdr(more);
    if (more != NULL) {
      bg = checkinteger(DRAWCHAR, car(more));
      more = cdr(more);
      if (more != NULL) size = checkinteger(DRAWCHAR, car(more));
    }
  }
  tft.drawChar(checkinteger(DRAWCHAR, first(args)), checkinteger(DRAWCHAR, second(args)), checkchar(DRAWCHAR, third(args)),
    colour, bg, size);
  tft.display();
  #endif
  return nil;
}

object *fn_setcursor (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  tft.setCursor(checkinteger(SETCURSOR, first(args)), checkinteger(SETCURSOR, second(args)));
  #endif
  return nil;
}

object *fn_settextcolor (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  if (cdr(args) != NULL) tft.setTextColor(checkinteger(SETTEXTCOLOR, first(args)), checkinteger(SETTEXTCOLOR, second(args)));
  else tft.setTextColor(checkinteger(SETTEXTCOLOR, first(args)));
  #endif
  return nil;
}

object *fn_settextsize (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  tft.setTextSize(checkinteger(SETTEXTSIZE, first(args)));
  #endif
  return nil;
}

object *fn_settextwrap (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  tft.setTextWrap(first(args) != NULL);
  #endif
  return nil;
}

object *fn_fillscreen (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  uint16_t colour = COLOR_BLACK;
  if (args != NULL) colour = checkinteger(FILLSCREEN, first(args));
  tft.fillScreen(colour);
  tft.display();
  #endif
  return nil;
}

object *fn_setrotation (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  tft.setRotation(checkinteger(SETROTATION, first(args)));
  tft.display();
  #endif
  return nil;
}

object *fn_invertdisplay (object *args, object *env) {
  #if defined(gfxsupport)
  (void) env;
  tft.invertDisplay(first(args) != NULL);
  tft.display();
  #endif
  return nil;
}

// Insert your own function definitions here

// Built-in symbol names
const char string0[] PROGMEM = "nil";
const char string1[] PROGMEM = "t";
const char string2[] PROGMEM = "nothing";
const char string3[] PROGMEM = "&optional";
const char string4[] PROGMEM = ":initial-element";
const char string5[] PROGMEM = ":element-type";
const char string6[] PROGMEM = "bit";
const char string7[] PROGMEM = "&rest";
const char string8[] PROGMEM = "lambda";
const char string9[] PROGMEM = "let";
const char string10[] PROGMEM = "let*";
const char string11[] PROGMEM = "closure";
const char string12[] PROGMEM = "";
const char string13[] PROGMEM = "quote";
const char string14[] PROGMEM = "or";
const char string15[] PROGMEM = "defun";
const char string16[] PROGMEM = "defvar";
const char string17[] PROGMEM = "setq";
const char string18[] PROGMEM = "loop";
const char string19[] PROGMEM = "return";
const char string20[] PROGMEM = "push";
const char string21[] PROGMEM = "pop";
const char string22[] PROGMEM = "incf";
const char string23[] PROGMEM = "decf";
const char string24[] PROGMEM = "setf";
const char string25[] PROGMEM = "dolist";
const char string26[] PROGMEM = "dotimes";
const char string27[] PROGMEM = "trace";
const char string28[] PROGMEM = "untrace";
const char string29[] PROGMEM = "for-millis";
const char string30[] PROGMEM = "with-output-to-string";
const char string31[] PROGMEM = "with-serial";
const char string32[] PROGMEM = "with-i2c";
const char string33[] PROGMEM = "with-spi";
const char string34[] PROGMEM = "with-sd-card";
const char string35[] PROGMEM = "with-gfx";
const char string36[] PROGMEM = "with-client";
const char string37[] PROGMEM = "";
const char string38[] PROGMEM = "progn";
const char string39[] PROGMEM = "if";
const char string40[] PROGMEM = "cond";
const char string41[] PROGMEM = "when";
const char string42[] PROGMEM = "unless";
const char string43[] PROGMEM = "case";
const char string44[] PROGMEM = "and";
const char string45[] PROGMEM = "";
const char string46[] PROGMEM = "not";
const char string47[] PROGMEM = "null";
const char string48[] PROGMEM = "cons";
const char string49[] PROGMEM = "atom";
const char string50[] PROGMEM = "listp";
const char string51[] PROGMEM = "consp";
const char string52[] PROGMEM = "symbolp";
const char string53[] PROGMEM = "arrayp";
const char string54[] PROGMEM = "boundp";
const char string55[] PROGMEM = "set";
const char string56[] PROGMEM = "streamp";
const char string57[] PROGMEM = "eq";
const char string58[] PROGMEM = "car";
const char string59[] PROGMEM = "first";
const char string60[] PROGMEM = "cdr";
const char string61[] PROGMEM = "rest";
const char string62[] PROGMEM = "caar";
const char string63[] PROGMEM = "cadr";
const char string64[] PROGMEM = "second";
const char string65[] PROGMEM = "cdar";
const char string66[] PROGMEM = "cddr";
const char string67[] PROGMEM = "caaar";
const char string68[] PROGMEM = "caadr";
const char string69[] PROGMEM = "cadar";
const char string70[] PROGMEM = "caddr";
const char string71[] PROGMEM = "third";
const char string72[] PROGMEM = "cdaar";
const char string73[] PROGMEM = "cdadr";
const char string74[] PROGMEM = "cddar";
const char string75[] PROGMEM = "cdddr";
const char string76[] PROGMEM = "length";
const char string77[] PROGMEM = "array-dimensions";
const char string78[] PROGMEM = "list";
const char string79[] PROGMEM = "make-array";
const char string80[] PROGMEM = "reverse";
const char string81[] PROGMEM = "nth";
const char string82[] PROGMEM = "aref";
const char string83[] PROGMEM = "assoc";
const char string84[] PROGMEM = "member";
const char string85[] PROGMEM = "apply";
const char string86[] PROGMEM = "funcall";
const char string87[] PROGMEM = "append";
const char string88[] PROGMEM = "mapc";
const char string89[] PROGMEM = "mapcar";
const char string90[] PROGMEM = "mapcan";
const char string91[] PROGMEM = "+";
const char string92[] PROGMEM = "-";
const char string93[] PROGMEM = "*";
const char string94[] PROGMEM = "/";
const char string95[] PROGMEM = "mod";
const char string96[] PROGMEM = "1+";
const char string97[] PROGMEM = "1-";
const char string98[] PROGMEM = "abs";
const char string99[] PROGMEM = "random";
const char string100[] PROGMEM = "max";
const char string101[] PROGMEM = "min";
const char string102[] PROGMEM = "/=";
const char string103[] PROGMEM = "=";
const char string104[] PROGMEM = "<";
const char string105[] PROGMEM = "<=";
const char string106[] PROGMEM = ">";
const char string107[] PROGMEM = ">=";
const char string108[] PROGMEM = "plusp";
const char string109[] PROGMEM = "minusp";
const char string110[] PROGMEM = "zerop";
const char string111[] PROGMEM = "oddp";
const char string112[] PROGMEM = "evenp";
const char string113[] PROGMEM = "integerp";
const char string114[] PROGMEM = "numberp";
const char string115[] PROGMEM = "float";
const char string116[] PROGMEM = "floatp";
const char string117[] PROGMEM = "sin";
const char string118[] PROGMEM = "cos";
const char string119[] PROGMEM = "tan";
const char string120[] PROGMEM = "asin";
const char string121[] PROGMEM = "acos";
const char string122[] PROGMEM = "atan";
const char string123[] PROGMEM = "sinh";
const char string124[] PROGMEM = "cosh";
const char string125[] PROGMEM = "tanh";
const char string126[] PROGMEM = "exp";
const char string127[] PROGMEM = "sqrt";
const char string128[] PROGMEM = "log";
const char string129[] PROGMEM = "expt";
const char string130[] PROGMEM = "ceiling";
const char string131[] PROGMEM = "floor";
const char string132[] PROGMEM = "truncate";
const char string133[] PROGMEM = "round";
const char string134[] PROGMEM = "char";
const char string135[] PROGMEM = "char-code";
const char string136[] PROGMEM = "code-char";
const char string137[] PROGMEM = "characterp";
const char string138[] PROGMEM = "stringp";
const char string139[] PROGMEM = "string=";
const char string140[] PROGMEM = "string<";
const char string141[] PROGMEM = "string>";
const char string142[] PROGMEM = "sort";
const char string143[] PROGMEM = "string";
const char string144[] PROGMEM = "concatenate";
const char string145[] PROGMEM = "subseq";
const char string146[] PROGMEM = "read-from-string";
const char string147[] PROGMEM = "princ-to-string";
const char string148[] PROGMEM = "prin1-to-string";
const char string149[] PROGMEM = "logand";
const char string150[] PROGMEM = "logior";
const char string151[] PROGMEM = "logxor";
const char string152[] PROGMEM = "lognot";
const char string153[] PROGMEM = "ash";
const char string154[] PROGMEM = "logbitp";
const char string155[] PROGMEM = "eval";
const char string156[] PROGMEM = "globals";
const char string157[] PROGMEM = "locals";
const char string158[] PROGMEM = "makunbound";
const char string159[] PROGMEM = "break";
const char string160[] PROGMEM = "read";
const char string161[] PROGMEM = "prin1";
const char string162[] PROGMEM = "print";
const char string163[] PROGMEM = "princ";
const char string164[] PROGMEM = "terpri";
const char string165[] PROGMEM = "read-byte";
const char string166[] PROGMEM = "read-line";
const char string167[] PROGMEM = "write-byte";
const char string168[] PROGMEM = "write-string";
const char string169[] PROGMEM = "write-line";
const char string170[] PROGMEM = "restart-i2c";
const char string171[] PROGMEM = "gc";
const char string172[] PROGMEM = "room";
const char string173[] PROGMEM = "save-image";
const char string174[] PROGMEM = "load-image";
const char string175[] PROGMEM = "cls";
const char string176[] PROGMEM = "pinmode";
const char string177[] PROGMEM = "digitalread";
const char string178[] PROGMEM = "digitalwrite";
const char string179[] PROGMEM = "analogread";
const char string180[] PROGMEM = "analogreadresolution";
const char string181[] PROGMEM = "analogwrite";
const char string182[] PROGMEM = "delay";
const char string183[] PROGMEM = "millis";
const char string184[] PROGMEM = "sleep";
const char string185[] PROGMEM = "note";
const char string186[] PROGMEM = "edit";
const char string187[] PROGMEM = "pprint";
const char string188[] PROGMEM = "pprintall";
const char string189[] PROGMEM = "format";
const char string190[] PROGMEM = "require";
const char string191[] PROGMEM = "list-library";
const char string192[] PROGMEM = "available";
const char string193[] PROGMEM = "wifi-server";
const char string194[] PROGMEM = "wifi-softap";
const char string195[] PROGMEM = "connected";
const char string196[] PROGMEM = "wifi-localip";
const char string197[] PROGMEM = "wifi-connect";
const char string198[] PROGMEM = "draw-pixel";
const char string199[] PROGMEM = "draw-line";
const char string200[] PROGMEM = "draw-rect";
const char string201[] PROGMEM = "fill-rect";
const char string202[] PROGMEM = "draw-circle";
const char string203[] PROGMEM = "fill-circle";
const char string204[] PROGMEM = "draw-round-rect";
const char string205[] PROGMEM = "fill-round-rect";
const char string206[] PROGMEM = "draw-triangle";
const char string207[] PROGMEM = "fill-triangle";
const char string208[] PROGMEM = "draw-char";
const char string209[] PROGMEM = "set-cursor";
const char string210[] PROGMEM = "set-text-color";
const char string211[] PROGMEM = "set-text-size";
const char string212[] PROGMEM = "set-text-wrap";
const char string213[] PROGMEM = "fill-screen";
const char string214[] PROGMEM = "set-rotation";
const char string215[] PROGMEM = "invert-display";
const char string216[] PROGMEM = "";
const char string217[] PROGMEM = ":high";
const char string218[] PROGMEM = ":low";
#if defined(ESP8266)
const char string219[] PROGMEM = ":input";
const char string220[] PROGMEM = ":input-pullup";
const char string221[] PROGMEM = ":output";
const char string222[] PROGMEM = "";
#elif defined(ESP32)
const char string219[] PROGMEM = ":input";
const char string220[] PROGMEM = ":input-pullup";
const char string221[] PROGMEM = ":input-pulldown";
const char string222[] PROGMEM = ":output";
const char string223[] PROGMEM = "";
#endif

// Insert your own function names here

// Built-in symbol lookup table
const tbl_entry_t lookup_table[] PROGMEM = {
  { string0, NULL, 0x00 },
  { string1, NULL, 0x00 },
  { string2, NULL, 0x00 },
  { string3, NULL, 0x00 },
  { string4, NULL, 0x00 },
  { string5, NULL, 0x00 },
  { string6, NULL, 0x00 },
  { string7, NULL, 0x00 },
  { string8, NULL, 0x0F },
  { string9, NULL, 0x0F },
  { string10, NULL, 0x0F },
  { string11, NULL, 0x0F },
  { string12, NULL, 0x00 },
  { string13, sp_quote, 0x11 },
  { string14, sp_or, 0x0F },
  { string15, sp_defun, 0x2F },
  { string16, sp_defvar, 0x12 },
  { string17, sp_setq, 0x2F },
  { string18, sp_loop, 0x0F },
  { string19, sp_return, 0x0F },
  { string20, sp_push, 0x22 },
  { string21, sp_pop, 0x11 },
  { string22, sp_incf, 0x12 },
  { string23, sp_decf, 0x12 },
  { string24, sp_setf, 0x2F },
  { string25, sp_dolist, 0x1F },
  { string26, sp_dotimes, 0x1F },
  { string27, sp_trace, 0x01 },
  { string28, sp_untrace, 0x01 },
  { string29, sp_formillis, 0x1F },
  { string30, sp_withoutputtostring, 0x1F },
  { string31, sp_withserial, 0x1F },
  { string32, sp_withi2c, 0x1F },
  { string33, sp_withspi, 0x1F },
  { string34, sp_withsdcard, 0x2F },
  { string35, sp_withgfx, 0x1F },
  { string36, sp_withclient, 0x12 },
  { string37, NULL, 0x00 },
  { string38, tf_progn, 0x0F },
  { string39, tf_if, 0x23 },
  { string40, tf_cond, 0x0F },
  { string41, tf_when, 0x1F },
  { string42, tf_unless, 0x1F },
  { string43, tf_case, 0x1F },
  { string44, tf_and, 0x0F },
  { string45, NULL, 0x00 },
  { string46, fn_not, 0x11 },
  { string47, fn_not, 0x11 },
  { string48, fn_cons, 0x22 },
  { string49, fn_atom, 0x11 },
  { string50, fn_listp, 0x11 },
  { string51, fn_consp, 0x11 },
  { string52, fn_symbolp, 0x11 },
  { string53, fn_arrayp, 0x11 },
  { string54, fn_boundp, 0x11 },
  { string55, fn_setfn, 0x2F },
  { string56, fn_streamp, 0x11 },
  { string57, fn_eq, 0x22 },
  { string58, fn_car, 0x11 },
  { string59, fn_car, 0x11 },
  { string60, fn_cdr, 0x11 },
  { string61, fn_cdr, 0x11 },
  { string62, fn_caar, 0x11 },
  { string63, fn_cadr, 0x11 },
  { string64, fn_cadr, 0x11 },
  { string65, fn_cdar, 0x11 },
  { string66, fn_cddr, 0x11 },
  { string67, fn_caaar, 0x11 },
  { string68, fn_caadr, 0x11 },
  { string69, fn_cadar, 0x11 },
  { string70, fn_caddr, 0x11 },
  { string71, fn_caddr, 0x11 },
  { string72, fn_cdaar, 0x11 },
  { string73, fn_cdadr, 0x11 },
  { string74, fn_cddar, 0x11 },
  { string75, fn_cdddr, 0x11 },
  { string76, fn_length, 0x11 },
  { string77, fn_arraydimensions, 0x11 },
  { string78, fn_list, 0x0F },
  { string79, fn_makearray, 0x15 },
  { string80, fn_reverse, 0x11 },
  { string81, fn_nth, 0x22 },
  { string82, fn_aref, 0x2F },
  { string83, fn_assoc, 0x22 },
  { string84, fn_member, 0x22 },
  { string85, fn_apply, 0x2F },
  { string86, fn_funcall, 0x1F },
  { string87, fn_append, 0x0F },
  { string88, fn_mapc, 0x2F },
  { string89, fn_mapcar, 0x2F },
  { string90, fn_mapcan, 0x2F },
  { string91, fn_add, 0x0F },
  { string92, fn_subtract, 0x1F },
  { string93, fn_multiply, 0x0F },
  { string94, fn_divide, 0x1F },
  { string95, fn_mod, 0x22 },
  { string96, fn_oneplus, 0x11 },
  { string97, fn_oneminus, 0x11 },
  { string98, fn_abs, 0x11 },
  { string99, fn_random, 0x11 },
  { string100, fn_maxfn, 0x1F },
  { string101, fn_minfn, 0x1F },
  { string102, fn_noteq, 0x1F },
  { string103, fn_numeq, 0x1F },
  { string104, fn_less, 0x1F },
  { string105, fn_lesseq, 0x1F },
  { string106, fn_greater, 0x1F },
  { string107, fn_greatereq, 0x1F },
  { string108, fn_plusp, 0x11 },
  { string109, fn_minusp, 0x11 },
  { string110, fn_zerop, 0x11 },
  { string111, fn_oddp, 0x11 },
  { string112, fn_evenp, 0x11 },
  { string113, fn_integerp, 0x11 },
  { string114, fn_numberp, 0x11 },
  { string115, fn_floatfn, 0x11 },
  { string116, fn_floatp, 0x11 },
  { string117, fn_sin, 0x11 },
  { string118, fn_cos, 0x11 },
  { string119, fn_tan, 0x11 },
  { string120, fn_asin, 0x11 },
  { string121, fn_acos, 0x11 },
  { string122, fn_atan, 0x12 },
  { string123, fn_sinh, 0x11 },
  { string124, fn_cosh, 0x11 },
  { string125, fn_tanh, 0x11 },
  { string126, fn_exp, 0x11 },
  { string127, fn_sqrt, 0x11 },
  { string128, fn_log, 0x12 },
  { string129, fn_expt, 0x22 },
  { string130, fn_ceiling, 0x12 },
  { string131, fn_floor, 0x12 },
  { string132, fn_truncate, 0x12 },
  { string133, fn_round, 0x12 },
  { string134, fn_char, 0x22 },
  { string135, fn_charcode, 0x11 },
  { string136, fn_codechar, 0x11 },
  { string137, fn_characterp, 0x11 },
  { string138, fn_stringp, 0x11 },
  { string139, fn_stringeq, 0x22 },
  { string140, fn_stringless, 0x22 },
  { string141, fn_stringgreater, 0x22 },
  { string142, fn_sort, 0x22 },
  { string143, fn_stringfn, 0x11 },
  { string144, fn_concatenate, 0x1F },
  { string145, fn_subseq, 0x23 },
  { string146, fn_readfromstring, 0x11 },
  { string147, fn_princtostring, 0x11 },
  { string148, fn_prin1tostring, 0x11 },
  { string149, fn_logand, 0x0F },
  { string150, fn_logior, 0x0F },
  { string151, fn_logxor, 0x0F },
  { string152, fn_lognot, 0x11 },
  { string153, fn_ash, 0x22 },
  { string154, fn_logbitp, 0x22 },
  { string155, fn_eval, 0x11 },
  { string156, fn_globals, 0x00 },
  { string157, fn_locals, 0x00 },
  { string158, fn_makunbound, 0x11 },
  { string159, fn_break, 0x00 },
  { string160, fn_read, 0x01 },
  { string161, fn_prin1, 0x12 },
  { string162, fn_print, 0x12 },
  { string163, fn_princ, 0x12 },
  { string164, fn_terpri, 0x01 },
  { string165, fn_readbyte, 0x02 },
  { string166, fn_readline, 0x01 },
  { string167, fn_writebyte, 0x12 },
  { string168, fn_writestring, 0x12 },
  { string169, fn_writeline, 0x12 },
  { string170, fn_restarti2c, 0x12 },
  { string171, fn_gc, 0x00 },
  { string172, fn_room, 0x00 },
  { string173, fn_saveimage, 0x01 },
  { string174, fn_loadimage, 0x01 },
  { string175, fn_cls, 0x00 },
  { string176, fn_pinmode, 0x22 },
  { string177, fn_digitalread, 0x11 },
  { string178, fn_digitalwrite, 0x22 },
  { string179, fn_analogread, 0x11 },
  { string180, fn_analogreadresolution, 0x11 },
  { string181, fn_analogwrite, 0x22 },
  { string182, fn_delay, 0x11 },
  { string183, fn_millis, 0x00 },
  { string184, fn_sleep, 0x11 },
  { string185, fn_note, 0x03 },
  { string186, fn_edit, 0x11 },
  { string187, fn_pprint, 0x12 },
  { string188, fn_pprintall, 0x01 },
  { string189, fn_format, 0x2F },
  { string190, fn_require, 0x11 },
  { string191, fn_listlibrary, 0x00 },
  { string192, fn_available, 0x11 },
  { string193, fn_wifiserver, 0x00 },
  { string194, fn_wifisoftap, 0x04 },
  { string195, fn_connected, 0x11 },
  { string196, fn_wifilocalip, 0x00 },
  { string197, fn_wificonnect, 0x02 },
  { string198, fn_drawpixel, 0x23 },
  { string199, fn_drawline, 0x45 },
  { string200, fn_drawrect, 0x45 },
  { string201, fn_fillrect, 0x45 },
  { string202, fn_drawcircle, 0x34 },
  { string203, fn_fillcircle, 0x34 },
  { string204, fn_drawroundrect, 0x56 },
  { string205, fn_fillroundrect, 0x56 },
  { string206, fn_drawtriangle, 0x67 },
  { string207, fn_filltriangle, 0x67 },
  { string208, fn_drawchar, 0x36 },
  { string209, fn_setcursor, 0x22 },
  { string210, fn_settextcolor, 0x12 },
  { string211, fn_settextsize, 0x11 },
  { string212, fn_settextwrap, 0x11 },
  { string213, fn_fillscreen, 0x01 },
  { string214, fn_setrotation, 0x11 },
  { string215, fn_invertdisplay, 0x11 },
  { string216, NULL, 0x00 },
  { string217, (fn_ptr_type)HIGH, DIGITALWRITE },
  { string218, (fn_ptr_type)LOW, DIGITALWRITE },
#if defined(ESP8266)
  { string219, (fn_ptr_type)INPUT, PINMODE },
  { string220, (fn_ptr_type)INPUT_PULLUP, PINMODE },
  { string221, (fn_ptr_type)OUTPUT, PINMODE },
  { string222, NULL, 0x00 },
#elif defined(ESP32)
  { string219, (fn_ptr_type)INPUT, PINMODE },
  { string220, (fn_ptr_type)INPUT_PULLUP, PINMODE },
  { string221, (fn_ptr_type)INPUT_PULLDOWN, PINMODE },
  { string222, (fn_ptr_type)OUTPUT, PINMODE },
  { string223, NULL, 0x00 },
#endif

// Insert your own table entries here

};

// Table lookup functions

int builtin (char* n) {
  int entry = 0;
  while (entry < ENDFUNCTIONS) {
    if (strcasecmp_P(n, lookup_table[entry].string) == 0)
      return entry;
    entry++;
  }
  return ENDFUNCTIONS;
}

int longsymbol (char *buffer) {
  char *p = SymbolTable;
  int i = 0;
  while (strcasecmp(p, buffer) != 0) {p = p + strlen(p) + 1; i++; }
  if (p == buffer) {
    // Add to symbol table?
    char *newtop = SymbolTop + strlen(p) + 1;
    if (SYMBOLTABLESIZE - (newtop - SymbolTable) < BUFFERSIZE) error2(0, PSTR("no room for long symbols"));
    SymbolTop = newtop;
  }
  if (i > 1535) error2(0, PSTR("too many long symbols"));
  return i + MAXSYMBOL; // First number unused by radix40
}

intptr_t lookupfn (symbol_t name) {
  return (intptr_t)pgm_read_ptr(&lookup_table[name].fptr);
}

uint8_t getminmax (symbol_t name) {
  uint8_t minmax = pgm_read_byte(&lookup_table[name].minmax);
  return minmax;
}

void checkminmax (symbol_t name, int nargs) {
  uint8_t minmax = getminmax(name);
  if (nargs<(minmax >> 4)) error2(name, toofewargs);
  if ((minmax & 0x0f) != 0x0f && nargs>(minmax & 0x0f)) error2(name, toomanyargs);
}

char *lookupbuiltin (symbol_t name) {
  char *buffer = SymbolTop;
  strcpy_P(buffer, lookup_table[name].string);
  return buffer;
}

char *lookupsymbol (symbol_t name) {
  char *p = SymbolTable;
  int i = name - MAXSYMBOL;
  while (i > 0 && p < SymbolTop) {p = p + strlen(p) + 1; i--; }
  if (p == SymbolTop) return NULL; else return p;
}

void deletesymbol (symbol_t name) {
  char *p = lookupsymbol(name);
  if (p == NULL) return;
  char *q = p + strlen(p) + 1;
  *p = '\0'; p++;
  while (q < SymbolTop) *(p++) = *(q++);
  SymbolTop = p;
}

void testescape () {
  if (Serial.read() == '~') error2(0, PSTR("escape!"));
}

// Main evaluator

object *eval (object *form, object *env) {
  int TC=0;
  EVAL:
  yield(); // Needed on ESP8266 to avoid Soft WDT Reset
  // Enough space?
  if (Freespace <= WORKSPACESIZE>>4) gc(form, env);
  // Escape
  if (tstflag(ESCAPE)) { clrflag(ESCAPE); error2(0, PSTR("Escape!"));}
  if (!tstflag(NOESC)) testescape();

  if (form == NULL) return nil;

  if (form->type >= NUMBER && form->type <= STRING) return form;

  if (symbolp(form)) {
    symbol_t name = form->name;
    object *pair = value(name, env);
    if (pair != NULL) return cdr(pair);
    pair = value(name, GlobalEnv);
    if (pair != NULL) return cdr(pair);
    else if (name < ENDFUNCTIONS) return form;
    error(0, PSTR("undefined"), form);
  }

  // It's a list
  object *function = car(form);
  object *args = cdr(form);

  if (function == NULL) error(0, PSTR("illegal function"), nil);
  if (!listp(args)) error(0, PSTR("can't evaluate a dotted pair"), args);

  // List starts with a symbol?
  if (symbolp(function)) {
    symbol_t name = function->name;

    if ((name == LET) || (name == LETSTAR)) {
      int TCstart = TC;
      if (args == NULL) error2(name, noargument);
      object *assigns = first(args);
      if (!listp(assigns)) error(name, notalist, assigns);
      object *forms = cdr(args);
      object *newenv = env;
      push(newenv, GCStack);
      while (assigns != NULL) {
        object *assign = car(assigns);
        if (!consp(assign)) push(cons(assign,nil), newenv);
        else if (cdr(assign) == NULL) push(cons(first(assign),nil), newenv);
        else push(cons(first(assign),eval(second(assign),env)), newenv);
        car(GCStack) = newenv;
        if (name == LETSTAR) env = newenv;
        assigns = cdr(assigns);
      }
      env = newenv;
      pop(GCStack);
      form = tf_progn(forms,env);
      TC = TCstart;
      goto EVAL;
    }

    if (name == LAMBDA) {
      if (env == NULL) return form;
      object *envcopy = NULL;
      while (env != NULL) {
        object *pair = first(env);
        if (pair != NULL) push(pair, envcopy);
        env = cdr(env);
      }
      return cons(symbol(CLOSURE), cons(envcopy,args));
    }

    if ((name > SPECIAL_FORMS) && (name < TAIL_FORMS)) {
      return ((fn_ptr_type)lookupfn(name))(args, env);
    }

    if ((name > TAIL_FORMS) && (name < FUNCTIONS)) {
      form = ((fn_ptr_type)lookupfn(name))(args, env);
      TC = 1;
      goto EVAL;
    }

    if ((name < SPECIAL_FORMS) || ((name > KEYWORDS) && (name < USERFUNCTIONS))) error2(name, PSTR("can't be used as a function"));
  }

  // Evaluate the parameters - result in head
  object *fname = car(form);
  int TCstart = TC;
  object *head = cons(eval(fname, env), NULL);
  push(head, GCStack); // Don't GC the result list
  object *tail = head;
  form = cdr(form);
  int nargs = 0;

  while (form != NULL){
    object *obj = cons(eval(car(form),env),NULL);
    cdr(tail) = obj;
    tail = obj;
    form = cdr(form);
    nargs++;
  }

  function = car(head);
  args = cdr(head);

  if (symbolp(function)) {
    symbol_t name = function->name;
    if (name >= ENDFUNCTIONS) error(0, PSTR("not valid here"), fname);
    checkminmax(name, nargs);
    object *result = ((fn_ptr_type)lookupfn(name))(args, env);
    pop(GCStack);
    return result;
  }

  if (consp(function)) {
    symbol_t name = 0;
    if (!listp(fname)) name = fname->name;

    if (issymbol(car(function), LAMBDA)) {
      form = closure(TCstart, name, NULL, cdr(function), args, &env);
      pop(GCStack);
      int trace = tracing(fname->name);
      if (trace) {
        object *result = eval(form, env);
        indent((--(TraceDepth[trace-1]))<<1, ' ', pserial);
        pint(TraceDepth[trace-1], pserial);
        pserial(':'); pserial(' ');
        printobject(fname, pserial); pfstring(PSTR(" returned "), pserial);
        printobject(result, pserial); pln(pserial);
        return result;
      } else {
        TC = 1;
        goto EVAL;
      }
    }

    if (issymbol(car(function), CLOSURE)) {
      function = cdr(function);
      form = closure(TCstart, name, car(function), cdr(function), args, &env);
      pop(GCStack);
      TC = 1;
      goto EVAL;
    }

  }
  error(0, PSTR("illegal function"), fname); return nil;
}

// Print functions

inline int maxbuffer (char *buffer) {
  return SYMBOLTABLESIZE-(buffer-SymbolTable)-1;
}

void pserial (char c) {
  LastPrint = c;
  if (c == '\n') Serial.write('\r');
  Serial.write(c);
}

const char ControlCodes[] PROGMEM = "Null\0SOH\0STX\0ETX\0EOT\0ENQ\0ACK\0Bell\0Backspace\0Tab\0Newline\0VT\0"
"Page\0Return\0SO\0SI\0DLE\0DC1\0DC2\0DC3\0DC4\0NAK\0SYN\0ETB\0CAN\0EM\0SUB\0Escape\0FS\0GS\0RS\0US\0Space\0";

void pcharacter (uint8_t c, pfun_t pfun) {
  if (!tstflag(PRINTREADABLY)) pfun(c);
  else {
    pfun('#'); pfun('\\');
    if (c <= 32) {
      PGM_P p = ControlCodes;
      while (c > 0) {p = p + strlen_P(p) + 1; c--; }
      pfstring(p, pfun);
    } else if (c < 127) pfun(c);
    else pint(c, pfun);
  }
}

void pstring (char *s, pfun_t pfun) {
  while (*s) pfun(*s++);
}

void printstring (object *form, pfun_t pfun) {
  if (tstflag(PRINTREADABLY)) pfun('"');
  form = cdr(form);
  while (form != NULL) {
    int chars = form->chars;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      char ch = chars>>i & 0xFF;
      if (tstflag(PRINTREADABLY) && (ch == '"' || ch == '\\')) pfun('\\');
      if (ch) pfun(ch);
    }
    form = car(form);
  }
  if (tstflag(PRINTREADABLY)) pfun('"');
}

void pfstring (PGM_P s, pfun_t pfun) {
  char str[strlen_P(s)+1];
  strcpy_P(str, s);
  pstring(str, pfun);
}

void pint (int i, pfun_t pfun) {
  int lead = 0;
  #if INT_MAX == 32767
  int p = 10000;
  #else
  int p = 1000000000;
  #endif
  if (i<0) pfun('-');
  for (int d=p; d>0; d=d/10) {
    int j = i/d;
    if (j!=0 || lead || d==1) { pfun(abs(j)+'0'); lead=1;}
    i = i - j*d;
  }
}

void pintbase (uint32_t i, uint8_t power2, pfun_t pfun) {
  int lead = 0;
  uint32_t p = 1<<(32-power2);
  for (uint32_t d=p; d>0; d=d>>power2) {
    uint32_t j = i/d;
    if (j!=0 || lead || d==1) { pfun((j<10) ? j+'0' : j+'W'); lead=1;}
    i = i - j*d;
  }
}

void pmantissa (float f, pfun_t pfun) {
  int sig = floor(log10(f));
  int mul = pow(10, 5 - sig);
  int i = round(f * mul);
  bool point = false;
  if (i == 1000000) { i = 100000; sig++; }
  if (sig < 0) {
    pfun('0'); pfun('.'); point = true;
    for (int j=0; j < - sig - 1; j++) pfun('0');
  }
  mul = 100000;
  for (int j=0; j<7; j++) {
    int d = (int)(i / mul);
    pfun(d + '0');
    i = i - d * mul;
    if (i == 0) {
      if (!point) {
        for (int k=j; k<sig; k++) pfun('0');
        pfun('.'); pfun('0');
      }
      return;
    }
    if (j == sig && sig >= 0) { pfun('.'); point = true; }
    mul = mul / 10;
  }
}

void pfloat (float f, pfun_t pfun) {
  if (isnan(f)) { pfstring(PSTR("NaN"), pfun); return; }
  if (f == 0.0) { pfun('0'); return; }
  if (isinf(f)) { pfstring(PSTR("Inf"), pfun); return; }
  if (f < 0) { pfun('-'); f = -f; }
  // Calculate exponent
  int e = 0;
  if (f < 1e-3 || f >= 1e5) {
    e = floor(log(f) / 2.302585); // log10 gives wrong result
    f = f / pow(10, e);
  }

  pmantissa (f, pfun);

  // Exponent
  if (e != 0) {
    pfun('e');
    pint(e, pfun);
  }
}

inline void pln (pfun_t pfun) {
  pfun('\n');
}

void pfl (pfun_t pfun) {
  if (LastPrint != '\n') pfun('\n');
}

void plist (object *form, pfun_t pfun) {
  pfun('(');
  printobject(car(form), pfun);
  form = cdr(form);
  while (form != NULL && listp(form)) {
    pfun(' ');
    printobject(car(form), pfun);
    form = cdr(form);
  }
  if (form != NULL) {
    pfstring(PSTR(" . "), pfun);
    printobject(form, pfun);
  }
  pfun(')');
}

void pstream (object *form, pfun_t pfun) {
  pfun('<');
  pfstring(streamname[(form->integer)>>8], pfun);
  pfstring(PSTR("-stream "), pfun);
  pint(form->integer & 0xFF, pfun);
  pfun('>');
}

void printobject (object *form, pfun_t pfun) {
  if (form == NULL) pfstring(PSTR("nil"), pfun);
  else if (listp(form) && issymbol(car(form), CLOSURE)) pfstring(PSTR("<closure>"), pfun);
  else if (listp(form)) plist(form, pfun);
  else if (integerp(form)) pint(form->integer, pfun);
  else if (floatp(form)) pfloat(form->single_float, pfun);
  else if (symbolp(form)) { if (form->name != NOTHING) pstring(symbolname(form->name), pfun); }
  else if (characterp(form)) pcharacter(form->chars, pfun);
  else if (stringp(form)) printstring(form, pfun);
  else if (arrayp(form)) printarray(form, pfun);
  else if (streamp(form)) pstream(form, pfun);
  else error2(0, PSTR("Error in print"));
}

void prin1object (object *form, pfun_t pfun) {
  char temp = Flags;
  clrflag(PRINTREADABLY);
  printobject(form, pfun);
  Flags = temp;
}

// Read functions

int glibrary () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
  char c = pgm_read_byte(&LispLibrary[GlobalStringIndex++]);
  return (c != 0) ? c : -1; // -1?
}

void loadfromlibrary (object *env) {
  GlobalStringIndex = 0;
  object *line = read(glibrary);
  while (line != NULL) {
    push(line, GCStack);
    eval(line, env);
    pop(GCStack);
    line = read(glibrary);
  }
}

// For line editor
const int TerminalWidth = 80;
volatile int WritePtr = 0, ReadPtr = 0;
const int KybdBufSize = 333; // 42*8 - 3
char KybdBuf[KybdBufSize];
volatile uint8_t KybdAvailable = 0;

// Parenthesis highlighting
void esc (int p, char c) {
  Serial.write('\e'); Serial.write('[');
  Serial.write((char)('0'+ p/100));
  Serial.write((char)('0'+ (p/10) % 10));
  Serial.write((char)('0'+ p % 10));
  Serial.write(c);
}

void hilight (char c) {
  Serial.write('\e'); Serial.write('['); Serial.write(c); Serial.write('m');
}

void Highlight (int p, int wp, uint8_t invert) {
  wp = wp + 2; // Prompt
#if defined (printfreespace)
  int f = Freespace;
  while (f) { wp++; f=f/10; }
#endif
  int line = wp/TerminalWidth;
  int col = wp%TerminalWidth;
  int targetline = (wp - p)/TerminalWidth;
  int targetcol = (wp - p)%TerminalWidth;
  int up = line-targetline, left = col-targetcol;
  if (p) {
    if (up) esc(up, 'A');
    if (col > targetcol) esc(left, 'D'); else esc(-left, 'C');
    if (invert) hilight('7');
    Serial.write('('); Serial.write('\b');
    // Go back
    if (up) esc(up, 'B'); // Down
    if (col > targetcol) esc(left, 'C'); else esc(-left, 'D');
    Serial.write('\b'); Serial.write(')');
    if (invert) hilight('0');
  }
}

void processkey (char c) {
  if (c == 27) { setflag(ESCAPE); return; }    // Escape key
#if defined(vt100)
  static int parenthesis = 0, wp = 0;
  // Undo previous parenthesis highlight
  Highlight(parenthesis, wp, 0);
  parenthesis = 0;
#endif
  // Edit buffer
  if (c == '\n' || c == '\r') {
    pserial('\n');
    KybdAvailable = 1;
    ReadPtr = 0;
    return;
  }
  if (c == 8 || c == 0x7f) {     // Backspace key
    if (WritePtr > 0) {
      WritePtr--;
      Serial.write(8); Serial.write(' '); Serial.write(8);
      if (WritePtr) c = KybdBuf[WritePtr-1];
    }
  } else if (WritePtr < KybdBufSize) {
    KybdBuf[WritePtr++] = c;
    Serial.write(c);
  }
#if defined(vt100)
  // Do new parenthesis highlight
  if (c == ')') {
    int search = WritePtr-1, level = 0;
    while (search >= 0 && parenthesis == 0) {
      c = KybdBuf[search--];
      if (c == ')') level++;
      if (c == '(') {
        level--;
        if (level == 0) {parenthesis = WritePtr-search-1; wp = WritePtr; }
      }
    }
    Highlight(parenthesis, wp, 1);
  }
#endif
  return;
}

int gserial () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
#if defined(lineeditor)
  while (!KybdAvailable) {
    while (!Serial.available());
    char temp = Serial.read();
    processkey(temp);
  }
  if (ReadPtr != WritePtr) return KybdBuf[ReadPtr++];
  KybdAvailable = 0;
  WritePtr = 0;
  return '\n';
#else
  unsigned long start = millis();
  while (!Serial.available()) if (millis() - start > 1000) clrflag(NOECHO);
  char temp = Serial.read();
  if (temp != '\n' && !tstflag(NOECHO)) pserial(temp);
  return temp;
#endif
}

object *nextitem (gfun_t gfun) {
  int ch = gfun();
  while(issp(ch)) ch = gfun();

  if (ch == ';') {
    do { ch = gfun(); if (ch == ';' || ch == '(') setflag(NOECHO); }
    while(ch != '(');
  }
  if (ch == '\n') ch = gfun();
  if (ch == -1) return nil;
  if (ch == ')') return (object *)KET;
  if (ch == '(') return (object *)BRA;
  if (ch == '\'') return (object *)QUO;

  // Parse string
  if (ch == '"') return readstring('"', gfun);

  // Parse symbol, character, or number
  int index = 0, base = 10, sign = 1;
  char *buffer = SymbolTop;
  int bufmax = maxbuffer(buffer); // Max index
  unsigned int result = 0;
  bool isfloat = false;
  float fresult = 0.0;

  if (ch == '+') {
    buffer[index++] = ch;
    ch = gfun();
  } else if (ch == '-') {
    sign = -1;
    buffer[index++] = ch;
    ch = gfun();
  } else if (ch == '.') {
    buffer[index++] = ch;
    ch = gfun();
    if (ch == ' ') return (object *)DOT;
    isfloat = true;
  }

  // Parse reader macros
  else if (ch == '#') {
    ch = gfun();
    char ch2 = ch & ~0x20; // force to upper case
    if (ch == '\\') { // Character
      base = 0; ch = gfun();
      if (issp(ch) || ch == ')' || ch == '(') return character(ch);
      else LastChar = ch;
    } else if (ch == '|') {
      do { while (gfun() != '|'); }
      while (gfun() != '#');
      return nextitem(gfun);
    } else if (ch2 == 'B') base = 2;
    else if (ch2 == 'O') base = 8;
    else if (ch2 == 'X') base = 16;
    else if (ch == '\'') return nextitem(gfun);
    else if (ch == '.') {
      setflag(NOESC);
      object *result = eval(read(gfun), NULL);
      clrflag(NOESC);
      return result;
    }
    else if (ch == '(') { LastChar = ch; return readarray(1, read(gfun)); }
    else if (ch == '*') return readbitarray(gfun);
    else if (ch >= '1' && ch <= '9' && (gfun() & ~0x20) == 'A') return readarray(ch - '0', read(gfun));
    else error2(0, PSTR("illegal character after #"));
    ch = gfun();
  }
  int valid; // 0=undecided, -1=invalid, +1=valid
  if (ch == '.') valid = 0; else if (digitvalue(ch)<base) valid = 1; else valid = -1;
  bool isexponent = false;
  int exponent = 0, esign = 1;
  buffer[2] = '\0'; buffer[3] = '\0'; buffer[4] = '\0'; buffer[5] = '\0'; // In case symbol is < 5 letters
  float divisor = 10.0;

  while(!issp(ch) && ch != ')' && ch != '(' && index < bufmax) {
    buffer[index++] = ch;
    if (base == 10 && ch == '.' && !isexponent) {
      isfloat = true;
      fresult = result;
    } else if (base == 10 && (ch == 'e' || ch == 'E')) {
      if (!isfloat) { isfloat = true; fresult = result; }
      isexponent = true;
      if (valid == 1) valid = 0; else valid = -1;
    } else if (isexponent && ch == '-') {
      esign = -esign;
    } else if (isexponent && ch == '+') {
    } else {
      int digit = digitvalue(ch);
      if (digitvalue(ch)<base && valid != -1) valid = 1; else valid = -1;
      if (isexponent) {
        exponent = exponent * 10 + digit;
      } else if (isfloat) {
        fresult = fresult + digit / divisor;
        divisor = divisor * 10.0;
      } else {
        result = result * base + digit;
      }
    }
    ch = gfun();
  }

  buffer[index] = '\0';
  if (ch == ')' || ch == '(') LastChar = ch;
  if (isfloat && valid == 1) return makefloat(fresult * sign * pow(10, exponent * esign));
  else if (valid == 1) {
    if (base == 10 && result > ((unsigned int)INT_MAX+(1-sign)/2))
      return makefloat((float)result*sign);
    return number(result*sign);
  } else if (base == 0) {
    if (index == 1) return character(buffer[0]);
    PGM_P p = ControlCodes; char c = 0;
    while (c < 33) {
      if (strcasecmp_P(buffer, p) == 0) return character(c);
      p = p + strlen_P(p) + 1; c++;
    }
    if (index == 3) return character((buffer[0]*10+buffer[1])*10+buffer[2]-5328);
    error2(0, PSTR("unknown character"));
  }

  int x = builtin(buffer);
  if (x == NIL) return nil;
  if (x < ENDFUNCTIONS) return newsymbol(x);
  else if (index <= 6 && valid40(buffer)) return newsymbol(pack40(buffer));
  else return newsymbol(longsymbol(buffer));
}

object *readrest (gfun_t gfun) {
  object *item = nextitem(gfun);
  object *head = NULL;
  object *tail = NULL;

  while (item != (object *)KET) {
    if (item == (object *)BRA) {
      item = readrest(gfun);
    } else if (item == (object *)QUO) {
      item = cons(symbol(QUOTE), cons(read(gfun), NULL));
    } else if (item == (object *)DOT) {
      tail->cdr = read(gfun);
      if (readrest(gfun) != NULL) error2(0, PSTR("malformed list"));
      return head;
    } else {
      object *cell = cons(item, NULL);
      if (head == NULL) head = cell;
      else tail->cdr = cell;
      tail = cell;
      item = nextitem(gfun);
    }
  }
  return head;
}

object *read (gfun_t gfun) {
  object *item = nextitem(gfun);
  if (item == (object *)KET) error2(0, PSTR("incomplete list"));
  if (item == (object *)BRA) return readrest(gfun);
  if (item == (object *)DOT) return read(gfun);
  if (item == (object *)QUO) return cons(symbol(QUOTE), cons(read(gfun), NULL));
  return item;
}

// Setup

void initgfx () {
#if defined(gfxsupport)
  Wire.begin();
  tft.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  tft.fillScreen(COLOR_BLACK);
  tft.display();
#endif
}

void initenv () {
  GlobalEnv = NULL;
  tee = symbol(TEE);
}

void setup () {
  Serial.begin(9600);
  int start = millis();
  while ((millis() - start) < 5000) { if (Serial) break; }
  initworkspace();
  initenv();
  initsleep();
  initgfx();
  pfstring(PSTR("uLisp 3.6 "), pserial); pln(pserial);
}

// Read/Evaluate/Print loop

void repl (object *env) {
  for (;;) {
    randomSeed(micros());
    gc(NULL, env);
    #if defined (printfreespace)
    pint(Freespace, pserial);
    #endif
    if (BreakLevel) {
      pfstring(PSTR(" : "), pserial);
      pint(BreakLevel, pserial);
    }
    pserial('>'); pserial(' ');
    object *line = read(gserial);
    if (BreakLevel && line == nil) { pln(pserial); return; }
    if (line == (object *)KET) error2(0, PSTR("unmatched right bracket"));
    push(line, GCStack);
    pfl(pserial);
    line = eval(line, env);
    pfl(pserial);
    printobject(line, pserial);
    pop(GCStack);
    pfl(pserial);
    pln(pserial);
  }
}

void loop () {
  if (!setjmp(exception)) {
    #if defined(resetautorun)
    volatile int autorun = 12; // Fudge to keep code size the same
    #else
    volatile int autorun = 13;
    #endif
    if (autorun == 12) autorunimage();
  }
  // Come here after error
  delay(100); while (Serial.available()) Serial.read();
  clrflag(NOESC); BreakLevel = 0;
  for (int i=0; i<TRACEMAX; i++) TraceDepth[i] = 0;
  #if defined(sdcardsupport)
  SDpfile.close(); SDgfile.close();
  #endif
  #if defined(lisplibrary)
  if (!tstflag(LIBRARYLOADED)) { setflag(LIBRARYLOADED); loadfromlibrary(NULL); }
  #endif
  client.stop();
  repl(NULL);
}
