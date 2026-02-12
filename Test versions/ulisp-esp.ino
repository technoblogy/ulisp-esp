/* uLisp ESP Release 4.9test - www.ulisp.com
   David Johnson-Davies - www.technoblogy.com - 12th February 2026

   Licensed under the MIT license: https://opensource.org/licenses/MIT
*/

// Lisp Library
const char LispLibrary[] = "";

// Compile options

// #define resetautorun
#define printfreespace
// #define printgcs
// #define sdcardsupport
// #define gfxsupport
// #define lisplibrary
// #define lineeditor
// #define vt100
// #define extensions
// #define streamextensions

// Includes

// #include "LispLibrary.h"
#include <setjmp.h>
#include <SPI.h>
#include <Wire.h>
#include <limits.h>
#include <WiFi.h>

#define TZ_OFFSET (0 * 3600) // Time zone offset
#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0
#define VT100_HIGHLIGHT "\e[7m"

// Platform specific settings

#if defined(sdcardsupport)
  #include <SD.h>
  #define SDSIZE 172
#else
  #define SDSIZE 0
#endif

#define WORDALIGNED __attribute__((aligned (4)))
#define BUFFERSIZE 36  // Number of bits+4

// ESP32 boards ***************************************************************

#if defined(ARDUINO_ESP32_DEV)                   /* For TTGO T-Display etc. */
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 260000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (9216-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX6
  #if defined(gfxsupport)
    #define TFT_ST7789
    #include <Adafruit_GFX.h>
    #include <Adafruit_ST7789.h>
    #define TFT_MOSI 19
    #define TFT_SCK 18
    Adafruit_ST7789 tft = Adafruit_ST7789(5, 16, 19, 18);
    #define TFT_BACKLITE 4
  #endif

#elif defined(ARDUINO_FEATHER_ESP32)
  #define WORKSPACESIZE (9500-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define CPU_LX6

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 250000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (9500-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define CPU_LX6

#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32_PICO) || defined(ARDUINO_ESP32_PICO)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 250000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (9500-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX6

#elif defined(ARDUINO_TTGO_LoRa32_v21new)
  #define WORKSPACESIZE 9800                     /* Objects (8*bytes) */
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 25
  SPIClass hspi(HSPI);
  #define CPU_LX6
  #if defined(gfxsupport)
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
    #define COLOR_WHITE 1
    #define COLOR_BLACK 0
    #define SCREEN_WIDTH 128 // OLED display width
    #define SCREEN_HEIGHT 64 // OLED display height
    #define OLED_RESET 16
    Adafruit_SSD1306 tft(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
  #endif
  
// ESP32-S2 boards ***************************************************************

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 250000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (6500-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define CPU_LX7
  #if defined(gfxsupport)
    #define TFT_ST7789
    #include <Adafruit_GFX.h>
    #include <Adafruit_ST7789.h>
    #define TFT_MOSI MOSI // Adafruit ESP32 Feathers
    #define TFT_SCK SCK
    Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST);
  #endif

#elif defined(ARDUINO_FEATHERS2)                 /* UM FeatherS2 */
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 1000000                  /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (8160-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX7

#elif defined(ARDUINO_ESP32S2_DEV)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 260000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (8160-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX7

#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 260000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (7232-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX7
  
// ESP32-S3 boards ***************************************************************

#elif defined(ARDUINO_ESP32S3_DEV)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 250000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (24750-SDSIZE)           /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 6500
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define CPU_LX7

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_TFT)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 250000                   /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (22000-SDSIZE)           /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX7
  #if defined(gfxsupport)
    #define TFT_ST7789
    #include <Adafruit_GFX.h>
    #include <Adafruit_ST7789.h>
    #define TFT_MOSI MOSI // Adafruit ESP32 Feathers
    #define TFT_SCK SCK
    Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST);
  #endif

// ESP32-C3 boards ***************************************************************

#elif defined(ARDUINO_ESP32C3_DEV)
  #define WORKSPACESIZE (9216-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 7500
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_RISC_V
  
#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32C3)
  #define WORKSPACESIZE (9216-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_RISC_V

// ESP32-C5 boards ***************************************************************

#elif defined(ARDUINO_ESP32C5_DEV)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 500000                    /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (20000-SDSIZE)            /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define CPU_RISC_V

// ESP32-C6 boards ***************************************************************

#elif defined(ARDUINO_ESP32C6_DEV)
  #define WORKSPACESIZE (25000-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define CPU_RISC_V

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32C6)
  #define WORKSPACESIZE (9216-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h>
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_RISC_V

// ESP32-P4 boards ***************************************************************

#elif defined(ARDUINO_ESP32P4_DEV)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 2000000                  /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (26500-SDSIZE)           /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h> 
  #define SDCARD_SS_PIN 42
  #define LED_BUILTIN 13
  #define CPU_RISC_V

#elif defined(ARDUINO_M5STACK_TAB5)
  #if defined(BOARD_HAS_PSRAM)
  #define WORKSPACESIZE 2000000                  /* Objects (8*bytes) */
  #else
  #define WORKSPACESIZE (26500-SDSIZE)           /* Objects (8*bytes) */
  #endif
  #define MAX_STACK 8000
  #define LITTLEFS
  #include <LittleFS.h> 
  #define SDCARD_SS_PIN 42
  #define LED_BUILTIN 13
  #define CPU_RISC_V
  #if defined(gfxsupport)
    #if !defined(BOARD_HAS_PSRAM)
      #error Tab5 graphics needs PSRAM!
    #endif
    #include <M5Unified.h>
    #include <M5GFX.h>
    M5GFX& tft = M5.Display;
    m5::touch_detail_t touch;
  #endif

// Legacy boards ***************************************************************
  
#elif defined(ESP32)                             /* Generic ESP32 board */
  #define WORKSPACESIZE (9216-SDSIZE)            /* Objects (8*bytes) */
  #define MAX_STACK 7000
  #define LITTLEFS
  #include <LittleFS.h>
  #define analogWrite(x,y) dacWrite((x),(y))
  #define SDCARD_SS_PIN 13
  #define LED_BUILTIN 13
  #define CPU_LX6

#else
#error "Board not supported!"
#endif

// C Macros

#define nil                NULL
#define car(x)             (((object *) (x))->car)
#define cdr(x)             (((object *) (x))->cdr)

#define first(x)           car(x)
#define rest(x)            cdr(x)
#define second(x)          first(rest(x))
#define cddr(x)            cdr(cdr(x))
#define third(x)           first(cddr(x))

#define push(x, y)         ((y) = cons((x),(y)))
#define pop(y)             ((y) = cdr(y))

#define protect(y)         push((y), GCStack)
#define unprotect()        pop(GCStack)

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
#define isbr(x)            (x == ')' || x == '(' || x == '"' || x == '#' || x == '\'')
#define fntype(x)          (getminmax((uint16_t)(x))>>6)
#define longsymbolp(x)     (((x)->name & 0x03) == 0)
#define longnamep(x)       (((x) & 0x03) == 0)
#define twist(x)           ((uint32_t)((x)<<2) | (((x) & 0xC0000000)>>30))
#define untwist(x)         (((x)>>2 & 0x3FFFFFFF) | ((x) & 0x03)<<30)
#define arraysize(x)       (sizeof(x) / sizeof(x[0]))
#define stringifyX(x)      #x
#define stringify(x)       stringifyX(x)
#define PACKEDS            0x43238000
#define BUILTINS           0xF4240000
#define ENDFUNCTIONS       0x0BDC0000

// Constants

#define USERSTREAMS        16
#define TRACEMAX 3  // Maximum number of traced functions
enum type { ZZERO=0, SYMBOL=2, CODE=4, NUMBER=6, STREAM=8, CHARACTER=10, FLOAT=12, ARRAY=14, STRING=16, PAIR=18 };  // ARRAY STRING and PAIR must be last
enum token { UNUSED, BRA, KET, QUO, DOT };
enum fntypes_t { OTHER_FORMS, TAIL_FORMS, FUNCTIONS, SPECIAL_FORMS };

// Typedefs

typedef uint32_t symbol_t;
typedef uint32_t builtin_t;
typedef uint32_t chars_t;

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
        chars_t chars; // For strings
        float single_float;
      };
    };
  };
} object;

typedef object *(*fn_ptr_type)(object *, object *);
typedef void (*mapfun_t)(object *, object **);

typedef const struct {
  const char *string;
  fn_ptr_type fptr;
  uint8_t minmax;
  const char *doc;
} tbl_entry_t;

// Stream typedefs

typedef uint8_t nstream_t;

typedef int (*gfun_t)();
typedef void (*pfun_t)(char);

typedef pfun_t (*pstream_ptr_t)(uint8_t address);
typedef gfun_t (*gstream_ptr_t)(uint8_t address);

typedef const struct {
  const char *streamname;
  pstream_ptr_t pfunptr;
  gstream_ptr_t gfunptr;
} stream_entry_t;

enum builtins: builtin_t { NIL, TEE, NOTHING, OPTIONAL, FEATURES, INITIALELEMENT, ELEMENTTYPE, TEST, COLONA, COLONB,
COLONC, BIT, AMPREST, LAMBDA, LET, LETSTAR, CLOSURE, PSTAR, QUOTE, DEFUN, DEFVAR, BIND, EQ, CAR, FIRST,
CDR, REST, NTH, AREF, CHAR, STRINGFN, PINMODE, DIGITALWRITE, ANALOGREAD, REGISTER, FORMAT, 
 };

// Global variables

#if defined(BOARD_HAS_PSRAM)
object *Workspace WORDALIGNED;
#else
object Workspace[WORKSPACESIZE] WORDALIGNED;
#endif

jmp_buf toplevel_handler;
jmp_buf *handler = &toplevel_handler;
unsigned int Freespace = 0;
object *Freelist;
unsigned int I2Ccount;
unsigned int TraceFn[TRACEMAX];
unsigned int TraceDepth[TRACEMAX];
builtin_t Context;
#define BACKTRACESIZE 8
uint8_t TraceStart = 0, TraceTop = 0;
symbol_t Backtrace[BACKTRACESIZE];

object *GlobalEnv;
object *GCStack = NULL;
object *GlobalString = NULL;
object *GlobalStringTail = NULL;
object *GlobalStringStreamTail = NULL;
int GlobalStringIndex = 0;
uint8_t PrintCount = 0;
uint8_t BreakLevel = 0;
char LastChar = 0;
char LastPrint = 0;
void* StackBottom;

// Flags
enum flag { PRINTREADABLY, RETURNFLAG, ESCAPE, EXITEDITOR, LIBRARYLOADED, NOESC, NOECHO, MUFFLEERRORS, BACKTRACE };
typedef uint16_t flags_t;
flags_t Flags = 1<<PRINTREADABLY; // Set by default

// Forward references
object *tee;
void pfstring (const char *s, pfun_t pfun);

// Error handling

int modbacktrace (int n) {
  return (n+BACKTRACESIZE) % BACKTRACESIZE;
}

void printbacktrace () {
  if (TraceStart != TraceTop) pserial('[');
  int tracesize = modbacktrace(TraceTop-TraceStart);
  for (int i=1; i<=tracesize; i++) {
    printsymbol(symbol(Backtrace[modbacktrace(TraceTop-i)]), pserial);
    if (i!=tracesize) pfstring(" <- ", pserial);
  }
  if (TraceStart != TraceTop) pserial(']');
}

void errorsub (symbol_t fname, const char *string) {
  pfl(pserial); pfstring("Error", pserial);
  if (TraceStart != TraceTop) pserial(' ');
  printbacktrace();
  pfstring(": ", pserial);
  if (fname != sym(NIL)) {
    pserial('\'');
    psymbol(fname, pserial);
    pserial('\''); pserial(' ');
  }
  pfstring(string, pserial);
}

void errorend () { GCStack = NULL; longjmp(*handler, 1); }

void errorsym (symbol_t fname, const char *string, object *symbol) {
  if (!tstflag(MUFFLEERRORS)) {
    errorsub(fname, string);
    pserial(':'); pserial(' ');
    printobject(symbol, pserial);
    pln(pserial);
  }
  errorend();
}

void errorsym2 (symbol_t fname, const char *string) {
  if (!tstflag(MUFFLEERRORS)) {
    errorsub(fname, string);
    pln(pserial);
  }
  errorend();
}

void error (const char *string, object *symbol) {
  errorsym(sym(Context), string, symbol);
}

void error2 (const char *string) {
  errorsym2(sym(Context), string);
}

void formaterr (object *formatstr, const char *string, uint8_t p) {
  pln(pserial); indent(4, ' ', pserial); printstring(formatstr, pserial); pln(pserial);
  indent(p+5, ' ', pserial); pserial('^');
  error2(string);
  pln(pserial);
  errorend();
}

// Save space as these are used multiple times
const char notanumber[] = "argument is not a number";
const char notaninteger[] = "argument is not an integer";
const char notastring[] = "argument is not a string";
const char notalist[] = "argument is not a list";
const char notasymbol[] = "argument is not a symbol";
const char notproper[] = "argument is not a proper list";
const char toomanyargs[] = "too many arguments";
const char toofewargs[] = "too few arguments";
const char noargument[] = "missing argument";
const char nostream[] = "missing stream argument";
const char overflow[] = "arithmetic overflow";
const char divisionbyzero[] = "division by zero";
const char indexnegative[] = "index can't be negative";
const char invalidarg[] = "invalid argument";
const char invalidkey[] = "invalid keyword";
const char illegalclause[] = "illegal clause";
const char illegalfn[] = "illegal function";
const char invalidpin[] = "invalid pin";
const char oddargs[] = "odd number of arguments";
const char indexrange[] = "index out of range";
const char canttakecar[] = "can't take car";
const char canttakecdr[] = "can't take cdr";
const char unknownstreamtype[] = "unknown stream type";
const char streamalreadyopen[] = "stream already in use";
const char streamclosed[] = "invalid stream";

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
  if (Freespace == 0) { Context = NIL; error2("no room"); }
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

inline object *bsymbol (builtin_t name) {
  return intern(twist(name+BUILTINS));
}

object *intern (symbol_t name) {
  #if !defined(BOARD_HAS_PSRAM)
  for (int i=0; i<WORKSPACESIZE; i++) {
    object *obj = &Workspace[i];
    if (obj->type == SYMBOL && obj->name == name) return obj;
  }
  #endif
  return symbol(name);
}

bool eqsymbols (object *obj, char *buffer) {
  object *arg = cdr(obj);
  int i = 0;
  while (!(arg == NULL && buffer[i] == 0)) {
    if (arg == NULL || buffer[i] == 0) return false;
    chars_t test = 0; int shift = 24;
    for (int j=0; j<4; j++, i++) {
      if (buffer[i] == 0) break;
      test = test | buffer[i]<<shift;
      shift = shift - 8;
    }
    if (arg->chars != test) return false;
    arg = car(arg);
  }
  return true;
}

object *internlong (char *buffer) {
  #if !defined(BOARD_HAS_PSRAM)
  for (int i=0; i<WORKSPACESIZE; i++) {
    object *obj = &Workspace[i];
    if (obj->type == SYMBOL && longsymbolp(obj) && eqsymbols(obj, buffer)) return obj;
  }
  #endif
  object *obj = lispstring(buffer);
  obj->type = SYMBOL;
  return obj;
}

object *stream (uint8_t streamtype, uint8_t address) {
  object *ptr = myalloc();
  ptr->type = STREAM;
  ptr->integer = streamtype<<8 | address;
  return ptr;
}

object *newstring () {
  object *ptr = myalloc();
  ptr->type = STRING;
  ptr->chars = 0;
  return ptr;
}

// Features

const char floatingpoint[] = ":floating-point";
const char arrays[] = ":arrays";
const char doc[] = ":documentation";
const char errorhandling[] = ":error-handling";
const char wifi[] = ":wi-fi";
const char gfx[] = ":gfx";
const char sdcard[] = ":sd-card";
const char lx6[] = ":lx6";
const char lx7[] = ":lx7";
const char riscv[] = ":risc-v";

object *features () {
  object *result = NULL;
  #if defined(CPU_RISC_V)
  push(internlong((char *)riscv), result);
  #elif defined(CPU_LX6)
  push(internlong((char *)lx6), result);
  #elif defined(CPU_LX7)
  push(internlong((char *)lx7), result);
  #endif
  #if defined(gfxsupport)
  push(internlong((char *)gfx), result);
  #endif
  #if defined(sdcardsupport)
  push(internlong((char *)sdcard), result);
  #endif
  push(internlong((char *)wifi), result);
  push(internlong((char *)errorhandling), result);
  push(internlong((char *)doc), result);
  push(internlong((char *)arrays), result);
  push(internlong((char *)floatingpoint), result);
  return result;
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

  if ((type == STRING) || (type == SYMBOL && longsymbolp(obj))) {
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
  uintptr_t limit = ((uintptr_t)(from) - (uintptr_t)(Workspace))/sizeof(object);
  for (uintptr_t i=0; i<=limit; i++) {
    object *obj = &Workspace[i];
    unsigned int type = (obj->type) & ~MARKBIT;
    if (marked(obj) && (type >= ARRAY || type==ZZERO || (type == SYMBOL && longsymbolp(obj)))) {
      if (car(obj) == (object *)((uintptr_t)from | MARKBIT))
        car(obj) = (object *)((uintptr_t)to | MARKBIT);
      if (cdr(obj) == from) cdr(obj) = to;
    }
  }
  // Fix strings and long symbols
  for (uintptr_t i=0; i<=limit; i++) {
    object *obj = &Workspace[i];
    if (marked(obj)) {
      unsigned int type = (obj->type) & ~MARKBIT;
      if (type == STRING || (type == SYMBOL && longsymbolp(obj))) {
        obj = cdr(obj);
        while (obj != NULL) {
          if (cdr(obj) == to) cdr(obj) = from;
          obj = (object *)((uintptr_t)(car(obj)) & ~MARKBIT);
        }
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

char *MakeFilename (object *arg, char *buffer) {
  int max = BUFFERSIZE-1;
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

void SDBegin () {
  #if defined(ARDUINO_TTGO_LoRa32_v21new)
  hspi.begin(14, 2, 15, SDCARD_SS_PIN);
  SD.begin(SDCARD_SS_PIN, hspi);
  #else
  SD.begin();
  #endif
}

void SDWriteInt (File file, int data) {
  file.write(data & 0xFF); file.write(data>>8 & 0xFF);
  file.write(data>>16 & 0xFF); file.write(data>>24 & 0xFF);
}

int SDReadInt (File file) {
  uintptr_t b0 = file.read(); uintptr_t b1 = file.read();
  uintptr_t b2 = file.read(); uintptr_t b3 = file.read();
  return b0 | b1<<8 | b2<<16 | b3<<24;
}
#elif defined(LITTLEFS)
void FSWrite32 (File file, uint32_t data) {
  union { uint32_t data2; uint8_t u8[4]; };
  data2 = data;
  if (file.write(u8, 4) != 4) error2("not enough room");
}

uint32_t FSRead32 (File file) {
  union { uint32_t data; uint8_t u8[4]; };
  file.read(u8, 4);
  return data;
}
#else
void EpromWriteInt(int *addr, uintptr_t data) {
  EEPROM.write((*addr)++, data & 0xFF); EEPROM.write((*addr)++, data>>8 & 0xFF);
  EEPROM.write((*addr)++, data>>16 & 0xFF); EEPROM.write((*addr)++, data>>24 & 0xFF);
}

int EpromReadInt (int *addr) {
  uint8_t b0 = EEPROM.read((*addr)++); uint8_t b1 = EEPROM.read((*addr)++);
  uint8_t b2 = EEPROM.read((*addr)++); uint8_t b3 = EEPROM.read((*addr)++);
  return b0 | b1<<8 | b2<<16 | b3<<24;
}
#endif

unsigned int saveimage (object *arg) {
#if defined(sdcardsupport)
  unsigned int imagesize = compactimage(&arg);
  SDBegin();
  File file;
  if (stringp(arg)) {
    char buffer[BUFFERSIZE];
    file = SD.open(MakeFilename(arg, buffer), FILE_WRITE);
    if (!file) error2("problem saving to SD card or invalid filename");
    arg = NULL;
  } else if (arg == NULL || listp(arg)) {
    file = SD.open("/ULISP.IMG", FILE_WRITE);
    if (!file) error2("problem saving to SD card");
  } else error(invalidarg, arg);
  SDWriteInt(file, (uintptr_t)arg);
  SDWriteInt(file, imagesize);
  SDWriteInt(file, (uintptr_t)GlobalEnv);
  SDWriteInt(file, (uintptr_t)GCStack);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    SDWriteInt(file, (uintptr_t)car(obj));
    SDWriteInt(file, (uintptr_t)cdr(obj));
  }
  file.close();
  return imagesize;
#elif defined(LITTLEFS)
  unsigned int imagesize = compactimage(&arg);
  if (!LittleFS.begin(true)) error2("problem mounting LittleFS");
  int bytesneeded = imagesize*8 + 36; int bytesavailable = LittleFS.totalBytes();
  if (bytesneeded > bytesavailable) error("image too large by", number(bytesneeded - bytesavailable));
  File file;
  if (stringp(arg)) {
    char buffer[BUFFERSIZE];
    file = LittleFS.open(MakeFilename(arg, buffer), "w");
    if (!file) error2("problem saving to LittleFS or invalid filename");
    arg = NULL;
  } else if (arg == NULL || listp(arg)) {
    file = LittleFS.open("/ULISP.IMG", "w");
    if (!file) error2("problem saving to LittleFS");
  } else error(invalidarg, arg);
  FSWrite32(file, (uintptr_t)arg);
  FSWrite32(file, imagesize);
  FSWrite32(file, (uintptr_t)GlobalEnv);
  FSWrite32(file, (uintptr_t)GCStack);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    FSWrite32(file, (uintptr_t)car(obj));
    FSWrite32(file, (uintptr_t)cdr(obj));
  }
  file.close();
  return imagesize;
#elif defined(EEPROMSIZE)
  unsigned int imagesize = compactimage(&arg);
  if (!(arg == NULL || listp(arg))) error("illegal argument", arg);
  int bytesneeded = imagesize*8 + 36;
  if (bytesneeded > EEPROMSIZE) error("image too large by", number(bytesneeded - EEPROMSIZE));
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  EpromWriteInt(&addr, (uintptr_t)arg);
  EpromWriteInt(&addr, imagesize);
  EpromWriteInt(&addr, (uintptr_t)GlobalEnv);
  EpromWriteInt(&addr, (uintptr_t)GCStack);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    EpromWriteInt(&addr, (uintptr_t)car(obj));
    EpromWriteInt(&addr, (uintptr_t)cdr(obj));
  }
  EEPROM.commit();
  return imagesize;
#else
  (void) arg;
  error2("not available");
  return 0;
#endif
}

unsigned int loadimage (object *arg) {
#if defined(sdcardsupport)
  SDBegin();
  File file;
  if (stringp(arg)) {
    char buffer[BUFFERSIZE];
    file = SD.open(MakeFilename(arg, buffer));
    if (!file) error2("problem loading from SD card or invalid filename");
  } else if (arg == NULL) {
    file = SD.open("/ULISP.IMG");
    if (!file) error2("problem loading from SD card");
  } else error(invalidarg, arg);
  SDReadInt(file);
  unsigned int imagesize = SDReadInt(file);
  GlobalEnv = (object *)SDReadInt(file);
  GCStack = (object *)SDReadInt(file);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    car(obj) = (object *)SDReadInt(file);
    cdr(obj) = (object *)SDReadInt(file);
  }
  file.close();
  gc(NULL, NULL);
  return imagesize;
#elif defined(LITTLEFS)
  if (!LittleFS.begin()) error2("problem mounting LittleFS");
  File file;
  if (stringp(arg)) {
    char buffer[BUFFERSIZE];
    file = LittleFS.open(MakeFilename(arg, buffer), "r");
    if (!file) error2("problem loading from LittleFS or invalid filename");
  }
  else if (arg == NULL) {
    file = LittleFS.open("/ULISP.IMG", "r");
    if (!file) error2("problem loading from LittleFS");
  }
  else error(invalidarg, arg);
  FSRead32(file);
  unsigned int imagesize = FSRead32(file);
  GlobalEnv = (object *)FSRead32(file);
  GCStack = (object *)FSRead32(file);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    car(obj) = (object *)FSRead32(file);
    cdr(obj) = (object *)FSRead32(file);
  }
  file.close();
  gc(NULL, NULL);
  return imagesize;
#elif defined(EEPROMSIZE)
  (void) arg;
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  EpromReadInt(&addr); // Skip eval address
  unsigned int imagesize = EpromReadInt(&addr);
  if (imagesize == 0 || imagesize == 0xFFFFFFFF) error2("no saved image");
  GlobalEnv = (object *)EpromReadInt(&addr);
  GCStack = (object *)EpromReadInt(&addr);
  for (unsigned int i=0; i<imagesize; i++) {
    object *obj = &Workspace[i];
    car(obj) = (object *)EpromReadInt(&addr);
    cdr(obj) = (object *)EpromReadInt(&addr);
  }
  gc(NULL, NULL);
  return imagesize;
#else
  (void) arg;
  error2("not available");
  return 0;
#endif
}

void autorunimage () {
#if defined(sdcardsupport)
  SDBegin();
  File file = SD.open("/ULISP.IMG");
  if (!file) error2("problem autorunning from SD card");
  object *autorun = (object *)SDReadInt(file);
  file.close();
  if (autorun != NULL) {
    loadimage(NULL);
    apply(autorun, NULL, NULL);
  }
#elif defined(LITTLEFS)
  if (!LittleFS.begin()) error2("problem mounting LittleFS");
  File file = LittleFS.open("/ULISP.IMG", "r");
  if (!file) error2("problem autorunning from LittleFS");
  object *autorun = (object *)FSRead32(file);
  file.close();
  if (autorun != NULL) {
    loadimage(NULL);
    apply(autorun, NULL, NULL);
  }
#elif defined(EEPROMSIZE)
  EEPROM.begin(EEPROMSIZE);
  int addr = 0;
  object *autorun = (object *)EpromReadInt(&addr);
  if (autorun != NULL && (unsigned int)autorun != 0xFFFF) {
    loadimage(NULL);
    apply(autorun, NULL, NULL);
  }
#else
  error2("autorun not available");
#endif
}

// Tracing

int tracing (symbol_t name) {
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == name) return i+1;
    i++;
  }
  return 0;
}

void trace (symbol_t name) {
  if (tracing(name)) error("already being traced", symbol(name));
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == 0) { TraceFn[i] = name; TraceDepth[i] = 0; return; }
    i++;
  }
  error2("already tracing " stringify(TRACEMAX) " functions");
}

void untrace (symbol_t name) {
  int i = 0;
  while (i < TRACEMAX) {
    if (TraceFn[i] == name) { TraceFn[i] = 0; return; }
    i++;
  }
  error("not tracing", symbol(name));
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

object *quote (object *arg) {
  return cons(bsymbol(QUOTE), cons(arg,NULL));
}

// Radix 40 encoding

builtin_t builtin (symbol_t name) {
  return (builtin_t)(untwist(name) - BUILTINS);
}

symbol_t sym (builtin_t x) {
  return twist(x + BUILTINS);
}

int8_t toradix40 (char ch) {
  if (ch == 0) return 0;
  if (ch >= '0' && ch <= '9') return ch-'0'+1;
  if (ch == '-') return 37; if (ch == '*') return 38; if (ch == '$') return 39;
  ch = ch | 0x20;
  if (ch >= 'a' && ch <= 'z') return ch-'a'+11;
  return -1; // Invalid
}

char fromradix40 (char n) {
  if (n >= 1 && n <= 10) return '0'+n-1;
  if (n >= 11 && n <= 36) return 'a'+n-11;
  if (n == 37) return '-'; if (n == 38) return '*'; if (n == 39) return '$';
  return 0;
}

uint32_t pack40 (char *buffer) {
  int x = 0, j = 0;
  for (int i=0; i<6; i++) {
    x = x * 40 + toradix40(buffer[j]);
    if (buffer[j] != 0) j++;
  }
  return x;
}

bool valid40 (char *buffer) {
  int t = 11;
  for (int i=0; i<6; i++) {
    if (toradix40(buffer[i]) < t) return false;
    if (buffer[i] == 0) break;
    t = 0;
  }
  return true;
}

int8_t digitvalue (char d) {
  if (d>='0' && d<='9') return d-'0';
  d = d | 0x20;
  if (d>='a' && d<='f') return d-'a'+10;
  return 16;
}

int checkinteger (object *obj) {
  if (!integerp(obj)) error(notaninteger, obj);
  return obj->integer;
}

int checkbitvalue (object *obj) {
  if (!integerp(obj)) error(notaninteger, obj);
  int n = obj->integer;
  if (n & ~1) error("argument is not a bit value", obj);
  return n;
}

float checkintfloat (object *obj) {
  if (integerp(obj)) return (float)obj->integer;
  if (!floatp(obj)) error(notanumber, obj);
  return obj->single_float;
}

int checkchar (object *obj) {
  if (!characterp(obj)) error("argument is not a character", obj);
  return obj->chars;
}

object *checkstring (object *obj) {
  if (!stringp(obj)) error(notastring, obj);
  return obj;
}

int isstream (object *obj){
  if (!streamp(obj)) error("not a stream", obj);
  return obj->integer;
}

int isbuiltin (object *obj, builtin_t n) {
  return symbolp(obj) && obj->name == sym(n);
}

bool builtinp (symbol_t name) {
  return (untwist(name) >= BUILTINS);
}

int checkkeyword (object *obj) {
  if (!keywordp(obj)) error("argument is not a keyword", obj);
  builtin_t kname = builtin(obj->name);
  uint8_t context = getminmax(kname);
  if (context != 0 && context != Context) error(invalidkey, obj);
  return ((int)lookupfn(kname));
}

void checkargs (object *args) {
  int nargs = listlength(args);
  checkminmax(Context, nargs);
}

bool eqlongsymbol (symbol_t sym1, symbol_t sym2) {
  object *arg1 = (object *)sym1; object *arg2 = (object *)sym2;
  while ((arg1 != NULL) || (arg2 != NULL)) {
    if (arg1 == NULL || arg2 == NULL) return false;
    if (arg1->chars != arg2->chars) return false;
    arg1 = car(arg1); arg2 = car(arg2);
  }
  return true;
}

bool eqsymbol (symbol_t sym1, symbol_t sym2) {
  if (!longnamep(sym1) && !longnamep(sym2)) return (sym1 == sym2);  // Same short symbol
  if (longnamep(sym1) && longnamep(sym2)) return eqlongsymbol(sym1, sym2);  // Same long symbol
  return false;
}

bool eq (object *arg1, object *arg2) {
  if (arg1 == arg2) return true;  // Same object
  if ((arg1 == nil) || (arg2 == nil)) return false;  // Not both values
  #if !defined(BOARD_HAS_PSRAM)
  if (arg1->cdr != arg2->cdr) return false;  // Different values
  if (symbolp(arg1) && symbolp(arg2)) return true;  // Same symbol
  #else
  if (symbolp(arg1) && symbolp(arg2)) return eqsymbol(arg1->name, arg2->name);  // Same symbol?
  if (arg1->cdr != arg2->cdr) return false;  // Different values
  #endif
  if (integerp(arg1) && integerp(arg2)) return true;  // Same integer
  if (floatp(arg1) && floatp(arg2)) return true; // Same float
  if (characterp(arg1) && characterp(arg2)) return true;  // Same character
  return false;
}

bool equal (object *arg1, object *arg2) {
  if (stringp(arg1) && stringp(arg2)) return (stringcompare(cons(arg1, cons(arg2, nil)), false, false, true) != -1);
  if (consp(arg1) && consp(arg2)) return (equal(car(arg1), car(arg2)) && equal(cdr(arg1), cdr(arg2)));
  return eq(arg1, arg2);
}

int listlength (object *list) {
  int length = 0;
  while (list != NULL) {
    if (atom(list)) error2(notproper);
    list = cdr(list);
    length++;
  }
  return length;
}

object *checkarguments (object *args, int min, int max) {
  if (args == NULL) error2(noargument);
  args = first(args);
  if (!listp(args)) error(notalist, args);
  int length = listlength(args);
  if (length < min) error(toofewargs, args);
  if (length > max) error(toomanyargs, args);
  return args;
}

// Mathematical helper functions

object *add_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    fresult = fresult + checkintfloat(arg);
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *subtract_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    fresult = fresult - checkintfloat(arg);
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
  else error(notanumber, arg);
  return nil;
}

object *multiply_floats (object *args, float fresult) {
  while (args != NULL) {
   object *arg = car(args);
    fresult = fresult * checkintfloat(arg);
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *divide_floats (object *args, float fresult) {
  while (args != NULL) {
    object *arg = car(args);
    float f = checkintfloat(arg);
    if (f == 0.0) error2(divisionbyzero);
    fresult = fresult / f;
    args = cdr(args);
  }
  return makefloat(fresult);
}

object *remmod (object *args, bool mod) {
  object *arg1 = first(args);
  object *arg2 = second(args);
  if (integerp(arg1) && integerp(arg2)) {
    int divisor = arg2->integer;
    if (divisor == 0) error2(divisionbyzero);
    int dividend = arg1->integer;
    int remainder = dividend % divisor;
    if (mod && (dividend<0) != (divisor<0)) remainder = remainder + divisor;
    return number(remainder);
  } else {
    float fdivisor = checkintfloat(arg2);
    if (fdivisor == 0.0) error2(divisionbyzero);
    float fdividend = checkintfloat(arg1);
    float fremainder = fmod(fdividend , fdivisor);
    if (mod && (fdividend<0) != (fdivisor<0)) fremainder = fremainder + fdivisor;
    return makefloat(fremainder);
  }
}

object *compare (object *args, bool lt, bool gt, bool eq) {
  object *arg1 = first(args);
  args = cdr(args);
  while (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg1) && integerp(arg2)) {
      if (!lt && ((arg1->integer) < (arg2->integer))) return nil;
      if (!eq && ((arg1->integer) == (arg2->integer))) return nil;
      if (!gt && ((arg1->integer) > (arg2->integer))) return nil;
    } else {
      if (!lt && (checkintfloat(arg1) < checkintfloat(arg2))) return nil;
      if (!eq && (checkintfloat(arg1) == checkintfloat(arg2))) return nil;
      if (!gt && (checkintfloat(arg1) > checkintfloat(arg2))) return nil;
    }
    arg1 = arg2;
    args = cdr(args);
  }
  return tee;
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

// Association lists

object *testargument (object *args) {
  object *test = bsymbol(EQ);
  if (args != NULL) {
    if (cdr(args) == NULL) error2("unpaired keyword");
    if ((isbuiltin(first(args), TEST))) test = second(args);
    else error("unsupported keyword", first(args));
  }
  return test;
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

object *makearray (object *dims, object *def, bool bitp) {
  int size = 1;
  object *dimensions = dims;
  while (dims != NULL) {
    int d = car(dims)->integer;
    if (d < 0) error2("dimension can't be negative");
    size = size * d;
    dims = cdr(dims);
  }
  // Bit array identified by making first dimension negative
  if (bitp) {
    size = (size + sizeof(int)*8 - 1)/(sizeof(int)*8);
    car(dimensions) = number(-(car(dimensions)->integer));
  }
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

object **getarray (object *array, object *subs, object *env, int *bit) {
  int index = 0, size = 1, s;
  *bit = -1;
  bool bitp = false;
  object *dims = cddr(array);
  while (dims != NULL && subs != NULL) {
    int d = car(dims)->integer;
    if (d < 0) { d = -d; bitp = true; }
    if (env) s = checkinteger(eval(car(subs), env)); else s = checkinteger(car(subs));
    if (s < 0 || s >= d) error("subscript out of range", car(subs));
    size = size * d;
    index = index * d + s;
    dims = cdr(dims); subs = cdr(subs);
  }
  if (dims != NULL) error2("too few subscripts");
  if (subs != NULL) error2("too many subscripts");
  if (bitp) {
    size = (size + sizeof(int)*8 - 1)/(sizeof(int)*8);
    *bit = index & (sizeof(int)==4 ? 0x1F : 0x0F);
    index = index>>(sizeof(int)==4 ? 5 : 4);
  }
  return arrayref(array, index, size);
}

void rslice (object *array, int size, int slice, object *dims, object *args) {
  int d = first(dims)->integer;
  for (int i = 0; i < d; i++) {
    int index = slice * d + i;
    if (!consp(args)) error2("initial contents don't match array type");
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
    if (!listp(list)) error2("initial contents don't match array type");
    int l = listlength(list);
    if (dims == NULL) { dims = cons(number(l), NULL); head = dims; }
    else { cdr(dims) = cons(number(l), NULL); dims = cdr(dims); }
    size = size * l;
    if (list != NULL) list = car(list);
  }
  object *array = makearray(head, NULL, false);
  rslice(array, size, 0, head, args);
  return array;
}

object *readbitarray (gfun_t gfun) {
  char ch = gfun();
  object *head = NULL;
  object *tail = NULL;
  while (!issp(ch) && !isbr(ch)) {
    if (ch != '0' && ch != '1') error2("illegal character in bit array");
    object *cell = cons(number(ch - '0'), NULL);
    if (head == NULL) head = cell;
    else tail->cdr = cell;
    tail = cell;
    ch = gfun();
  }
  LastChar = ch;
  int size = listlength(head);
  object *array = makearray(cons(number(size), NULL), number(0), true);
  size = (size + sizeof(int)*8 - 1)/(sizeof(int)*8);
  int index = 0;
  while (head != NULL) {
    object **loc = arrayref(array, index>>(sizeof(int)==4 ? 5 : 4), size);
    int bit = index & (sizeof(int)==4 ? 0x1F : 0x0F);
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
      if (bitp) pint(((*arrayref(array, index>>(sizeof(int)==4 ? 5 : 4), size))->integer)>>
        (index & (sizeof(int)==4 ? 0x1F : 0x0F)) & 1, pfun);
      else printobject(*arrayref(array, index, size), pfun);
    } else { pfun('('); pslice(array, size, index, cdr(dims), pfun, bitp); pfun(')'); }
    testescape();
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
  if (bitp) size = (size + sizeof(int)*8 - 1)/(sizeof(int)*8);
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

object *startstring () {
  object *string = newstring();
  GlobalString = string;
  GlobalStringTail = string;
  return string;
}

object *princtostring (object *arg) {
  object *obj = startstring();
  prin1object(arg, pstr);
  return obj;
}

void buildstring (char ch, object** tail) {
  object* cell;
  if (cdr(*tail) == NULL) {
    cell = myalloc(); cdr(*tail) = cell;
  } else if (((*tail)->chars & 0xFFFFFF) == 0) {
    (*tail)->chars |= ch<<16; return;
  } else if (((*tail)->chars & 0xFFFF) == 0) {
    (*tail)->chars |= ch<<8; return;
  } else if (((*tail)->chars & 0xFF) == 0) {
    (*tail)->chars |= ch; return;
  } else {
    cell = myalloc(); car(*tail) = cell;
  }
  car(cell) = NULL; cell->chars = ch<<24; *tail = cell;
}

object *copystring (object *arg) {
  object *obj = newstring();
  object *ptr = obj;
  arg = cdr(arg);
  while (arg != NULL) {
    object *cell =  myalloc(); car(cell) = NULL;
    if (cdr(obj) == NULL) cdr(obj) = cell; else car(ptr) = cell;
    ptr = cell;
    ptr->chars = arg->chars;
    arg = car(arg);
  }
  return obj;
}

object *readstring (uint8_t delim, bool esc, gfun_t gfun) {
  object *obj = newstring();
  object *tail = obj;
  int ch = gfun();
  if (ch == -1) return nil;
  while ((ch != delim) && (ch != -1)) {
    if (esc && ch == '\\') ch = gfun();
    buildstring(ch, &tail);
    ch = gfun();
  }
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

object **getcharplace (object *string, int n, int *shift) {
  object **arg = &cdr(string);
  int top;
  if (sizeof(int) == 4) { top = n>>2; *shift = 3 - (n&3); }
  else { top = n>>1; *shift = 1 - (n&1); }
  *shift = - (*shift + 2);
  for (int i=0; i<top; i++) {
    if (*arg == NULL) break;
    arg = &car(*arg);
  }
  return arg;
}

uint8_t nthchar (object *string, int n) {
  int shift;
  object **arg = getcharplace(string, n, &shift);
  if (*arg == NULL) return 0;
  return (((*arg)->chars)>>((-shift-2)<<3)) & 0xFF;
}

int gstr () {
  char c = nthchar(GlobalString, GlobalStringIndex++);
  if (c != 0) return c;
  return '\n'; // -1?
}

void pstr (char c) {
  buildstring(c, &GlobalStringTail);
}

void pstrstream (char c) {
  buildstring(c, &GlobalStringStreamTail);
}

object *lispstring (char *s) {
  object *obj = newstring();
  object *tail = obj;
  while(1) {
    char ch = *s++;
    if (ch == 0) break;
    if (ch == '\\') ch = *s++;
    buildstring(ch, &tail);
  }
  return obj;
}

int stringcompare (object *args, bool lt, bool gt, bool eq) {
  object *arg1 = checkstring(first(args));
  object *arg2 = checkstring(second(args));
  arg1 = cdr(arg1); arg2 = cdr(arg2);
  int m = 0; chars_t a = 0, b = 0;
  while ((arg1 != NULL) || (arg2 != NULL)) {
    if (arg1 == NULL) return lt ? m : -1;
    if (arg2 == NULL) return gt ? m : -1;
    a = arg1->chars; b = arg2->chars;
    if (a < b) { if (lt) { m = m + sizeof(int); while (a != b) { m--; a = a >> 8; b = b >> 8; } return m; } else return -1; }
    if (a > b) { if (gt) { m = m + sizeof(int); while (a != b) { m--; a = a >> 8; b = b >> 8; } return m; } else return -1; }
    arg1 = car(arg1); arg2 = car(arg2);
    m = m + sizeof(int);
  }
  if (eq) { m = m - sizeof(int); while (a != 0) { m++; a = a << 8;} return m;} else return -1;
}

object *documentation (object *arg, object *env) {
  if (arg == NULL) return nil;
  if (!symbolp(arg)) error(notasymbol, arg);
  object *pair = findpair(arg, env);
  if (pair != NULL) {
    object *val = cdr(pair);
    if (consp(val) && first(val)->name == sym(LAMBDA) && cdr(val) != NULL && cddr(val) != NULL) {
      if (stringp(third(val))) return third(val);
    }
  }
  symbol_t docname = arg->name;
  if (!builtinp(docname)) return nil;
  char *docstring = lookupdoc(builtin(docname));
  if (docstring == NULL) return nil;
  object *obj = startstring();
  pfstring(docstring, pstr);
  return obj;
}

object *apropos (object *arg, bool print) {
  char buf[17], buf2[33];
  char *part = cstring(princtostring(arg), buf, 17);
  object *result = cons(NULL, NULL);
  object *ptr = result;
  // User-defined?
  object *globals = GlobalEnv;
  while (globals != NULL) {
    object *pair = first(globals);
    object *var = car(pair);
    object *val = cdr(pair);
    char *full = cstring(princtostring(var), buf2, 33);
    if (strstr(full, part) != NULL) {
      if (print) {
        printsymbol(var, pserial); pserial(' '); pserial('(');
        if (consp(val) && isbuiltin(car(val), LAMBDA)) pfstring("user function", pserial);
        else if (consp(val) && car(val)->type == CODE) pfstring("code", pserial);
        else pfstring("user symbol", pserial);
        pserial(')'); pln(pserial);
      } else {
        cdr(ptr) = cons(var, NULL); ptr = cdr(ptr);
      }
    }
    globals = cdr(globals);
    testescape();
  }
  // Built-in?
  int entries = tablesize(0) + tablesize(1);
  for (int i = 0; i < entries; i++) {
    if (findsubstring(part, (builtin_t)i)) {
      if (print) {
        uint8_t fntype = fntype(i);
        pbuiltin((builtin_t)i, pserial); pserial(' '); pserial('(');
        if (fntype == FUNCTIONS) pfstring("function", pserial);
        else if (fntype == SPECIAL_FORMS || fntype == TAIL_FORMS) pfstring("special form", pserial);
        else pfstring("symbol/keyword", pserial);
        pserial(')'); pln(pserial);
      } else {
        cdr(ptr) = cons(bsymbol(i), NULL); ptr = cdr(ptr);
      }
    }
    testescape();
  }
  return cdr(result);
}

char *cstring (object *form, char *buffer, int buflen) {
  form = cdr(checkstring(form));
  int index = 0;
  while (form != NULL) {
    int chars = form->integer;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      char ch = chars>>i & 0xFF;
      if (ch) {
        if (index >= buflen-1) error2("no room for string");
        buffer[index++] = ch;
      }
    }
    form = car(form);
  }
  buffer[index] = '\0';
  return buffer;
}

object *iptostring (uint32_t ip) {
  union { uint32_t data2; uint8_t u8[4]; };
  object *obj = startstring();
  data2 = ip;
  for (int i=0; i<4; i++) {
    if (i) pstr('.');
    pintbase(u8[i], 10, pstr);
  }
  return obj;
}

uint32_t ipstring (object *form) {
  form = cdr(checkstring(form));
  int p = 0;
  union { uint32_t ipaddress; uint8_t ipbytes[4]; } ;
  ipaddress = 0;
  while (form != NULL) {
    int chars = form->integer;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      char ch = chars>>i & 0xFF;
      if (ch) {
        if (ch == '.') { p++; if (p > 3) error2("illegal IP address"); }
        else ipbytes[p] = (ipbytes[p] * 10) + ch - '0';
      }
    }
    form = car(form);
  }
  return ipaddress;
}

object *value (symbol_t n, object *env) {
  while (env != NULL) {
    object *pair = car(env);
    #if !defined(BOARD_HAS_PSRAM)
    if (pair != NULL && car(pair)->name == n) return pair;
    #else
    if (pair != NULL && eqsymbol(car(pair)->name, n)) return pair;
    #endif
    env = cdr(env);
  }
  return nil;
}

object *findpair (object *var, object *env) {
  symbol_t name = var->name;
  object *pair = value(name, env);
  if (pair == NULL) pair = value(name, GlobalEnv);
  return pair;
}

bool boundp (object *var, object *env) {
  if (!symbolp(var)) error(notasymbol, var);
  return (findpair(var, env) != NULL);
}

object *findvalue (object *var, object *env) {
  object *pair = findpair(var, env);
  if (pair == NULL) error("unknown variable", var);
  return pair;
}

// Handling closures

object *closure (int tc, symbol_t name, object *function, object *args, object **env) {
  object *state = car(function);
  function = cdr(function);
  int trace = tracing(name);
  if (trace) {
    indent(TraceDepth[trace-1]<<1, ' ', pserial);
    pint(TraceDepth[trace-1]++, pserial);
    pserial(':'); pserial(' '); pserial('('); printsymbol(symbol(name), pserial);
  }
  object *params = first(function);
  if (!listp(params)) errorsym(name, notalist, params);
  function = cdr(function);
  // Dropframe
  if (tc) {
    if (*env != NULL && car(*env) == NULL) {
      pop(*env);
      while (*env != NULL && car(*env) != NULL) pop(*env);
    } else push(nil, *env);
  }
  // Push state
  while (consp(state)) {
    object *pair = first(state);
    push(pair, *env);
    state = cdr(state);
  }
  // Add arguments to environment
  bool optional = false;
  object *paramsfull = params;
  while (params != NULL) {
    if (atom(params)) errorsym(name, notproper, paramsfull);
    object *value;
    object *var = first(params);
    if (isbuiltin(var, OPTIONAL)) optional = true;
    else {
      if (consp(var)) {
        if (!optional) errorsym(name, "invalid default value", var);
        if (args == NULL) value = eval(second(var), *env);
        else { value = first(args); args = cdr(args); }
        var = first(var);
        if (!symbolp(var)) errorsym(name, "illegal optional parameter", var);
      } else if (!symbolp(var)) {
        errorsym(name, "illegal function parameter", var);
      } else if (isbuiltin(var, AMPREST)) {
        params = cdr(params);
        var = first(params);
        value = args;
        args = NULL;
      } else {
        if (args == NULL) {
          if (optional) value = nil;
          else errorsym2(name, toofewargs);
        } else { value = first(args); args = cdr(args); }
      }
      push(cons(var,value), *env);
      if (trace) { pserial(' '); printobject(value, pserial); }
    }
    params = cdr(params);
  }
  if (args != NULL) errorsym(name, toomanyargs, args);
  if (trace) { pserial(')'); pln(pserial); }
  // Do an implicit progn
  if (tc) push(nil, *env);
  return tf_progn(function, *env);
}

object *apply (object *function, object *args, object *env) {
  if (symbolp(function)) {
    builtin_t fname = builtin(function->name);
    if ((fname < ENDFUNCTIONS) && (fntype(fname) == FUNCTIONS)) {
      Context = fname;
      checkargs(args);
      return ((fn_ptr_type)lookupfn(fname))(args, env);
    } else function = eval(function, env);
  }
  if (consp(function) && isbuiltin(car(function), LAMBDA)) {
    object *result = closure(0, sym(NIL), function, args, &env);
    return eval(result, env);
  }
  if (consp(function) && isbuiltin(car(function), CLOSURE)) {
    function = cdr(function);
    object *result = closure(0, sym(NIL), function, args, &env);
    return eval(result, env);
  }
  error(illegalfn, function);
  return NULL;
}

// In-place operations

object **place (object *args, object *env, int *bit) {
  *bit = -1;
  if (atom(args)) return &cdr(findvalue(args, env));
  object* function = first(args);
  if (symbolp(function)) {
    symbol_t sname = function->name;
    if (sname == sym(CAR) || sname == sym(FIRST)) {
      object *value = eval(second(args), env);
      if (!listp(value)) error(canttakecar, value);
      return &car(value);
    }
    if (sname == sym(CDR) || sname == sym(REST)) {
      object *value = eval(second(args), env);
      if (!listp(value)) error(canttakecdr, value);
      return &cdr(value);
    }
    if (sname == sym(NTH)) {
      int index = checkinteger(eval(second(args), env));
      object *list = eval(third(args), env);
      if (atom(list)) { Context = NTH; error("second argument is not a list", list); }
      int i = index;
      while (i > 0) {
        list = cdr(list);
        if (list == NULL) { Context = NTH; error(indexrange, number(index)); }
        i--;
      }
      return &car(list);
    }
    if (sname == sym(CHAR)) {
      int index = checkinteger(eval(third(args), env));
      object *string = checkstring(eval(second(args), env));
      object **loc = getcharplace(string, index, bit);
      if ((*loc) == NULL || (((((*loc)->chars)>>((-(*bit)-2)<<3)) & 0xFF) == 0)) { Context = CHAR; error(indexrange, number(index)); }
      return loc;
    }
    if (sname == sym(AREF)) {
      object *array = eval(second(args), env);
      if (!arrayp(array)) { Context = AREF; error("first argument is not an array", array); }
      return getarray(array, cddr(args), env, bit);
    }
  }
  error2("illegal place");
  return nil;
}

// Checked car and cdr

object *carx (object *arg) {
  if (!listp(arg)) error(canttakecar, arg);
  if (arg == nil) return nil;
  return car(arg);
}

object *cdrx (object *arg) {
  if (!listp(arg)) error(canttakecdr, arg);
  if (arg == nil) return nil;
  return cdr(arg);
}

object *cxxxr (object *args, uint8_t pattern) {
  object *arg = first(args);
  while (pattern != 1) {
    if ((pattern & 1) == 0) arg = carx(arg); else arg = cdrx(arg);
    pattern = pattern>>1;
  }
  return arg;
}

// Mapping helper functions

object *mapcl (object *args, object *env, bool mapl) {
  object *function = first(args);
  args = cdr(args);
  object *result = first(args);
  protect(result);
  object *params = cons(NULL, NULL);
  protect(params);
  // Make parameters
  while (true) {
    object *tailp = params;
    object *lists = args;
    while (lists != NULL) {
      object *list = car(lists);
      if (list == NULL) {
         unprotect(); unprotect();
         return result;
      }
      if (atom(list)) error(notproper, list);
      object *item = mapl ? list : first(list);
      object *obj = cons(item, NULL);
      car(lists) = cdr(list);
      cdr(tailp) = obj; tailp = obj;
      lists = cdr(lists);
    }
    apply(function, cdr(params), env);
  }
}

void mapcarfun (object *result, object **tail) {
  object *obj = cons(result,NULL);
  cdr(*tail) = obj; *tail = obj;
}

void mapcanfun (object *result, object **tail) {
  if (cdr(*tail) != NULL) error(notproper, *tail);
  while (consp(result)) {
    cdr(*tail) = result; *tail = result;
    result = cdr(result);
  }
}

object *mapcarcan (object *args, object *env, mapfun_t fun, bool maplist) {
  object *function = first(args);
  args = cdr(args);
  object *params = cons(NULL, NULL);
  protect(params);
  object *head = cons(NULL, NULL);
  protect(head);
  object *tail = head;
  // Make parameters
  while (true) {
    object *tailp = params;
    object *lists = args;
    while (lists != NULL) {
      object *list = car(lists);
      if (list == NULL) {
         unprotect(); unprotect();
         return cdr(head);
      }
      if (atom(list)) error(notproper, list);
      object *item = maplist ? list : first(list);
      object *obj = cons(item, NULL);
      car(lists) = cdr(list);
      cdr(tailp) = obj; tailp = obj;
      lists = cdr(lists);
    }
    object *result = apply(function, cdr(params), env);
    fun(result, &tail);
  }
}

object *dobody (object *args, object *env, bool star) {
  object *varlist = first(args), *endlist = second(args);
  object *head = cons(NULL, NULL);
  protect(head);
  object *ptr = head;
  object *newenv = env;
  protect(newenv);
  while (varlist != NULL) {
    object *varform = first(varlist);
    object *var, *init = NULL, *step = NULL;
    if (atom(varform)) var = varform;
    else {
      var = first(varform);
      varform = cdr(varform);
      if (varform != NULL) {  
        init = eval(first(varform), env);
        varform = cdr(varform);
        if (varform != NULL) step = cons(first(varform), NULL);
      }
    }  
    object *pair = cons(var, init);
    unprotect(); // newenv
    push(pair, newenv);
    protect(newenv);
    if (star) env = newenv;
    object *cell = cons(cons(step, pair), NULL);
    cdr(ptr) = cell; ptr = cdr(ptr);
    varlist = cdr(varlist);
  }
  env = newenv;
  head = cdr(head);
  object *endtest = first(endlist), *results = cdr(endlist);
  while (eval(endtest, env) == NULL) {
    object *forms = cddr(args);
    while (forms != NULL) {
      object *result = eval(car(forms), env);
      if (tstflag(RETURNFLAG)) {
        clrflag(RETURNFLAG);
        unprotect(); // newenv
        unprotect(); // head
        return result;
      }
      forms = cdr(forms);
    }
    object *varlist = head;
    int count = 0;
    while (varlist != NULL) {
      object *varform = first(varlist);
      object *step = car(varform), *pair = cdr(varform);
      if (step != NULL) {
        object *val = eval(first(step), env);
        if (star) {
          cdr(pair) = val;
        } else {
          push(val, GCStack);
          push(pair, GCStack);
          count++;
        }
      } 
      varlist = cdr(varlist);
    }
    while (count > 0) {
      cdr(car(GCStack)) = car(cdr(GCStack));
      pop(GCStack); pop(GCStack);
      count--;
    }
  }
  unprotect(); // newenv
  unprotect(); // head
  return eval(tf_progn(results, env), env);
}

// I2C interface for up to two ports, using Arduino Wire

void I2Cinit (TwoWire *port, bool enablePullup) {
  (void) enablePullup;
  port->begin();
}

int I2Cread (TwoWire *port) {
  return port->read();
}

void I2Cwrite (TwoWire *port, uint8_t data) {
  port->write(data);
}

bool I2Cstart (TwoWire *port, uint8_t address, uint8_t read) {
 int ok = true;
 if (read == 0) {
   port->beginTransmission(address);
   ok = (port->endTransmission(true) == 0);
   port->beginTransmission(address);
 }
 else port->requestFrom(address, I2Ccount);
 return ok;
}

bool I2Crestart (TwoWire *port, uint8_t address, uint8_t read) {
  int error = (port->endTransmission(false) != 0);
  if (read == 0) port->beginTransmission(address);
  else port->requestFrom(address, I2Ccount);
  return error ? false : true;
}

void I2Cstop (TwoWire *port, uint8_t read) {
  if (read == 0) port->endTransmission(); // Check for error?
  // Release pins
  port->end();
}

// Streams

enum stream { SERIALSTREAM, I2CSTREAM, SPISTREAM, SDSTREAM, WIFISTREAM, STRINGSTREAM, GFXSTREAM };

// Simplify board differences
#if defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2) || defined(ARDUINO_ESP32C6_DEV)
#define ULISP_HOWMANYI2C 2
#endif

#define ULISP_WIFI

WiFiClient client;
WiFiServer server(80);

void spiwrite (char c) { SPI.transfer(c); }
void i2cwrite (char c) { I2Cwrite(&Wire, c); }
#if ULISP_HOWMANYI2C == 2
void i2c1write (char c) { I2Cwrite(&Wire1, c); }
#endif
void serial1write (char c) { Serial1.write(c); }
void WiFiwrite (char c) { client.write(c); }
#if defined(sdcardsupport)
File SDpfile, SDgfile;
void SDwrite (char c) { SDpfile.write(c); }
#endif
#if defined(gfxsupport)
void gfxwrite (char c) { tft.write(c); }
#endif

int spiread () { return SPI.transfer(0); }
int i2cread () { return I2Cread(&Wire); }
#if ULISP_HOWMANYI2C == 2
int i2c1read () { return I2Cread(&Wire1); }
#endif
int serial1read () { while (!Serial1.available()) testescape(); return Serial1.read(); }
#if defined(sdcardsupport)
int SDread () { return SDgfile.read(); }
#endif

int WiFiread () {
  while (!client.available()) testescape();
  return client.read();
}

void serialbegin (int address, int baud) {
  if (address == 1) Serial1.begin((long)baud*100);
  else error("port not supported", number(address));
}

void serialend (int address) {
  if (address == 1) {Serial1.flush(); Serial1.end(); }
  else error("port not supported", number(address));
}

// Stream writing functions

pfun_t pfun_i2c (uint8_t address) {
  pfun_t pfun;
  if (address < 128) pfun = i2cwrite;
  #if ULISP_HOWMANYI2C == 2
  else pfun = i2c1write;
  #endif
  return pfun;
}

pfun_t pfun_spi (uint8_t address) {
  (void) address;
  return spiwrite;
}

pfun_t pfun_serial (uint8_t address) {
  pfun_t pfun = pserial;
  if (address == 1) pfun = serial1write;
  return pfun;
}

pfun_t pfun_string (uint8_t address) {
  (void) address;
  if (GlobalStringStreamTail == NULL) error2(streamclosed);
  return pstrstream;
}

pfun_t pfun_sd (uint8_t address) {
  (void) address;
  pfun_t pfun = pserial;
  #if defined(sdcardsupport)
  pfun = (pfun_t)SDwrite;
  #endif
  return pfun;
}

pfun_t pfun_gfx (uint8_t address) {
  (void) address;
  pfun_t pfun = pserial;
  #if defined(gfxsupport)
  pfun = (pfun_t)gfxwrite;
  #endif
  return pfun;
}

pfun_t pfun_wifi (uint8_t address) {
  (void) address; 
  pfun_t pfun = pserial;
  #if defined(ULISP_WIFI)
  pfun = (pfun_t)WiFiwrite;
  #endif
  return pfun;
}

// Stream reading functions

gfun_t gfun_i2c (uint8_t address) {
  gfun_t gfun;
  if (address < 128) gfun = i2cread;
  #if ULISP_HOWMANYI2C == 2
  else gfun = i2c1read;
  #endif
  return gfun;
}

gfun_t gfun_spi (uint8_t address) {
  return spiread;
}

gfun_t gfun_serial (uint8_t address) {
  gfun_t gfun = gserial;
  if (address == 1) gfun = serial1read;
  return gfun;
}

gfun_t gfun_sd (uint8_t address) {
  (void) address;
  gfun_t gfun = gserial;
  #if defined(sdcardsupport)
  gfun = (gfun_t)SDread;
  #endif
  return gfun;
}

gfun_t gfun_wifi (uint8_t address) {
  (void) address; 
  gfun_t gfun = gserial;
  #if defined(ULISP_WIFI)
  gfun = (gfun_t)WiFiread;
  #endif
  return gfun;
}

// Stream names used by printobject
const char serialstreamname[] = "serial";
const char i2cstreamname[] = "i2c";
const char spistreamname[] = "spi";
const char sdstreamname[] = "sd";
const char wifistreamname[] = "wifi";
const char stringstreamname[] = "string";
const char gfxstreamname[] = "gfx";

// Stream lookup table - needs to be in same order as enum stream
const stream_entry_t stream_table[] = {
  { serialstreamname, pfun_serial, gfun_serial },
  { i2cstreamname, pfun_i2c, gfun_i2c },
  { spistreamname, pfun_spi, gfun_spi },
  { sdstreamname, pfun_sd, gfun_sd },
  { wifistreamname, pfun_wifi, gfun_wifi },
  { stringstreamname, pfun_string, NULL },
  { gfxstreamname, pfun_gfx, NULL },
};

#if !defined(streamextensions)
// Stream table cross-reference functions

stream_entry_t *streamtables[] = {stream_table, NULL};

const stream_entry_t *streamtable (int n) {
  return streamtables[n];
}
#endif

pfun_t pstreamfun (object *args) {
  nstream_t nstream = SERIALSTREAM;
  int address = 0;
  pfun_t pfun = pserial;
  if (args != NULL && first(args) != NULL) {
    int stream = isstream(first(args));
    nstream = stream>>8; address = stream & 0xFF;
  }
  bool n = nstream<USERSTREAMS;
  pstream_ptr_t streamfunction = streamtable(n?0:1)[n?nstream:nstream-USERSTREAMS].pfunptr;
  pfun = streamfunction(address);
  return pfun;
}

gfun_t gstreamfun (object *args) {
  nstream_t nstream = SERIALSTREAM;
  int address = 0;
  gfun_t gfun = gserial;
  if (args != NULL) {
    int stream = isstream(first(args));
    nstream = stream>>8; address = stream & 0xFF;
  }
  bool n = nstream<USERSTREAMS;
  gstream_ptr_t streamfunction = streamtable(n?0:1)[n?nstream:nstream-USERSTREAMS].gfunptr;
  gfun = streamfunction(address);
  return gfun;
}

// Check pins

void checkanalogread (int pin) {
#if defined(ESP32) || defined(ARDUINO_ESP32_DEV)
  if (!(pin==0 || pin==2 || pin==4 || (pin>=12 && pin<=15) || (pin>=25 && pin<=27) || (pin>=32 && pin<=36) || pin==39))
    error("invalid pin", number(pin));
#elif defined(ARDUINO_FEATHER_ESP32) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
  if (!(pin==4 || (pin>=12 && pin<=15) || (pin>=25 && pin<=27) || (pin>=32 && pin<=36) || pin==39)) error("invalid pin", number(pin));
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT)
  if (!(pin==8 || (pin>=14 && pin<=18))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32_PICO)
  if (!(pin==4 || pin==7 || (pin>=12 && pin<=15) || (pin>=25 && pin<=27) || (pin>=32 && pin<=33))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2)
  if (!((pin>=5 && pin<=9) || (pin>=16 && pin<=18))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ADAFRUIT_QTPY_ESP32C3)
  if (!((pin>=0 && pin<=1) || (pin>=3 && pin<=5))) error("invalid pin", number(pin));
#elif defined(ARDUINO_FEATHERS2) || defined(ARDUINO_ESP32S2_DEV)
  if (!((pin>=1 && pin<=20))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ESP32C3_DEV)
  if (!((pin>=0 && pin<=5))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ESP32S3_DEV)
  if (!((pin>=1 && pin<=20))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ESP32C6_DEV)
  if (!((pin>=1 && pin<=20))) error("invalid pin", number(pin));
#elif defined(ARDUINO_ESP32P4_DEV) || defined(ARDUINO_M5STACK_TAB5)
  if (!((pin>=16 && pin<=23) || (pin>=49 && pin<=54))) error("invalid pin", number(pin));
#elif defined(ARDUINO_TTGO_LoRa32_v21new)
  if (!(pin==0 || pin==2 || pin==4 || (pin>=12 && pin<=15) || (pin>=25 && pin<=26) || (pin>=34 && pin<=36) || pin==39)) \
  error("invalid pin", number(pin));
#endif
}

void checkanalogwrite (int pin) {
#if defined(ESP32) || defined(ARDUINO_FEATHER_ESP32) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2) || defined(ARDUINO_ESP32_DEV) \
  || defined(ARDUINO_ADAFRUIT_QTPY_ESP32_PICO)
  if (!(pin>=25 && pin<=26)) error("invalid pin", number(pin));
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT) || defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2) \
  || defined(ARDUINO_FEATHERS2) || defined(ARDUINO_ESP32S2_DEV)
  if (!(pin>=17 && pin<=18)) error("invalid pin", number(pin));
#else
  error2(ANALOGWRITE, "not supported");
#endif
}

// Note

void tone (int pin, int note) {
  (void) pin, (void) note;
}

void noTone (int pin) {
  (void) pin;
}

const int scale[] = {4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902};

void playnote (int pin, int note, int octave) {
  int oct = octave + note/12;
  int prescaler = 8 - oct;
  if (prescaler<0 || prescaler>8) error("octave out of range", number(oct));
  tone(pin, scale[note%12]>>prescaler);
}

void nonote (int pin) {
  noTone(pin);
}

// Sleep

void initsleep () { }

void doze (int secs) {
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

uint8_t basewidth (object *obj, uint8_t base) {
  PrintCount = 0;
  pintbase(obj->integer, base, pcount);
  return PrintCount;
}

bool quoted (object *obj) {
  return (consp(obj) && car(obj) != NULL && car(obj)->name == sym(QUOTE) && consp(cdr(obj)) && cddr(obj) == NULL);
}

int subwidth (object *obj, int w) {
  if (atom(obj)) return w - atomwidth(obj);
  if (quoted(obj)) obj = car(cdr(obj));
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
    if (isbuiltin(form, NOTHING)) printsymbol(form, pfun);
    else printobject(form, pfun);
  } else if (quoted(form)) {
    pfun('\'');
    superprint(car(cdr(form)), lm + 1, pfun);
  } else {
    lm = lm + PPINDENT;
    bool fits = (subwidth(form, ppwidth - lm - PPINDENT) >= 0);
    int special = 0, extra = 0; bool separate = true;
    object *arg = car(form);
    if (symbolp(arg) && builtinp(arg->name)) {
      uint8_t minmax = getminmax(builtin(arg->name));
      if (minmax == 0327 || minmax == 0313) special = 2; // defun, setq, setf, defvar
      else if (minmax == 0317 || minmax == 0017 || minmax == 0117 || minmax == 0123) special = 1;
    }
    while (form != NULL) {
      if (atom(form)) { pfstring(" . ", pfun); printobject(form, pfun); pfun(')'); return; }
      else if (separate) { 
        pfun('(');
        separate = false;
      } else if (special) {
        pfun(' ');
        special--; 
      } else if (fits) {
        pfun(' ');
      } else { pln(pfun); indent(lm, ' ', pfun); }
      superprint(car(form), lm+extra, pfun);
      form = cdr(form);
    }
    pfun(')');
    testescape();
  }
}

object *edit (object *fun) {
  while (1) {
    if (tstflag(EXITEDITOR)) return fun;
    char c = gserial();
    if (c == 'q') setflag(EXITEDITOR);
    else if (c == 'b') return fun;
    else if (c == 'r') fun = readmain(gserial);
    else if (c == '\n') { pfl(pserial); superprint(fun, 0, pserial); pln(pserial); }
    else if (c == 'c') fun = cons(readmain(gserial), fun);
    else if (atom(fun)) pserial('!');
    else if (c == 'd') fun = cons(car(fun), edit(cdr(fun)));
    else if (c == 'a') fun = cons(edit(car(fun)), cdr(fun));
    else if (c == 'x') fun = cdr(fun);
    else pserial('?');
  }
}

// Special forms

object *sp_quote (object *args, object *env) {
  (void) env;
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
  object *var = first(args);
  if (!symbolp(var)) error(notasymbol, var);
  object *val = cons(bsymbol(LAMBDA), cdr(args));
  object *pair = value(var->name, GlobalEnv);
  if (pair != NULL) cdr(pair) = val;
  else push(cons(var, val), GlobalEnv);
  return var;
}

object *sp_defvar (object *args, object *env) {
  object *var = first(args);
  if (!symbolp(var)) error(notasymbol, var);
  object *val = NULL;
  args = cdr(args);
  if (args != NULL) { setflag(NOESC); val = eval(first(args), env); clrflag(NOESC); }
  object *pair = value(var->name, GlobalEnv);
  if (pair != NULL) cdr(pair) = val;
  else push(cons(var, val), GlobalEnv);
  return var;
}

object *sp_setq (object *args, object *env) {
  object *arg = nil; builtin_t setq = Context;
  while (args != NULL) {
    if (cdr(args) == NULL) { Context = setq; error2(oddargs); }
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
    testescape();
  }
}

object *sp_push (object *args, object *env) {
  int bit;
  object *item = eval(first(args), env);
  object **loc = place(second(args), env, &bit);
  if (bit != -1) error2(invalidarg);
  push(item, *loc);
  return *loc;
}

object *sp_pop (object *args, object *env) {
  int bit;
  object *arg = first(args);
  if (arg == NULL) error2(invalidarg);
  object **loc = place(arg, env, &bit);
  if (bit < -1) error(invalidarg, arg);
  if (!consp(*loc)) error(notalist, *loc);
  object *result = car(*loc);
  pop(*loc);
  return result;
}

// Accessors

object *sp_incf (object *args, object *env) {
  int bit;
  object **loc = place(first(args), env, &bit);
  if (bit < -1) error2(notanumber);
  args = cdr(args);

  object *x = *loc;
  object *inc = (args != NULL) ? eval(first(args), env) : NULL;

  if (bit != -1) {
    int increment;
    if (inc == NULL) increment = 1; else increment = checkbitvalue(inc);
    int newvalue = (((*loc)->integer)>>bit & 1) + increment;

    if (newvalue & ~1) error2("result is not a bit value");
    *loc = number((((*loc)->integer) & ~(1<<bit)) | newvalue<<bit);
    return number(newvalue);
  }

  if (floatp(x) || floatp(inc)) {
    float increment;
    float value = checkintfloat(x);

    if (inc == NULL) increment = 1.0; else increment = checkintfloat(inc);

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
  } else error2(notanumber);
  return *loc;
}

object *sp_decf (object *args, object *env) {
  int bit;
  object **loc = place(first(args), env, &bit);
  if (bit < -1) error2(notanumber);
  args = cdr(args);

  object *x = *loc;
  object *dec = (args != NULL) ? eval(first(args), env) : NULL;

  if (bit != -1) {
    int decrement;
    if (dec == NULL) decrement = 1; else decrement = checkbitvalue(dec);
    int newvalue = (((*loc)->integer)>>bit & 1) - decrement;

    if (newvalue & ~1) error2("result is not a bit value");
    *loc = number((((*loc)->integer) & ~(1<<bit)) | newvalue<<bit);
    return number(newvalue);
  }

  if (floatp(x) || floatp(dec)) {
    float decrement;
    float value = checkintfloat(x);

    if (dec == NULL) decrement = 1.0; else decrement = checkintfloat(dec);

    *loc = makefloat(value - decrement);
  } else if (integerp(x) && (integerp(dec) || dec == NULL)) {
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
  } else error2(notanumber);
  return *loc;
}

object *sp_setf (object *args, object *env) {
  int bit; builtin_t setf = Context;
  object *arg = nil;
  while (args != NULL) {
    if (cdr(args) == NULL) { Context = setf; error2(oddargs); }
    object **loc = place(first(args), env, &bit);
    arg = eval(second(args), env);
    if (bit == -1) *loc = arg;
    else if (bit < -1) (*loc)->chars = ((*loc)->chars & ~(0xff<<((-bit-2)<<3))) | checkchar(arg)<<((-bit-2)<<3);
    else *loc = number((checkinteger(*loc) & ~(1<<bit)) | checkbitvalue(arg)<<bit);
    args = cddr(args);
  }
  return arg;
}

// Other special forms

object *sp_dolist (object *args, object *env) {
  object *params = checkarguments(args, 2, 3);
  object *var = first(params);
  object *list = eval(second(params), env);
  protect(list); // Don't GC the list
  object *pair = cons(var,nil);
  push(pair,env);
  params = cddr(params);
  args = cdr(args);
  while (list != NULL) {
    if (atom(list)) error(notproper, list);
    cdr(pair) = first(list);
    object *forms = args;
    while (forms != NULL) {
      object *result = eval(car(forms), env);
      if (tstflag(RETURNFLAG)) {
        clrflag(RETURNFLAG);
        unprotect();
        return result;
      }
      forms = cdr(forms);
    }
    list = cdr(list);
  }
  cdr(pair) = nil;
  unprotect();
  if (params == NULL) return nil;
  return eval(car(params), env);
}

object *sp_dotimes (object *args, object *env) {
  object *params = checkarguments(args, 2, 3);
  object *var = first(params);
  int count = checkinteger(eval(second(params), env));
  int index = 0;
  params = cddr(params);
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

object *sp_do (object *args, object *env) {
  return dobody(args, env, false);
}

object *sp_dostar (object *args, object *env) {
  return dobody(args, env, true);
}

object *sp_bind (object *args, object *env) {
  object *params = cons(NULL, cons(first(args), cddr(args)));
  protect(params);
  object *result = closure(0, sym(BIND), params, eval(second(args), env), &env);
  unprotect();
  return eval(result, env);
}

object *sp_trace (object *args, object *env) {
  (void) env;
  while (args != NULL) {
    object *var = first(args);
    if (!symbolp(var)) error(notasymbol, var);
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
      if (!symbolp(var)) error(notasymbol, var);
      untrace(var->name);
      args = cdr(args);
    }
  }
  return args;
}

object *sp_formillis (object *args, object *env) {
  object *param = checkarguments(args, 0, 1);
  unsigned long start = millis();
  unsigned long now, total = 0;
  if (param != NULL) total = checkinteger(eval(first(param), env));
  eval(tf_progn(cdr(args),env), env);
  do {
    now = millis() - start;
    testescape();
  } while (now < total);
  if (now <= INT_MAX) return number(now);
  return nil;
}

object *sp_time (object *args, object *env) {
  unsigned long start = millis();
  object *result = eval(first(args), env);
  unsigned long elapsed = millis() - start;
  printobject(result, pserial);
  pfstring("\nTime: ", pserial);
  if (elapsed < 1000) {
    pint(elapsed, pserial);
    pfstring(" ms\n", pserial);
  } else {
    elapsed = elapsed+50;
    pint(elapsed/1000, pserial);
    pserial('.'); pint((elapsed/100)%10, pserial);
    pfstring(" s\n", pserial);
  }
  return bsymbol(NOTHING);
}

object *sp_withoutputtostring (object *args, object *env) {
  object *params = checkarguments(args, 1, 1);
  if (GlobalStringStreamTail != NULL) error2(streamalreadyopen);
  object *var = first(params);
  object *pair = cons(var, stream(STRINGSTREAM, 0));
  push(pair,env);
  object *string = newstring();
  GlobalStringStreamTail = string;
  protect(string);
  object *forms = cdr(args);
  eval(tf_progn(forms,env), env);
  unprotect();
  GlobalStringStreamTail = NULL;
  return string;
}

object *sp_withserial (object *args, object *env) {
  object *params = checkarguments(args, 2, 3);
  object *var = first(params);
  int address = checkinteger(eval(second(params), env));
  params = cddr(params);
  int baud = 96;
  if (params != NULL) baud = checkinteger(eval(first(params), env));
  object *pair = cons(var, stream(SERIALSTREAM, address));
  push(pair,env);
  serialbegin(address, baud);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  serialend(address);
  return result;
}

object *sp_withi2c (object *args, object *env) {
  object *params = checkarguments(args, 2, 4);
  object *var = first(params);
  int address = checkinteger(eval(second(params), env));
  params = cddr(params);
  if ((address == 0 || address == 1) && params != NULL) {
    address = address * 128 + checkinteger(eval(first(params), env));
    params = cdr(params);
  }
  int read = 0; // Write
  I2Ccount = 0;
  if (params != NULL) {
    object *rw = eval(first(params), env);
    if (integerp(rw)) I2Ccount = rw->integer;
    read = (rw != NULL);
  }
  // Top bit of address is I2C port
  TwoWire *port = &Wire;
  #if ULISP_HOWMANYI2C == 2
  if (address > 127) port = &Wire1;
  #endif
  I2Cinit(port, 1); // Pullups
  object *pair = cons(var, (I2Cstart(port, address & 0x7F, read)) ? stream(I2CSTREAM, address) : nil);
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  I2Cstop(port, read);
  return result;
}

object *sp_withspi (object *args, object *env) {
  object *params = checkarguments(args, 2, 6);
  object *var = first(params);
  params = cdr(params);
  if (params == NULL) error2(nostream);
  int pin = checkinteger(eval(car(params), env));
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  params = cdr(params);
  int clock = 4000, mode = SPI_MODE0; // Defaults
  int bitorder = MSBFIRST;
  if (params != NULL) {
    clock = checkinteger(eval(car(params), env));
    params = cdr(params);
    if (params != NULL) {
      bitorder = (checkinteger(eval(car(params), env)) == 0) ? LSBFIRST : MSBFIRST;
      params = cdr(params);
      if (params != NULL) {
        int modeval = checkinteger(eval(car(params), env));
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
  object *params = checkarguments(args, 2, 3);
  object *var = first(params);
  params = cdr(params);
  if (params == NULL) error2("no filename specified");
  builtin_t temp = Context;
  object *filename = eval(first(params), env);
  Context = temp;
  if (!stringp(filename)) error("filename is not a string", filename);
  params = cdr(params);
  SDBegin();
  int mode = 0;
  if (params != NULL && first(params) != NULL) mode = checkinteger(first(params));
  const char *oflag = FILE_READ;
  if (mode == 1) oflag = FILE_APPEND; else if (mode == 2) oflag = FILE_WRITE;
  if (mode >= 1) {
    char buffer[BUFFERSIZE];
    SDpfile = SD.open(MakeFilename(filename, buffer), oflag);
    if (!SDpfile) error2("problem writing to SD card or invalid filename");
  } else {
    char buffer[BUFFERSIZE];
    SDgfile = SD.open(MakeFilename(filename, buffer), oflag);
    if (!SDgfile) error2("problem reading from SD card or invalid filename");
  }
  object *pair = cons(var, stream(SDSTREAM, 1));
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  if (mode >= 1) SDpfile.close(); else SDgfile.close();
  return result;
  #else
  (void) args, (void) env;
  error2("not supported");
  return nil;
  #endif
}

// Tail-recursive forms

object *tf_progn (object *args, object *env) {
  if (args == NULL) return nil;
  object *more = cdr(args);
  while (more != NULL) {
    object *result = eval(car(args),env);
    if (tstflag(RETURNFLAG)) return quote(result);
    args = more;
    more = cdr(args);
  }
  return car(args);
}

object *tf_if (object *args, object *env) {
  if (args == NULL || cdr(args) == NULL) error2(toofewargs);
  if (eval(first(args), env) != nil) return second(args);
  args = cddr(args);
  return (args != NULL) ? first(args) : nil;
}

object *tf_cond (object *args, object *env) {
  while (args != NULL) {
    object *clause = first(args);
    if (!consp(clause)) error(illegalclause, clause);
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
  if (args == NULL) error2(noargument);
  if (eval(first(args), env) != nil) return tf_progn(cdr(args),env);
  else return nil;
}

object *tf_unless (object *args, object *env) {
  if (args == NULL) error2(noargument);
  if (eval(first(args), env) != nil) return nil;
  else return tf_progn(cdr(args),env);
}

object *tf_case (object *args, object *env) {
  object *test = eval(first(args), env);
  args = cdr(args);
  while (args != NULL) {
    object *clause = first(args);
    if (!consp(clause)) error(illegalclause, clause);
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
  return boundp(first(args), env) ? tee : nil;
}

object *fn_keywordp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!symbolp(arg)) return nil;
  if (colonp(arg->name)) return tee;
  if (keywordp(arg)) return (number((int)lookupfn(builtin(arg->name))));
  return nil;
}

object *fn_setfn (object *args, object *env) {
  object *arg = nil;
  while (args != NULL) {
    if (cdr(args) == NULL) error2(oddargs);
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

object *fn_equal (object *args, object *env) {
  (void) env;
  return equal(first(args), second(args)) ? tee : nil;
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
  return cxxxr(args, 0b100);
}

object *fn_cadr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b101);
}

object *fn_cdar (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b110);
}

object *fn_cddr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b111);
}

object *fn_caaar (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1000);
}

object *fn_caadr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1001);
}

object *fn_cadar (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1010);
}

object *fn_caddr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1011);
}

object *fn_cdaar (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1100);
}

object *fn_cdadr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1101);
}

object *fn_cddar (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1110);
}

object *fn_cdddr (object *args, object *env) {
  (void) env;
  return cxxxr(args, 0b1111);
}

object *fn_length (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (listp(arg)) return number(listlength(arg));
  if (stringp(arg)) return number(stringlength(arg));
  if (!(arrayp(arg) && cdr(cddr(arg)) == NULL)) error("argument is not a list, 1d array, or string", arg);
  return number(abs(first(cddr(arg))->integer));
}

object *fn_arraydimensions (object *args, object *env) {
  (void) env;
  object *array = first(args);
  if (!arrayp(array)) error("argument is not an array", array);
  object *dimensions = cddr(array);
  return (first(dimensions)->integer < 0) ? cons(number(-(first(dimensions)->integer)), cdr(dimensions)) : dimensions;
}

object *fn_list (object *args, object *env) {
  (void) env;
  return args;
}

object *fn_copylist (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!listp(arg)) error(notalist, arg);
  object *result = cons(NULL, NULL);
  object *ptr = result;
  while (consp(arg)) {
    cdr(ptr) = cons(car(arg), cdr(arg)); 
    ptr = cdr(ptr); arg = cdr(arg);
  }
  return cdr(result);
}

object *fn_makearray (object *args, object *env) {
  (void) env;
  object *def = nil;
  bool bitp = false;
  object *dims = first(args);
  if (dims == NULL) error2("dimensions can't be nil");
  else if (atom(dims)) dims = cons(dims, NULL);
  args = cdr(args);
  while (args != NULL && cdr(args) != NULL) {
    object *var = first(args);
    if (isbuiltin(first(args), INITIALELEMENT)) def = second(args);
    else if (isbuiltin(first(args), ELEMENTTYPE) && isbuiltin(second(args), BIT)) bitp = true;
    else error("argument not recognised", var);
    args = cddr(args);
  }
  if (bitp) {
    if (def == nil) def = number(0);
    else def = number(-checkbitvalue(def)); // 1 becomes all ones
  }
  return makearray(dims, def, bitp);
}

object *fn_reverse (object *args, object *env) {
  (void) env;
  object *list = first(args);
  object *result = NULL;
  while (list != NULL) {
    if (atom(list)) error(notproper, list);
    push(first(list),result);
    list = cdr(list);
  }
  return result;
}

object *fn_nth (object *args, object *env) {
  (void) env;
  int n = checkinteger(first(args));
  if (n < 0) error(indexnegative, first(args));
  object *list = second(args);
  while (list != NULL) {
    if (atom(list)) error(notproper, list);
    if (n == 0) return car(list);
    list = cdr(list);
    n--;
  }
  return nil;
}

object *fn_aref (object *args, object *env) {
  (void) env;
  int bit;
  object *array = first(args);
  if (!arrayp(array)) error("first argument is not an array", array);
  object *loc = *getarray(array, cdr(args), 0, &bit);
  if (bit == -1) return loc;
  else return number((loc->integer)>>bit & 1);
}

object *fn_assoc (object *args, object *env) {
  (void) env;
  object *key = first(args);
  object *list = second(args);
  object *test = testargument(cddr(args));
  while (list != NULL) {
    if (atom(list)) error(notproper, list);
    object *pair = first(list);
    if (!listp(pair)) error("element is not a list", pair);
    if (pair != NULL && apply(test, cons(key, cons(car(pair), NULL)), env) != NULL) return pair;
    list = cdr(list);
  }
  return nil;
}

object *fn_member (object *args, object *env) {
  (void) env;
  object *item = first(args);
  object *list = second(args);
  object *test = testargument(cddr(args));
  while (list != NULL) {
    if (atom(list)) error(notproper, list);
    if (apply(test, cons(item, cons(car(list), NULL)), env) != NULL) return list;
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
  if (!listp(arg)) error(notalist, arg);
  cdr(previous) = arg;
  return apply(first(args), cdr(args), env);
}

object *fn_funcall (object *args, object *env) {
  return apply(first(args), cdr(args), env);
}

object *fn_append (object *args, object *env) {
  (void) env;
  object *head = NULL;
  object *tail;
  while (args != NULL) {
    object *list = first(args);
    if (!listp(list)) error(notalist, list);
    while (consp(list)) {
      object *obj = cons(car(list), cdr(list));
      if (head == NULL) head = obj;
      else cdr(tail) = obj;
      tail = obj;
      list = cdr(list);
      if (cdr(args) != NULL && !listp(list)) error(notproper, first(args));
    }
    args = cdr(args);
  }
  return head;
}

object *fn_mapc (object *args, object *env) {
  return mapcl(args, env, false);
}

object *fn_mapl (object *args, object *env) {
  return mapcl(args, env, true);
}

object *fn_mapcar (object *args, object *env) {
  return mapcarcan(args, env, mapcarfun, false);
}

object *fn_mapcan (object *args, object *env) {
  return mapcarcan(args, env, mapcanfun, false);
}

object *fn_maplist (object *args, object *env) {
  return mapcarcan(args, env, mapcarfun, true);
}

object *fn_mapcon (object *args, object *env) {
  return mapcarcan(args, env, mapcanfun, true);
}

// Arithmetic functions

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
    } else error(notanumber, arg);
    args = cdr(args);
  }
  return number(result);
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
      } else error(notanumber, arg);
      args = cdr(args);
    }
    return number(result);
  } else error(notanumber, arg);
  return nil;
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
    } else error(notanumber, arg);
    args = cdr(args);
  }
  return number(result);
}

object *fn_divide (object *args, object *env) {
  (void) env;
  object* arg = first(args);
  args = cdr(args);
  // One argument
  if (args == NULL) {
    if (floatp(arg)) {
      float f = arg->single_float;
      if (f == 0.0) error2(divisionbyzero);
      return makefloat(1.0 / f);
    } else if (integerp(arg)) {
      int i = arg->integer;
      if (i == 0) error2(divisionbyzero);
      else if (i == 1) return number(1);
      else return makefloat(1.0 / i);
    } else error(notanumber, arg);
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
        if (i == 0) error2(divisionbyzero);
        if ((result % i) != 0) return divide_floats(args, result);
        if ((result == INT_MIN) && (i == -1)) return divide_floats(args, result);
        result = result / i;
        args = cdr(args);
      } else error(notanumber, arg);
    }
    return number(result);
  } else error(notanumber, arg);
  return nil;
}

object *fn_mod (object *args, object *env) {
  (void) env;
  return remmod(args, true);
}

object *fn_rem (object *args, object *env) {
  (void) env;
  return remmod(args, false);
}

object *fn_oneplus (object *args, object *env) {
  (void) env;
  object* arg = first(args);
  if (floatp(arg)) return makefloat((arg->single_float) + 1.0);
  else if (integerp(arg)) {
    int result = arg->integer;
    if (result == INT_MAX) return makefloat((arg->integer) + 1.0);
    else return number(result + 1);
  } else error(notanumber, arg);
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
  } else error(notanumber, arg);
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
  } else error(notanumber, arg);
  return nil;
}

object *fn_random (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (integerp(arg)) return number(random(arg->integer));
  else if (floatp(arg)) return makefloat((float)rand()/(float)(RAND_MAX/(arg->single_float)));
  else error(notanumber, arg);
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
    } else if ((checkintfloat(arg) > checkintfloat(result))) result = arg;
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
    } else if ((checkintfloat(arg) < checkintfloat(result))) result = arg;
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
      } else if ((checkintfloat(arg1) == checkintfloat(arg2))) return nil;
      nargs = cdr(nargs);
    }
    args = cdr(args);
  }
  return tee;
}

object *fn_numeq (object *args, object *env) {
  (void) env;
  return compare(args, false, false, true);
}

object *fn_less (object *args, object *env) {
  (void) env;
  return compare(args, true, false, false);
}

object *fn_lesseq (object *args, object *env) {
  (void) env;
  return compare(args, true, false, true);
}

object *fn_greater (object *args, object *env) {
  (void) env;
  return compare(args, false, true, false);
}

object *fn_greatereq (object *args, object *env) {
  (void) env;
  return compare(args, false, true, true);
}

object *fn_plusp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) > 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) > 0) ? tee : nil;
  else error(notanumber, arg);
  return nil;
}

object *fn_minusp (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) < 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) < 0) ? tee : nil;
  else error(notanumber, arg);
  return nil;
}

object *fn_zerop (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (floatp(arg)) return ((arg->single_float) == 0.0) ? tee : nil;
  else if (integerp(arg)) return ((arg->integer) == 0) ? tee : nil;
  else error(notanumber, arg);
  return nil;
}

object *fn_oddp (object *args, object *env) {
  (void) env;
  int arg = checkinteger(first(args));
  return ((arg & 1) == 1) ? tee : nil;
}

object *fn_evenp (object *args, object *env) {
  (void) env;
  int arg = checkinteger(first(args));
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
  return makefloat(sin(checkintfloat(first(args))));
}

object *fn_cos (object *args, object *env) {
  (void) env;
  return makefloat(cos(checkintfloat(first(args))));
}

object *fn_tan (object *args, object *env) {
  (void) env;
  return makefloat(tan(checkintfloat(first(args))));
}

object *fn_asin (object *args, object *env) {
  (void) env;
  return makefloat(asin(checkintfloat(first(args))));
}

object *fn_acos (object *args, object *env) {
  (void) env;
  return makefloat(acos(checkintfloat(first(args))));
}

object *fn_atan (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  float div = 1.0;
  args = cdr(args);
  if (args != NULL) div = checkintfloat(first(args));
  return makefloat(atan2(checkintfloat(arg), div));
}

object *fn_sinh (object *args, object *env) {
  (void) env;
  return makefloat(sinh(checkintfloat(first(args))));
}

object *fn_cosh (object *args, object *env) {
  (void) env;
  return makefloat(cosh(checkintfloat(first(args))));
}

object *fn_tanh (object *args, object *env) {
  (void) env;
  return makefloat(tanh(checkintfloat(first(args))));
}

object *fn_exp (object *args, object *env) {
  (void) env;
  return makefloat(exp(checkintfloat(first(args))));
}

object *fn_sqrt (object *args, object *env) {
  (void) env;
  return makefloat(sqrt(checkintfloat(first(args))));
}

object *fn_log (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  float fresult = log(checkintfloat(arg));
  args = cdr(args);
  if (args == NULL) return makefloat(fresult);
  else return makefloat(fresult / log(checkintfloat(first(args))));
}

object *fn_expt (object *args, object *env) {
  (void) env;
  object *arg1 = first(args); object *arg2 = second(args);
  float float1 = checkintfloat(arg1);
  float value = log(abs(float1)) * checkintfloat(arg2);
  if (integerp(arg1) && integerp(arg2) && ((arg2->integer) >= 0) && (abs(value) < 21.4875))
    return number(intpower(arg1->integer, arg2->integer));
  if (float1 < 0) {
    if (integerp(arg2)) return makefloat((arg2->integer & 1) ? -exp(value) : exp(value));
    else error2("invalid result");
  }
  return makefloat(exp(value));
}

object *fn_ceiling (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg) && integerp(arg2)) {
      int num = arg->integer, den = arg2->integer, quo = num/den, rem = num-(quo*den);
      if (((num<0) != (den<0)) || rem==0) return number(quo); else return number(quo+1);
    } else return number(ceil(checkintfloat(arg) / checkintfloat(first(args))));
  } else {
    if (integerp(arg)) return number(arg->integer);
    else return number(ceil(checkintfloat(arg)));
  }
}

object *fn_floor (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg) && integerp(arg2)) {
      int num = arg->integer, den = arg2->integer, quo = num/den, rem = num-(quo*den);
      if (((num<0) == (den<0)) || rem==0) return number(quo); else return number(quo-1);
    } else return number(floor(checkintfloat(arg) / checkintfloat(first(args))));
  } else {
    if (integerp(arg)) return number(arg->integer);
    else return number(floor(checkintfloat(arg)));
  }
}

object *fn_truncate (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) {
    object *arg2 = first(args);
    if (integerp(arg) && integerp(arg2)) return number(arg->integer / arg2->integer);
    else return number((int)(checkintfloat(arg) / checkintfloat(first(args))));
  } else {
    if (integerp(arg)) return number(arg->integer);
    else return number((int)(checkintfloat(arg)));
  }
}

object *fn_round (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  args = cdr(args);
  if (args != NULL) return number(round(checkintfloat(arg) / checkintfloat(first(args))));
  else return number(round(checkintfloat(arg)));
}

// Characters

object *fn_char (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (!stringp(arg)) error(notastring, arg);
  object *n = second(args);
  char c = nthchar(arg, checkinteger(n));
  if (c == 0) error(indexrange, n);
  return character(c);
}

object *fn_charcode (object *args, object *env) {
  (void) env;
  return number(checkchar(first(args)));
}

object *fn_codechar (object *args, object *env) {
  (void) env;
  return character(checkinteger(first(args)));
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

object *fn_stringeq (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, false, false, true);
  return m == -1 ? nil : tee;
}

object *fn_stringless (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, true, false, false);
  return m == -1 ? nil : number(m);
}

object *fn_stringgreater (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, false, true, false);
  return m == -1 ? nil : number(m);
}

object *fn_stringnoteq (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, true, true, false);
  return m == -1 ? nil : number(m);
}

object *fn_stringlesseq (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, true, false, true);
  return m == -1 ? nil : number(m);
}

object *fn_stringgreatereq (object *args, object *env) {
  (void) env;
  int m = stringcompare(args, false, true, true);
  return m == -1 ? nil : number(m);
}

object *fn_sort (object *args, object *env) {
  object *arg = first(args);
  if (!listp(arg)) error(notalist, arg);
  if (arg == NULL) return nil;
  object *list = cons(nil, arg);
  protect(list);
  object *predicate = second(args);
  object *compare = cons(NULL, cons(NULL, NULL));
  protect(compare);
  object *ptr = cdr(list);
  while (cdr(ptr) != NULL) {
    object *go = list;
    while (go != ptr) {
      car(compare) = car(cdr(ptr));
      car(cdr(compare)) = car(cdr(go));
      if (apply(predicate, compare, env)) break;
      go = cdr(go);
    }
    if (go != ptr) {
      object *obj = cdr(ptr);
      cdr(ptr) = cdr(obj);
      cdr(obj) = cdr(go);
      cdr(go) = obj;
    } else ptr = cdr(ptr);
  }
  unprotect(); unprotect();
  return cdr(list);
}

object *fn_stringfn (object *args, object *env) {
  return fn_princtostring(args, env);
}

object *fn_concatenate (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  if (builtin(arg->name) != STRINGFN) error2("only supports strings");
  args = cdr(args);
  object *result = newstring();
  object *tail = result;
  while (args != NULL) {
    object *obj = checkstring(first(args));
    obj = cdr(obj);
    while (obj != NULL) {
      int quad = obj->chars;
      while (quad != 0) {
         char ch = quad>>((sizeof(int)-1)*8) & 0xFF;
         buildstring(ch, &tail);
         quad = quad<<8;
      }
      obj = car(obj);
    }
    args = cdr(args);
  }
  return result;
}

object *fn_subseq (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  int start = checkinteger(second(args)), end;
  if (start < 0) error(indexnegative, second(args));
  args = cddr(args);
  if (listp(arg)) {
    int length = listlength(arg);
    if (args != NULL) end = checkinteger(car(args)); else end = length;
    if (start > end || end > length) error2(indexrange);
    object *result = cons(NULL, NULL);
    object *ptr = result;
    for (int x = 0; x < end; x++) {
      if (x >= start) { cdr(ptr) = cons(car(arg), NULL); ptr = cdr(ptr); }
      arg = cdr(arg);
    }
    return cdr(result);
  } else if (stringp(arg)) {
    int length = stringlength(arg);
    if (args != NULL) end = checkinteger(car(args)); else end = length;
    if (start > end || end > length) error2(indexrange);
    object *result = newstring();
    object *tail = result;
    for (int i=start; i<end; i++) {
      char ch = nthchar(arg, i);
      buildstring(ch, &tail);
    }
    return result;
  } else error2("argument is not a list or string");
  return nil;
}

object *fn_search (object *args, object *env) {
  (void) env;
  object *pattern = first(args);
  object *target = second(args);
  if (pattern == NULL) return number(0);
  else if (target == NULL) return nil;
  
  else if (listp(pattern) && listp(target)) {
    object *test = testargument(cddr(args));
    int l = listlength(target);
    int m = listlength(pattern);
    for (int i = 0; i <= l-m; i++) {
      object *target1 = target;
      while (pattern != NULL && apply(test, cons(car(target1), cons(car(pattern), NULL)), env) != NULL) {
        pattern = cdr(pattern);
        target1 = cdr(target1);
      }
      if (pattern == NULL) return number(i);
      pattern = first(args); target = cdr(target);
    }
    return nil;

  } else if (stringp(pattern) && stringp(target)) {
    if (cddr(args) != NULL) error2("keyword argument not supported for strings");
    int l = stringlength(target);
    int m = stringlength(pattern);
    for (int i = 0; i <= l-m; i++) {
      int j = 0;
      while (j < m && nthchar(target, i+j) == nthchar(pattern, j)) j++;
      if (j == m) return number(i);
    }
    return nil;
  } else error2("arguments are not both lists or strings");
  return nil;
}

object *fn_readfromstring (object *args, object *env) {
  (void) env;
  object *arg = checkstring(first(args));
  if (stringlength(arg) == 0) error2("zero length string");
  GlobalString = arg;
  GlobalStringIndex = 0;
  object *val = readmain(gstr);
  return val;
}

object *fn_princtostring (object *args, object *env) {
  (void) env;
  return princtostring(first(args));
}

object *fn_prin1tostring (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  object *obj = startstring();
  printobject(arg, pstr);
  return obj;
}

// Bitwise operators

object *fn_logand (object *args, object *env) {
  (void) env;
  int result = -1;
  while (args != NULL) {
    result = result & checkinteger(first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_logior (object *args, object *env) {
  (void) env;
  int result = 0;
  while (args != NULL) {
    result = result | checkinteger(first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_logxor (object *args, object *env) {
  (void) env;
  int result = 0;
  while (args != NULL) {
    result = result ^ checkinteger(first(args));
    args = cdr(args);
  }
  return number(result);
}

object *fn_lognot (object *args, object *env) {
  (void) env;
  int result = checkinteger(car(args));
  return number(~result);
}

object *fn_ash (object *args, object *env) {
  (void) env;
  int value = checkinteger(first(args));
  int count = checkinteger(second(args));
  if (count >= 0) return number(value << count);
  else return number(value >> abs(count));
}

object *fn_logbitp (object *args, object *env) {
  (void) env;
  int index = checkinteger(first(args));
  int value = checkinteger(second(args));
  return (bitRead(value, index) == 1) ? tee : nil;
}

// System functions

object *fn_eval (object *args, object *env) {
  return eval(first(args), env);
}

object *fn_return (object *args, object *env) {
  (void) env;
  setflag(RETURNFLAG);
  if (args == NULL) return nil; else return first(args);
}

object *fn_globals (object *args, object *env) {
  (void) args, (void) env;
  object *result = cons(NULL, NULL);
  object *ptr = result;
  object *arg = GlobalEnv;
  while (arg != NULL) {
    cdr(ptr) = cons(car(car(arg)), NULL); ptr = cdr(ptr);
    arg = cdr(arg);
  }
  return cdr(result);
}

object *fn_locals (object *args, object *env) {
  (void) args;
  return env;
}

object *fn_makunbound (object *args, object *env) {
  (void) env;
  object *var = first(args);
  if (!symbolp(var)) error(notasymbol, var);
  delassoc(var, &GlobalEnv);
  return var;
}

object *fn_break (object *args, object *env) {
  (void) args;
  pfstring("\nBreak!\n", pserial);
  BreakLevel++;
  repl(env);
  BreakLevel--;
  return nil;
}

object *fn_read (object *args, object *env) {
  (void) env;
  gfun_t gfun = gstreamfun(args);
  return readmain(gfun);
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
  if (gfun == gserial) gserial_flush();
  int c = gfun();
  return (c == -1) ? nil : number(c);
}

object *fn_readline (object *args, object *env) {
  (void) env;
  gfun_t gfun = gstreamfun(args);
  if (gfun == gserial) gserial_flush();
  return readstring('\n', false, gfun);
}

object *fn_writebyte (object *args, object *env) {
  (void) env;
  int c = checkinteger(first(args));
  pfun_t pfun = pstreamfun(cdr(args));
  if (c == '\n' && pfun == pserial) Serial.write('\n');
  else (pfun)(c);
  return nil;
}

object *fn_writestring (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  flags_t temp = Flags;
  clrflag(PRINTREADABLY);
  printstring(obj, pfun);
  Flags = temp;
  return nil;
}

object *fn_writeline (object *args, object *env) {
  (void) env;
  object *obj = first(args);
  pfun_t pfun = pstreamfun(cdr(args));
  flags_t temp = Flags;
  clrflag(PRINTREADABLY);
  printstring(obj, pfun);
  pln(pfun);
  Flags = temp;
  return nil;
}

object *fn_restarti2c (object *args, object *env) {
  (void) env;
  int stream = isstream(first(args));
  args = cdr(args);
  int read = 0; // Write
  I2Ccount = 0;
  if (args != NULL) {
    object *rw = first(args);
    if (integerp(rw)) I2Ccount = rw->integer;
    read = (rw != NULL);
  }
  int address = stream & 0xFF;
  if (stream>>8 != I2CSTREAM) error2("not an i2c stream");
  TwoWire *port;
  if (address < 128) port = &Wire;
  #if ULISP_HOWMANYI2C == 2
  else port = &Wire1;
  #endif
  return I2Crestart(port, address & 0x7F, read) ? tee : nil;
}

object *fn_gc (object *args, object *env) {
  if (args == NULL || first(args) != NULL) {
    int initial = Freespace;
    unsigned long start = micros();
    gc(args, env);
    unsigned long elapsed = micros() - start;
    pfstring("Space: ", pserial);
    pint(Freespace - initial, pserial);
    pfstring(" bytes, Time: ", pserial);
    pint(elapsed, pserial);
    pfstring(" us\n", pserial);
  } else gc(args, env);
  return nil;
}

object *fn_room (object *args, object *env) {
  (void) args, (void) env;
  return number(Freespace);
}

object *fn_backtrace (object *args, object *env) {
  (void) env;
  if (args == NULL) return (tstflag(BACKTRACE)) ? tee : nil;
  if (first(args) == NULL) clrflag(BACKTRACE); else setflag(BACKTRACE);
  return first(args);
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
  (void) env; int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(arg);
  else pin = checkinteger(first(args));
  int pm = INPUT;
  arg = second(args);
  if (keywordp(arg)) pm = checkkeyword(arg);
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
  int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(arg);
  else pin = checkinteger(arg);
  if (digitalRead(pin) != 0) return tee; else return nil;
}

object *fn_digitalwrite (object *args, object *env) {
  (void) env;
  int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(arg);
  else pin = checkinteger(arg);
  arg = second(args);
  int mode;
  if (keywordp(arg)) mode = checkkeyword(arg);
  else if (integerp(arg)) mode = arg->integer ? HIGH : LOW;
  else mode = (arg != nil) ? HIGH : LOW;
  digitalWrite(pin, mode);
  return arg;
}

object *fn_analogread (object *args, object *env) {
  (void) env;
  int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(arg);
  else {
    pin = checkinteger(arg);
    checkanalogread(pin);
  }
  return number(analogRead(pin));
}

object *fn_analogreadresolution (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  #if defined(ESP32)
  analogReadResolution(checkinteger(arg));
  #else
  error2("not supported");
  #endif
  return arg;
}

object *fn_analogwrite (object *args, object *env) {
  (void) env;
  int pin;
  object *arg = first(args);
  if (keywordp(arg)) pin = checkkeyword(arg);
  else pin = checkinteger(arg);
  checkanalogwrite(pin);
  object *value = second(args);
  analogWrite(pin, checkinteger(value));
  return value;
}

object *fn_delay (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  unsigned long start = millis();
  unsigned long total = checkinteger(arg1);
  do testescape();
  while (millis() - start < total);
  return arg1;
}

object *fn_millis (object *args, object *env) {
  (void) args, (void) env;
  return number(millis());
}

object *fn_sleep (object *args, object *env) {
  (void) env;
  object *arg1 = first(args);
  doze(checkinteger(arg1));
  return arg1;
}

object *fn_note (object *args, object *env) {
  (void) env;
  static int pin = 255;
  if (args != NULL) {
    pin = checkinteger(first(args));
    int note = 48, octave = 0;
    if (cdr(args) != NULL) {
      note = checkinteger(second(args));
      if (cddr(args) != NULL) octave = checkinteger(third(args));
    }
    playnote(pin, note, octave);
  } else nonote(pin);
  return nil;
}

object *fn_register (object *args, object *env) {
  (void) env;
  object *arg = first(args);
  int addr;
  if (keywordp(arg)) addr = checkkeyword(arg);
  else addr = checkinteger(first(args));
  if (cdr(args) == NULL) return number(*(uint32_t *)addr);
  (*(uint32_t *)addr) = checkinteger(second(args));
  return second(args);
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
  return bsymbol(NOTHING);
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
    if (consp(val) && symbolp(car(val)) && builtin(car(val)->name) == LAMBDA) {
      superprint(cons(bsymbol(DEFUN), cons(var, cdr(val))), 0, pfun);
    } else {
      superprint(cons(bsymbol(DEFVAR), cons(var, cons(quote(val), NULL))), 0, pfun);
    }
    pln(pfun);
    testescape();
    globals = cdr(globals);
  }
  ppwidth = PPWIDTH;
  return bsymbol(NOTHING);
}

// Format

object *fn_format (object *args, object *env) {
  (void) env;
  pfun_t pfun = pserial;
  object *output = first(args);
  object *obj;
  if (output == nil) { obj = startstring(); pfun = pstr; }
  else if (!eq(output, tee)) pfun = pstreamfun(args);
  object *formatstr = checkstring(second(args));
  object *save = NULL;
  args = cddr(args);
  uint16_t len = stringlength(formatstr);
  uint16_t n = 0, width = 0, w, bra = 0;
  char pad = ' ';
  bool tilde = false, mute = false, comma = false, quote = false;
  while (n < len) {
    char ch = nthchar(formatstr, n);
    char ch2 = ch & ~0x20; // force to upper case
    if (tilde) {
     if (ch == '}') {
        if (save == NULL) formaterr(formatstr, "no matching ~{", n);
        if (args == NULL) { args = cdr(save); save = NULL; } else n = bra;
        mute = false; tilde = false;
      }
      else if (!mute) {
        if (comma && quote) { pad = ch; comma = false, quote = false; }
        else if (ch == '\'') {
          if (comma) quote = true;
          else formaterr(formatstr, "quote not valid", n);
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
          if (save != NULL) formaterr(formatstr, "can't nest ~{", n);
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
              uint8_t base = (ch2 == 'B') ? 2 : 16;
              uint8_t hw = basewidth(arg, base); if (width < hw) w = 0; else w = width-hw;
              indent(w, pad, pfun); pintbase(arg->integer, base, pfun);
            } else {
              indent(w, pad, pfun); prin1object(arg, pfun);
            }
          }
          tilde = false;
        } else formaterr(formatstr, "invalid directive", n);
      }
    } else {
      if (ch == '~') { tilde = true; pad = ' '; width = 0; comma = false; quote = false; }
      else if (!mute) pfun(ch);
    }
    n++;
  }
  if (output == nil) return obj;
  else return nil;
}

// LispLibrary

object *fn_require (object *args, object *env) {
  object *arg = first(args);
  object *globals = GlobalEnv;
  if (!symbolp(arg)) error(notasymbol, arg);
  while (globals != NULL) {
    object *pair = first(globals);
    object *var = car(pair);
    if (symbolp(var) && var == arg) return nil;
    globals = cdr(globals);
  }
  GlobalStringIndex = 0;
  object *line = readmain(glibrary);
  while (line != NULL) {
    // Is this the definition we want
    symbol_t fname = first(line)->name;
    if ((fname == sym(DEFUN) || fname == sym(DEFVAR)) && symbolp(second(line)) && second(line)->name == arg->name) {
      eval(line, env);
      return tee;
    }
    line = readmain(glibrary);
  }
  return nil;
}

object *fn_listlibrary (object *args, object *env) {
  (void) args, (void) env;
  GlobalStringIndex = 0;
  object *line = readmain(glibrary);
  while (line != NULL) {
    builtin_t bname = builtin(first(line)->name);
    if (bname == DEFUN || bname == DEFVAR) {
      printsymbol(second(line), pserial); pserial(' ');
    }
    line = readmain(glibrary);
  }
  return bsymbol(NOTHING);
}

// Documentation

object *sp_help (object *args, object *env) {
  if (args == NULL) error2(noargument);
  object *docstring = documentation(first(args), env);
  if (docstring) {
    flags_t temp = Flags;
    clrflag(PRINTREADABLY);
    printstring(docstring, pserial);
    Flags = temp;
  }
  return bsymbol(NOTHING);
}

object *fn_documentation (object *args, object *env) {
  return documentation(first(args), env);
}

object *fn_apropos (object *args, object *env) {
  (void) env;
  apropos(first(args), true);
  return bsymbol(NOTHING);
}

object *fn_aproposlist (object *args, object *env) {
  (void) env;
  return apropos(first(args), false);
}

// Error handling

object *sp_unwindprotect (object *args, object *env) {
  if (args == NULL) error2(toofewargs);
  object *current_GCStack = GCStack;
  jmp_buf dynamic_handler;
  jmp_buf *previous_handler = handler;
  handler = &dynamic_handler;
  object *protected_form = first(args);
  object *volatile result;
  // volatile to solve: argument 'result' might be clobbered by 'longjmp' or 'vfork' [-Wclobbered]
  bool signaled = false;
  if (!setjmp(dynamic_handler)) {
    result = eval(protected_form, env);
  } else {
    GCStack = current_GCStack;
    signaled = true;
  }
  handler = previous_handler;

  object *protective_forms = cdr(args);
  while (protective_forms != NULL) {
    eval(car(protective_forms), env);
    if (tstflag(RETURNFLAG)) break;
    protective_forms = cdr(protective_forms);
  }

  if (!signaled) return result;
  GCStack = NULL;
  longjmp(*handler, 1);
}

object *sp_ignoreerrors (object *args, object *env) {
  object *volatile args1 = args;
  // volatile declarations to solve: argument 'args' might be clobbered by 'longjmp' or 'vfork' [-Wclobbered]
  object *current_GCStack = GCStack;
  jmp_buf dynamic_handler;
  jmp_buf *previous_handler = handler;
  handler = &dynamic_handler;
  object *volatile result = nil;

  bool muffled = tstflag(MUFFLEERRORS);
  setflag(MUFFLEERRORS);
  volatile bool signaled = false;
  if (!setjmp(dynamic_handler)) {
    while (args1 != NULL) {
      result = eval(car(args1), env);
      if (tstflag(RETURNFLAG)) break;
      args1 = cdr(args1);
    }
  } else {
    GCStack = current_GCStack;
    signaled = true;
  }
  handler = previous_handler;
  if (!muffled) clrflag(MUFFLEERRORS);

  if (signaled) return bsymbol(NOTHING);
  else return result;
}

object *sp_error (object *args, object *env) {
  object *message = eval(cons(bsymbol(FORMAT), cons(nil, args)), env);
  if (!tstflag(MUFFLEERRORS)) {
    flags_t temp = Flags;
    clrflag(PRINTREADABLY);
    pfstring("Error: ", pserial); printstring(message, pserial);
    Flags = temp;
    pln(pserial);
  }
  GCStack = NULL;
  longjmp(*handler, 1);
}

// SD Card utilities

object *fn_directory (object *args, object *env) {
  (void) args, (void) env;
  #if defined(sdcardsupport)
  SDBegin();
  File root = SD.open("/");
  if (!root) error2("problem reading from SD card");
  object *result = cons(NULL, NULL);
  object *ptr = result;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    object *filename = lispstring((char*)entry.name());
    cdr(ptr) = cons(filename, NULL);
    ptr = cdr(ptr);
    entry.close();
  }
  root.close();
  return cdr(result);
  #else
  error2("not supported");
  return nil;
  #endif
}

// Wi-Fi

object *sp_withclient (object *args, object *env) {
  object *params = checkarguments(args, 1, 3);
  object *var = first(params);
  char buffer[BUFFERSIZE];
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
    if (stringp(address)) success = client.connect(cstring(address, buffer, BUFFERSIZE), checkinteger(port));
    else if (integerp(address)) success = client.connect(address->integer, checkinteger(port));
    else error2("invalid address");
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

object *fn_available (object *args, object *env) {
  (void) env;
  if (isstream(first(args))>>8 != WIFISTREAM) error2("invalid stream");
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
    bool hidden = false;
    if (args != NULL) {
      channel = checkinteger(first(args));
      args = cdr(args);
      if (args != NULL) hidden = (first(args) != nil);
    }
    WiFi.softAP(cstring(first, ssid, 33), cstring(second, pass, 65), channel, hidden);
  }
  return iptostring(WiFi.softAPIP());
}

object *fn_connected (object *args, object *env) {
  (void) env;
  if (isstream(first(args))>>8 != WIFISTREAM) error2("invalid stream");
  return client.connected() ? tee : nil;
}

object *fn_wifilocalip (object *args, object *env) {
  (void) args, (void) env;
  return iptostring(WiFi.localIP());
}

object *fn_wificonnect (object *args, object *env) {
  (void) env;
  char ssid[33], pass[65];
  if (args == NULL) { WiFi.disconnect(true); return nil; }
  if (cdr(args) == NULL) WiFi.begin(cstring(first(args), ssid, 33));
  else WiFi.begin(cstring(first(args), ssid, 33), cstring(second(args), pass, 65));
  int result = WiFi.waitForConnectResult();
  if (result == WL_CONNECTED) {
    configTime(0, 0, "pool.ntp.org");
    return iptostring(WiFi.localIP());
  }
  else if (result == WL_NO_SSID_AVAIL) error2("network not found");
  else if (result == WL_CONNECT_FAILED) error2("connection failed");
  else if (result == WL_DISCONNECTED) error2("already connected");
  else error2("unable to connect");
  return nil;
}

object *fn_gettime (object *args, object *env) {
  #if defined (ULISP_WIFI)
  (void) args, (void) env;
  time_t now;
  time(&now);                      // For ESP32 boards
  if (now < 31536000) return nil;  // Not connected or ready
  now = now + TZ_OFFSET;
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo); // UTC broken-down time
  return cons(number(timeinfo.tm_year + 1900),
    cons(number(timeinfo.tm_mon + 1),
    cons(number(timeinfo.tm_mday),
    cons(number(timeinfo.tm_hour),
    cons(number(timeinfo.tm_min),
    cons(number(timeinfo.tm_sec),
    cons(number((timeinfo.tm_wday + 6) % 7 + 1),
    cons(number(timeinfo.tm_yday + 1), NULL))))))));
  #else
  (void) args, (void) env;
  error2("not supported");
  return nil;
  #endif
}

// Graphics functions

object *sp_withgfx (object *args, object *env) {
#if defined(gfxsupport)
  object *params = checkarguments(args, 1, 1);
  object *var = first(params);
  object *pair = cons(var, stream(GFXSTREAM, 1));
  push(pair,env);
  object *forms = cdr(args);
  object *result = eval(tf_progn(forms,env), env);
  return result;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}

object *fn_drawpixel (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t colour = COLOR_WHITE;
  if (cddr(args) != NULL) colour = checkinteger(third(args));
  tft.drawPixel(checkinteger(first(args)), checkinteger(second(args)), colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawline (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.drawLine(params[0], params[1], params[2], params[3], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawrect (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.drawRect(params[0], params[1], params[2], params[3], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_fillrect (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[4], colour = COLOR_WHITE;
  for (int i=0; i<4; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.fillRect(params[0], params[1], params[2], params[3], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawcircle (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[3], colour = COLOR_WHITE;
  for (int i=0; i<3; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.drawCircle(params[0], params[1], params[2], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_fillcircle (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[3], colour = COLOR_WHITE;
  for (int i=0; i<3; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.fillCircle(params[0], params[1], params[2], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawroundrect (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[5], colour = COLOR_WHITE;
  for (int i=0; i<5; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.drawRoundRect(params[0], params[1], params[2], params[3], params[4], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_fillroundrect (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[5], colour = COLOR_WHITE;
  for (int i=0; i<5; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.fillRoundRect(params[0], params[1], params[2], params[3], params[4], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawtriangle (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[6], colour = COLOR_WHITE;
  for (int i=0; i<6; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.drawTriangle(params[0], params[1], params[2], params[3], params[4], params[5], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_filltriangle (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t params[6], colour = COLOR_WHITE;
  for (int i=0; i<6; i++) { params[i] = checkinteger(car(args)); args = cdr(args); }
  if (args != NULL) colour = checkinteger(car(args));
  tft.fillTriangle(params[0], params[1], params[2], params[3], params[4], params[5], colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_drawchar (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t colour = COLOR_WHITE, bg = COLOR_BLACK, size = 1;
  object *more = cdr(cddr(args));
  if (more != NULL) {
    colour = checkinteger(car(more));
    more = cdr(more);
    if (more != NULL) {
      bg = checkinteger(car(more));
      more = cdr(more);
      if (more != NULL) size = checkinteger(car(more));
    }
  }
  tft.drawChar(checkinteger(first(args)), checkinteger(second(args)), checkchar(third(args)),
    colour, bg, size);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_setcursor (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  tft.setCursor(checkinteger(first(args)), checkinteger(second(args)));
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_settextcolor (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  if (cdr(args) != NULL) tft.setTextColor(checkinteger(first(args)), checkinteger(second(args)));
  else tft.setTextColor(checkinteger(first(args)));
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_settextsize (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  tft.setTextSize(checkinteger(first(args)));
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_settextwrap (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  tft.setTextWrap(first(args) != NULL);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_fillscreen (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  uint16_t colour = COLOR_BLACK;
  if (args != NULL) colour = checkinteger(first(args));
  tft.fillScreen(colour);
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_setrotation (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  tft.setRotation(checkinteger(first(args)));
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_invertdisplay (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
  tft.invertDisplay(first(args) != NULL);
  #else
  (void) args;
  #endif
  return nil;
}

uint16_t readpixel (uint16_t x, uint16_t y) {
  #if defined(gfxsupport)
    #if defined(TFT_ST7789)
      uint32_t ret = 0;
      tft.startWrite();
      tft.setAddrWindow(x, y, 1, 1);
      tft.writeCommand(ST77XX_RAMRD);
      pinMode(TFT_MOSI, INPUT); pinMode(TFT_SCK, OUTPUT);
      for (int i=0; i<33; i++) {
        #if defined(TFT_ST7735)
          digitalWrite(TFT_SCK, HIGH); ret = ret<<1 | digitalRead(TFT_MOSI);
        #else
          ret = ret<<1 | digitalRead(TFT_MOSI); digitalWrite(TFT_SCK, HIGH);
        #endif
        digitalWrite(TFT_SCK, LOW);
      }
      pinMode(TFT_MOSI, OUTPUT);
      tft.endWrite();
      return ((ret & 0xf80000)>>8 | (ret & 0xfc00)>>5 | (ret & 0xf8)>>3);
    #endif
  #endif
  (void) x, (void) y;
  return 0;
}

object *fn_readpixel (object *args, object *env) {
  (void) env;
  #if defined(gfxsupport)
    int x = checkinteger(first(args)), y = checkinteger(second(args));
    #if defined(ARDUINO_M5STACK_TAB5)
    return number(tft.readPixel(x, y));
    #else
    return number(readpixel(x, y));
    #endif
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_touchscreen (object *args, object *env) {
  (void) args, (void) env;
  #if defined(gfxsupport) && defined(ARDUINO_M5STACK_TAB5)
    M5.update();
    touch = M5.Touch.getDetail();
    if (!touch.isPressed()) return nil;
    return cons(number(touch.x), cons(number(touch.y), NULL)); 
  #else
  (void) args;
  #endif
  return nil;
}

object *fn_displaysize (object *args, object *env) {
  (void) args, (void) env;
  #if defined(gfxsupport)
  return cons(number(tft.width()), cons(number(tft.height()), NULL));
  #else
  return nil;
  #endif
}

object *fn_display (object *args, object *env) {
  (void) args, (void) env;
 #if defined(gfxsupport) && defined(ARDUINO_TTGO_LoRa32_v21new)
  tft.display();
  #endif
  return nil;
}

// Documentation strings
const char doc_nil[] = "nil\n"
"A symbol equivalent to the empty list (). Also represents false.";
const char doc_tee[] = "t\n"
"A symbol representing true.";
const char doc_nothing[] = "nothing\n"
"A symbol with no value.\n"
"It is useful if you want to suppress printing the result of evaluating a function.";
const char doc_optional[] = "&optional\n"
"Can be followed by one or more optional parameters in a lambda or defun parameter list.";
const char doc_features[] = "*features*\n"
"Returns a list of keywords representing features supported by this platform.";
const char doc_amprest[] = "&rest\n"
"Can be followed by a parameter in a lambda or defun parameter list,\n"
"and is assigned a list of the corresponding arguments.";
const char doc_lambda[] = "(lambda (parameter*) form*)\n"
"Creates an unnamed function with parameters. The body is evaluated with the parameters as local variables\n"
"whose initial values are defined by the values of the forms after the lambda form.";
const char doc_let[] = "(let ((var value) ... ) forms*)\n"
"Declares local variables with values, and evaluates the forms with those local variables.";
const char doc_letstar[] = "(let* ((var value) ... ) forms*)\n"
"Declares local variables with values, and evaluates the forms with those local variables.\n"
"Each declaration can refer to local variables that have been defined earlier in the let*.";
const char doc_defun[] = "(defun name (parameters) form*)\n"
"Defines a function.";
const char doc_defvar[] = "(defvar variable form)\n"
"Defines a global variable.";
const char doc_bind[] = "(bind (var*) (form) body)\n"
"Binds each variable to the corresponding item in the list returned by evaluating form, and then evaluates body with that binding.";
const char doc_eq[] = "(eq item item)\n"
"Tests whether the two arguments are the same symbol, same character, equal numbers,\n"
"or point to the same cons, and returns t or nil as appropriate.";
const char doc_car[] = "(car list)\n"
"Returns the first item in a list.";
const char doc_first[] = "(first list)\n"
"Returns the first item in a list. Equivalent to car.";
const char doc_cdr[] = "(cdr list)\n"
"Returns a list with the first item removed.";
const char doc_rest[] = "(rest list)\n"
"Returns a list with the first item removed. Equivalent to cdr.";
const char doc_nth[] = "(nth number list)\n"
"Returns the nth item in list, counting from zero.";
const char doc_aref[] = "(aref array index [index*])\n"
"Returns an element from the specified array.";
const char doc_char[] = "(char string n)\n"
"Returns the nth character in a string, counting from zero.";
const char doc_stringfn[] = "(string item)\n"
"Converts its argument to a string.";
const char doc_pinmode[] = "(pinmode pin mode)\n"
"Sets the input/output mode of an Arduino pin number, and returns nil.\n"
"The mode parameter can be an integer, a keyword, or t or nil.";
const char doc_digitalwrite[] = "(digitalwrite pin state)\n"
"Sets the state of the specified Arduino pin number.";
const char doc_analogread[] = "(analogread pin)\n"
"Reads the specified Arduino analogue pin number and returns the value.";
const char doc_register[] = "(register address [value])\n"
"Reads or writes the value of a peripheral register.\n"
"If value is not specified the function returns the value of the register at address.\n"
"If value is specified the value is written to the register at address and the function returns value.";
const char doc_format[] = "(format output controlstring [arguments]*)\n"
"Outputs its arguments formatted according to the format directives in controlstring.";
const char doc_or[] = "(or item*)\n"
"Evaluates its arguments until one returns non-nil, and returns its value.";
const char doc_setq[] = "(setq symbol value [symbol value]*)\n"
"For each pair of arguments assigns the value of the second argument\n"
"to the variable specified in the first argument.";
const char doc_loop[] = "(loop forms*)\n"
"Executes its arguments repeatedly until one of the arguments calls (return),\n"
"which then causes an exit from the loop.";
const char doc_push[] = "(push item place)\n"
"Modifies the value of place, which should be a list, to add item onto the front of the list,\n"
"and returns the new list.";
const char doc_pop[] = "(pop place)\n"
"Modifies the value of place, which should be a non-nil list, to remove its first item,\n"
"and returns that item.";
const char doc_incf[] = "(incf place [number])\n"
"Increments a place, which should have an numeric value, and returns the result.\n"
"The third argument is an optional increment which defaults to 1.";
const char doc_decf[] = "(decf place [number])\n"
"Decrements a place, which should have an numeric value, and returns the result.\n"
"The third argument is an optional decrement which defaults to 1.";
const char doc_setf[] = "(setf place value [place value]*)\n"
"For each pair of arguments modifies a place to the result of evaluating value.";
const char doc_dolist[] = "(dolist (var list [result]) form*)\n"
"Sets the local variable var to each element of list in turn, and executes the forms.\n"
"It then returns result, or nil if result is omitted.";
const char doc_dotimes[] = "(dotimes (var number [result]) form*)\n"
"Executes the forms number times, with the local variable var set to each integer from 0 to number-1 in turn.\n"
"It then returns result, or nil if result is omitted.";
const char doc_do[] = "(do ((var [init [step]])*) (end-test result*) form*)\n"
"Accepts an arbitrary number of iteration vars, which are initialised to init and stepped by step sequentially.\n"
"The forms are executed until end-test is true. It returns result.";
const char doc_dostar[] = "(do* ((var [init [step]])*) (end-test result*) form*)\n"
"Accepts an arbitrary number of iteration vars, which are initialised to init and stepped by step in parallel.\n"
"The forms are executed until end-test is true. It returns result.";
const char doc_trace[] = "(trace [function]*)\n"
"Turns on tracing of up to TRACEMAX user-defined functions,\n"
"and returns a list of the functions currently being traced.";
const char doc_untrace[] = "(untrace [function]*)\n"
"Turns off tracing of up to TRACEMAX user-defined functions, and returns a list of the functions untraced.\n"
"If no functions are specified it untraces all functions.";
const char doc_formillis[] = "(for-millis ([number]) form*)\n"
"Executes the forms and then waits until a total of number milliseconds have elapsed.\n"
"Returns the total number of milliseconds taken.";
const char doc_time[] = "(time form)\n"
"Prints the value returned by the form, and the time taken to evaluate the form\n"
"in milliseconds or seconds.";
const char doc_withoutputtostring[] = "(with-output-to-string (str) form*)\n"
"Returns a string containing the output to the stream variable str.";
const char doc_withserial[] = "(with-serial (str port [baud]) form*)\n"
"Evaluates the forms with str bound to a serial-stream using port.\n"
"The optional baud gives the baud rate divided by 100, default 96.";
const char doc_withi2c[] = "(with-i2c (str [port] address [read-p]) form*)\n"
"Evaluates the forms with str bound to an i2c-stream defined by address.\n"
"If read-p is nil or omitted the stream is written to, otherwise it specifies the number of bytes\n"
"to be read from the stream. If port is omitted it defaults to 0, otherwise it specifies the port, 0 or 1.";
const char doc_withspi[] = "(with-spi (str pin [clock] [bitorder] [mode]) form*)\n"
"Evaluates the forms with str bound to an spi-stream.\n"
"The parameters specify the enable pin, clock in kHz (default 4000),\n"
"bitorder 0 for LSBFIRST and 1 for MSBFIRST (default 1), and SPI mode (default 0).";
const char doc_withsdcard[] = "(with-sd-card (str filename [mode]) form*)\n"
"Evaluates the forms with str bound to an sd-stream reading from or writing to the file filename.\n"
"If mode is omitted the file is read, otherwise 0 means read, 1 write-append, or 2 write-overwrite.";
const char doc_progn[] = "(progn form*)\n"
"Evaluates several forms grouped together into a block, and returns the result of evaluating the last form.";
const char doc_if[] = "(if test then [else])\n"
"Evaluates test. If it's non-nil the form then is evaluated and returned;\n"
"otherwise the form else is evaluated and returned.";
const char doc_cond[] = "(cond ((test form*) (test form*) ... ))\n"
"Each argument is a list consisting of a test optionally followed by one or more forms.\n"
"If the test evaluates to non-nil the forms are evaluated, and the last value is returned as the result of the cond.\n"
"If the test evaluates to nil, none of the forms are evaluated, and the next argument is processed in the same way.";
const char doc_when[] = "(when test form*)\n"
"Evaluates the test. If it's non-nil the forms are evaluated and the last value is returned.";
const char doc_unless[] = "(unless test form*)\n"
"Evaluates the test. If it's nil the forms are evaluated and the last value is returned.";
const char doc_case[] = "(case keyform ((key form*) (key form*) ... ))\n"
"Evaluates a keyform to produce a test key, and then tests this against a series of arguments,\n"
"each of which is a list containing a key optionally followed by one or more forms.";
const char doc_and[] = "(and item*)\n"
"Evaluates its arguments until one returns nil, and returns the last value.";
const char doc_not[] = "(not item)\n"
"Returns t if its argument is nil, or nil otherwise. Equivalent to null.";
const char doc_nullfn[] = "(null list)\n"
"Returns t if its argument is nil, or nil otherwise. Equivalent to not.";
const char doc_cons[] = "(cons item item)\n"
"If the second argument is a list, cons returns a new list with item added to the front of the list.\n"
"If the second argument isn't a list cons returns a dotted pair.";
const char doc_atom[] = "(atom item)\n"
"Returns t if its argument is a single number, symbol, or nil.";
const char doc_listp[] = "(listp item)\n"
"Returns t if its argument is a list.";
const char doc_consp[] = "(consp item)\n"
"Returns t if its argument is a non-null list.";
const char doc_symbolp[] = "(symbolp item)\n"
"Returns t if its argument is a symbol.";
const char doc_arrayp[] = "(arrayp item)\n"
"Returns t if its argument is an array.";
const char doc_boundp[] = "(boundp item)\n"
"Returns t if its argument is a symbol with a value.";
const char doc_keywordp[] = "(keywordp item)\n"
"Returns non-nil if its argument is a built-in or user-defined keyword.";
const char doc_setfn[] = "(set symbol value [symbol value]*)\n"
"For each pair of arguments, assigns the value of the second argument to the value of the first argument.";
const char doc_streamp[] = "(streamp item)\n"
"Returns t if its argument is a stream.";
const char doc_equal[] = "(equal item item)\n"
"Tests whether the two arguments are the same symbol, same character, equal numbers,\n"
"or point to the same cons, and returns t or nil as appropriate.";
const char doc_caar[] = "(caar list)";
const char doc_cadr[] = "(cadr list)";
const char doc_second[] = "(second list)\n"
"Returns the second item in a list. Equivalent to cadr.";
const char doc_cdar[] = "(cdar list)\n"
"Equivalent to (cdr (car list)).";
const char doc_cddr[] = "(cddr list)\n"
"Equivalent to (cdr (cdr list)).";
const char doc_caaar[] = "(caaar list)\n"
"Equivalent to (car (car (car list))).";
const char doc_caadr[] = "(caadr list)\n"
"Equivalent to (car (car (cdar list))).";
const char doc_cadar[] = "(cadar list)\n"
"Equivalent to (car (cdr (car list))).";
const char doc_caddr[] = "(caddr list)\n"
"Equivalent to (car (cdr (cdr list))).";
const char doc_third[] = "(third list)\n"
"Returns the third item in a list. Equivalent to caddr.";
const char doc_cdaar[] = "(cdaar list)\n"
"Equivalent to (cdar (car (car list))).";
const char doc_cdadr[] = "(cdadr list)\n"
"Equivalent to (cdr (car (cdr list))).";
const char doc_cddar[] = "(cddar list)\n"
"Equivalent to (cdr (cdr (car list))).";
const char doc_cdddr[] = "(cdddr list)\n"
"Equivalent to (cdr (cdr (cdr list))).";
const char doc_length[] = "(length item)\n"
"Returns the number of items in a list, the length of a string, or the length of a one-dimensional array.";
const char doc_arraydimensions[] = "(array-dimensions item)\n"
"Returns a list of the dimensions of an array.";
const char doc_list[] = "(list item*)\n"
"Returns a list of the values of its arguments.";
const char doc_copylist[] = "(copy-list list)\n"
"Returns a copy of a list.";
const char doc_makearray[] = "(make-array size [:initial-element element] [:element-type 'bit])\n"
"If size is an integer it creates a one-dimensional array with elements from 0 to size-1.\n"
"If size is a list of n integers it creates an n-dimensional array with those dimensions.\n"
"If :element-type 'bit is specified the array is a bit array.";
const char doc_reverse[] = "(reverse list)\n"
"Returns a list with the elements of list in reverse order.";
const char doc_assoc[] = "(assoc key list [:test function])\n"
"Looks up a key in an association list of (key . value) pairs, using eq or the specified test function,\n"
"and returns the matching pair, or nil if no pair is found.";
const char doc_member[] = "(member item list [:test function])\n"
"Searches for an item in a list, using eq or the specified test function, and returns the list starting\n"
"from the first occurrence of the item, or nil if it is not found.";
const char doc_apply[] = "(apply function list)\n"
"Returns the result of evaluating function, with the list of arguments specified by the second parameter.";
const char doc_funcall[] = "(funcall function argument*)\n"
"Evaluates function with the specified arguments.";
const char doc_append[] = "(append list*)\n"
"Joins its arguments, which should be lists, into a single list.";
const char doc_mapc[] = "(mapc function list1 [list]*)\n"
"Applies the function to each element in one or more lists, ignoring the results.\n"
"It returns the first list argument.";
const char doc_mapl[] = "(mapl function list1 [list]*)\n"
"Applies the function to one or more lists and then successive cdrs of those lists,\n"
"ignoring the results. It returns the first list argument.";
const char doc_mapcar[] = "(mapcar function list1 [list]*)\n"
"Applies the function to each element in one or more lists, and returns the resulting list.";
const char doc_mapcan[] = "(mapcan function list1 [list]*)\n"
"Applies the function to each element in one or more lists. The results should be lists,\n"
"and these are destructively concatenated together to give the value returned.";
const char doc_maplist[] = "(maplist function list1 [list]*)\n"
"Applies the function to one or more lists and then successive cdrs of those lists,\n"
"and returns the resulting list.";
const char doc_mapcon[] = "(mapcon function list1 [list]*)\n"
"Applies the function to one or more lists and then successive cdrs of those lists,\n"
"and these are destructively concatenated together to give the value returned.";
const char doc_add[] = "(+ number*)\n"
"Adds its arguments together.\n"
"If each argument is an integer, and the running total doesn't overflow, the result is an integer,\n"
"otherwise a floating-point number.";
const char doc_subtract[] = "(- number*)\n"
"If there is one argument, negates the argument.\n"
"If there are two or more arguments, subtracts the second and subsequent arguments from the first argument.\n"
"If each argument is an integer, and the running total doesn't overflow, returns the result as an integer,\n"
"otherwise a floating-point number.";
const char doc_multiply[] = "(* number*)\n"
"Multiplies its arguments together.\n"
"If each argument is an integer, and the running total doesn't overflow, the result is an integer,\n"
"otherwise it's a floating-point number.";
const char doc_divide[] = "(/ number*)\n"
"Divides the first argument by the second and subsequent arguments.\n"
"If each argument is an integer, and each division produces an exact result, the result is an integer;\n"
"otherwise it's a floating-point number.";
const char doc_mod[] = "(mod number number)\n"
"Returns its first argument modulo the second argument.\n"
"If both arguments are integers the result is an integer; otherwise it's a floating-point number.";
const char doc_rem[] = "(rem number number)\n"
"Returns the remainder from dividing the first argument by the second argument.\n"
"If both arguments are integers the result is an integer; otherwise it's a floating-point number.";
const char doc_oneplus[] = "(1+ number)\n"
"Adds one to its argument and returns it.\n"
"If the argument is an integer the result is an integer if possible;\n"
"otherwise it's a floating-point number.";
const char doc_oneminus[] = "(1- number)\n"
"Subtracts one from its argument and returns it.\n"
"If the argument is an integer the result is an integer if possible;\n"
"otherwise it's a floating-point number.";
const char doc_abs[] = "(abs number)\n"
"Returns the absolute, positive value of its argument.\n"
"If the argument is an integer the result will be returned as an integer if possible,\n"
"otherwise a floating-point number.";
const char doc_random[] = "(random number)\n"
"If number is an integer returns a random number between 0 and one less than its argument.\n"
"Otherwise returns a floating-point number between zero and number.";
const char doc_maxfn[] = "(max number*)\n"
"Returns the maximum of one or more arguments.";
const char doc_minfn[] = "(min number*)\n"
"Returns the minimum of one or more arguments.";
const char doc_noteq[] = "(/= number*)\n"
"Returns t if none of the arguments are equal, or nil if two or more arguments are equal.";
const char doc_numeq[] = "(= number*)\n"
"Returns t if all the arguments, which must be numbers, are numerically equal, and nil otherwise.";
const char doc_less[] = "(< number*)\n"
"Returns t if each argument is less than the next argument, and nil otherwise.";
const char doc_lesseq[] = "(<= number*)\n"
"Returns t if each argument is less than or equal to the next argument, and nil otherwise.";
const char doc_greater[] = "(> number*)\n"
"Returns t if each argument is greater than the next argument, and nil otherwise.";
const char doc_greatereq[] = "(>= number*)\n"
"Returns t if each argument is greater than or equal to the next argument, and nil otherwise.";
const char doc_plusp[] = "(plusp number)\n"
"Returns t if the argument is greater than zero, or nil otherwise.";
const char doc_minusp[] = "(minusp number)\n"
"Returns t if the argument is less than zero, or nil otherwise.";
const char doc_zerop[] = "(zerop number)\n"
"Returns t if the argument is zero.";
const char doc_oddp[] = "(oddp number)\n"
"Returns t if the integer argument is odd.";
const char doc_evenp[] = "(evenp number)\n"
"Returns t if the integer argument is even.";
const char doc_integerp[] = "(integerp number)\n"
"Returns t if the argument is an integer.";
const char doc_numberp[] = "(numberp number)\n"
"Returns t if the argument is a number.";
const char doc_floatfn[] = "(float number)\n"
"Returns its argument converted to a floating-point number.";
const char doc_floatp[] = "(floatp number)\n"
"Returns t if the argument is a floating-point number.";
const char doc_sin[] = "(sin number)\n"
"Returns sin(number).";
const char doc_cos[] = "(cos number)\n"
"Returns cos(number).";
const char doc_tan[] = "(tan number)\n"
"Returns tan(number).";
const char doc_asin[] = "(asin number)\n"
"Returns asin(number).";
const char doc_acos[] = "(acos number)\n"
"Returns acos(number).";
const char doc_atan[] = "(atan number1 [number2])\n"
"Returns the arc tangent of number1/number2, in radians. If number2 is omitted it defaults to 1.";
const char doc_sinh[] = "(sinh number)\n"
"Returns sinh(number).";
const char doc_cosh[] = "(cosh number)\n"
"Returns cosh(number).";
const char doc_tanh[] = "(tanh number)\n"
"Returns tanh(number).";
const char doc_exp[] = "(exp number)\n"
"Returns exp(number).";
const char doc_sqrt[] = "(sqrt number)\n"
"Returns sqrt(number).";
const char doc_log[] = "(log number [base])\n"
"Returns the logarithm of number to the specified base. If base is omitted it defaults to e.";
const char doc_expt[] = "(expt number power)\n"
"Returns number raised to the specified power.\n"
"Returns the result as an integer if the arguments are integers and the result will be within range,\n"
"otherwise a floating-point number.";
const char doc_ceiling[] = "(ceiling number [divisor])\n"
"Returns the integer closest to +infinity for number/divisor. If divisor is omitted it defaults to 1.";
const char doc_floor[] = "(floor number [divisor])\n"
"Returns the integer closest to -infinity for number/divisor. If divisor is omitted it defaults to 1.";
const char doc_truncate[] = "(truncate number [divisor])\n"
"Returns the integer part of number/divisor. If divisor is omitted it defaults to 1.";
const char doc_round[] = "(round number [divisor])\n"
"Returns the integer closest to number/divisor. If divisor is omitted it defaults to 1.";
const char doc_charcode[] = "(char-code character)\n"
"Returns the ASCII code for a character, as an integer.";
const char doc_codechar[] = "(code-char integer)\n"
"Returns the character for the specified ASCII code.";
const char doc_characterp[] = "(characterp item)\n"
"Returns t if the argument is a character and nil otherwise.";
const char doc_stringp[] = "(stringp item)\n"
"Returns t if the argument is a string and nil otherwise.";
const char doc_stringeq[] = "(string= string string)\n"
"Returns t if the two strings are the same, or nil otherwise.";
const char doc_stringless[] = "(string< string string)\n"
"Returns the index to the first mismatch if the first string is alphabetically less than the second string,\n"
"or nil otherwise.";
const char doc_stringgreater[] = "(string> string string)\n"
"Returns the index to the first mismatch if the first string is alphabetically greater than the second string,\n"
"or nil otherwise.";
const char doc_stringnoteq[] = "(string/= string string)\n"
"Returns the index to the first mismatch if the two strings are not the same, or nil otherwise.";
const char doc_stringlesseq[] = "(string<= string string)\n"
"Returns the index to the first mismatch if the first string is alphabetically less than or equal to\n"
"the second string, or nil otherwise.";
const char doc_stringgreatereq[] = "(string>= string string)\n"
"Returns the index to the first mismatch if the first string is alphabetically greater than or equal to\n"
"the second string, or nil otherwise.";
const char doc_sort[] = "(sort list test)\n"
"Destructively sorts list according to the test function, using an insertion sort, and returns the sorted list.";
const char doc_concatenate[] = "(concatenate 'string string*)\n"
"Joins together the strings given in the second and subsequent arguments, and returns a single string.";
const char doc_subseq[] = "(subseq seq start [end])\n"
"Returns a subsequence of a list or string from item start to item end-1.";
const char doc_search[] = "(search pattern target [:test function])\n"
"Returns the index of the first occurrence of pattern in target, or nil if it's not found.\n"
"The target can be a list or string. If it's a list a test function can be specified; default eq.";
const char doc_readfromstring[] = "(read-from-string string)\n"
"Reads an atom or list from the specified string and returns it.";
const char doc_princtostring[] = "(princ-to-string item)\n"
"Prints its argument to a string, and returns the string.\n"
"Characters and strings are printed without quotation marks or escape characters.";
const char doc_prin1tostring[] = "(prin1-to-string item [stream])\n"
"Prints its argument to a string, and returns the string.\n"
"Characters and strings are printed with quotation marks and escape characters,\n"
"in a format that will be suitable for read-from-string.";
const char doc_logand[] = "(logand [value*])\n"
"Returns the bitwise & of the values.";
const char doc_logior[] = "(logior [value*])\n"
"Returns the bitwise | of the values.";
const char doc_logxor[] = "(logxor [value*])\n"
"Returns the bitwise ^ of the values.";
const char doc_lognot[] = "(lognot value)\n"
"Returns the bitwise logical NOT of the value.";
const char doc_ash[] = "(ash value shift)\n"
"Returns the result of bitwise shifting value by shift bits. If shift is positive, value is shifted to the left.";
const char doc_logbitp[] = "(logbitp bit value)\n"
"Returns t if bit number bit in value is a '1', and nil if it is a '0'.";
const char doc_eval[] = "(eval form*)\n"
"Evaluates its argument an extra time.";
const char doc_return[] = "(return [value])\n"
"Exits from a (dotimes ...), (dolist ...), or (loop ...) loop construct and returns value.";
const char doc_globals[] = "(globals)\n"
"Returns a list of global variables.";
const char doc_locals[] = "(locals)\n"
"Returns an association list of local variables and their values.";
const char doc_makunbound[] = "(makunbound symbol)\n"
"Removes the value of the symbol from GlobalEnv and returns the symbol.";
const char doc_break[] = "(break)\n"
"Inserts a breakpoint in the program. When evaluated prints Break! and reenters the REPL.";
const char doc_read[] = "(read [stream])\n"
"Reads an atom or list from the serial input and returns it.\n"
"If stream is specified the item is read from the specified stream.";
const char doc_prin1[] = "(prin1 item [stream])\n"
"Prints its argument, and returns its value.\n"
"Strings are printed with quotation marks and escape characters.";
const char doc_print[] = "(print item [stream])\n"
"Prints its argument with quotation marks and escape characters, on a new line, and followed by a space.\n"
"If stream is specified the argument is printed to the specified stream.";
const char doc_princ[] = "(princ item [stream])\n"
"Prints its argument, and returns its value.\n"
"Characters and strings are printed without quotation marks or escape characters.";
const char doc_terpri[] = "(terpri [stream])\n"
"Prints a new line, and returns nil.\n"
"If stream is specified the new line is written to the specified stream.";
const char doc_readbyte[] = "(read-byte stream)\n"
"Reads a byte from a stream and returns it.";
const char doc_readline[] = "(read-line [stream])\n"
"Reads characters from the serial input up to a newline character, and returns them as a string,\n"
"excluding the newline. If stream is specified the line is read from the specified stream.";
const char doc_writebyte[] = "(write-byte number [stream])\n"
"Writes a byte to a stream.";
const char doc_writestring[] = "(write-string string [stream])\n"
"Writes a string. If stream is specified the string is written to the stream.";
const char doc_writeline[] = "(write-line string [stream])\n"
"Writes a string terminated by a newline character. If stream is specified the string is written to the stream.";
const char doc_restarti2c[] = "(restart-i2c stream [read-p])\n"
"Restarts an i2c-stream.\n"
"If read-p is nil or omitted the stream is written to.\n"
"If read-p is an integer it specifies the number of bytes to be read from the stream.";
const char doc_gc[] = "(gc [print time])\n"
"Forces a garbage collection and prints the number of objects collected, and the time taken.";
const char doc_room[] = "(room)\n"
"Returns the number of free Lisp cells remaining.";
const char doc_backtrace[] = "(backtrace [on])\n"
"Sets the state of backtrace according to the boolean flag 'on',\n"
"or with no argument displays the current state of backtrace.";
const char doc_saveimage[] = "(save-image [symbol])\n"
"Saves the current uLisp image to non-volatile memory or SD card so it can be loaded using load-image.";
const char doc_loadimage[] = "(load-image [filename])\n"
"Loads a saved uLisp image from non-volatile memory or SD card.";
const char doc_cls[] = "(cls)\n"
"Prints a clear-screen character.";
const char doc_digitalread[] = "(digitalread pin)\n"
"Reads the state of the specified Arduino pin number and returns t (high) or nil (low).";
const char doc_analogreadresolution[] = "(analogreadresolution bits)\n"
"Specifies the resolution for the analogue inputs on platforms that support it.\n"
"The default resolution on all platforms is 10 bits.";
const char doc_analogwrite[] = "(analogwrite pin value)\n"
"Writes the value to the specified Arduino pin number.";
const char doc_delay[] = "(delay number)\n"
"Delays for a specified number of milliseconds.";
const char doc_millis[] = "(millis)\n"
"Returns the time in milliseconds that uLisp has been running.";
const char doc_sleep[] = "(sleep secs)\n"
"Puts the processor into a low-power sleep mode for secs.\n"
"Only supported on some platforms. On other platforms it does delay(1000*secs).";
const char doc_note[] = "(note [pin] [note] [octave])\n"
"Generates a square wave on pin.\n"
"note represents the note in the well-tempered scale.\n"
"The argument octave can specify an octave; default 0.";
const char doc_edit[] = "(edit 'function)\n"
"Calls the Lisp tree editor to allow you to edit a function definition.";
const char doc_pprint[] = "(pprint item [str])\n"
"Prints its argument, using the pretty printer, to display it formatted in a structured way.\n"
"If str is specified it prints to the specified stream. It returns no value.";
const char doc_pprintall[] = "(pprintall [str])\n"
"Pretty-prints the definition of every function and variable defined in the uLisp workspace.\n"
"If str is specified it prints to the specified stream. It returns no value.";
const char doc_require[] = "(require 'symbol)\n"
"Loads the definition of a function defined with defun, or a variable defined with defvar, from the Lisp Library.\n"
"It returns t if it was loaded, or nil if the symbol is already defined or isn't defined in the Lisp Library.";
const char doc_listlibrary[] = "(list-library)\n"
"Prints a list of the functions defined in the List Library.";
const char doc_help[] = "(? item)\n"
"Prints the documentation string of a built-in or user-defined function.";
const char doc_documentation[] = "(documentation 'symbol [type])\n"
"Returns the documentation string of a built-in or user-defined function. The type argument is ignored.";
const char doc_apropos[] = "(apropos item)\n"
"Prints the user-defined and built-in functions whose names contain the specified string or symbol.";
const char doc_aproposlist[] = "(apropos-list item)\n"
"Returns a list of user-defined and built-in functions whose names contain the specified string or symbol.";
const char doc_unwindprotect[] = "(unwind-protect form1 [forms]*)\n"
"Evaluates form1 and forms in order and returns the value of form1,\n"
"but guarantees to evaluate forms even if an error occurs in form1.";
const char doc_ignoreerrors[] = "(ignore-errors [forms]*)\n"
"Evaluates forms ignoring errors.";
const char doc_error[] = "(error controlstring [arguments]*)\n"
"Signals an error. The message is printed by format using the controlstring and arguments.";
const char doc_directory[] = "(directory)\n"
"Returns a list of the filenames of the files on the SD card.";
const char doc_withclient[] = "(with-client (str [address port]) form*)\n"
"Evaluates the forms with str bound to a wifi-stream.";
const char doc_available[] = "(available stream)\n"
"Returns the number of bytes available for reading from the wifi-stream, or zero if no bytes are available.";
const char doc_wifiserver[] = "(wifi-server)\n"
"Starts a Wi-Fi server running. It returns nil.";
const char doc_wifisoftap[] = "(wifi-softap ssid [password channel hidden])\n"
"Set up a soft access point to establish a Wi-Fi network.\n"
"Returns the IP address as a string or nil if unsuccessful.";
const char doc_connected[] = "(connected stream)\n"
"Returns t or nil to indicate if the client on stream is connected.";
const char doc_wifilocalip[] = "(wifi-localip)\n"
"Returns the IP address of the local network as a string.";
const char doc_wificonnect[] = "(wifi-connect [ssid pass])\n"
"Connects to the Wi-Fi network ssid using password pass. It returns the IP address as a string.";
const char doc_gettime[] = "(get-time)\n"
"Returns a list of the current date and time, or nil if the time isn't available, as:\n"
"(year, month (1-12), day (1-31), hour (0-32), minute (0-59), second (0-60), weekday (1-7), yearday (1-366)).";
const char doc_withgfx[] = "(with-gfx (str) form*)\n"
"Evaluates the forms with str bound to an gfx-stream so you can print text\n"
"to the graphics display using the standard uLisp print commands.";
const char doc_drawpixel[] = "(draw-pixel x y [colour])\n"
"Draws a pixel at coordinates (x,y) in colour, or white if omitted.";
const char doc_drawline[] = "(draw-line x0 y0 x1 y1 [colour])\n"
"Draws a line from (x0,y0) to (x1,y1) in colour, or white if omitted.";
const char doc_drawrect[] = "(draw-rect x y w h [colour])\n"
"Draws an outline rectangle with its top left corner at (x,y), with width w,\n"
"and with height h. The outline is drawn in colour, or white if omitted.";
const char doc_fillrect[] = "(fill-rect x y w h [colour])\n"
"Draws a filled rectangle with its top left corner at (x,y), with width w,\n"
"and with height h. The outline is drawn in colour, or white if omitted.";
const char doc_drawcircle[] = "(draw-circle x y r [colour])\n"
"Draws an outline circle with its centre at (x, y) and with radius r.\n"
"The circle is drawn in colour, or white if omitted.";
const char doc_fillcircle[] = "(fill-circle x y r [colour])\n"
"Draws a filled circle with its centre at (x, y) and with radius r.\n"
"The circle is drawn in colour, or white if omitted.";
const char doc_drawroundrect[] = "(draw-round-rect x y w h radius [colour])\n"
"Draws an outline rounded rectangle with its top left corner at (x,y), with width w,\n"
"height h, and corner radius radius. The outline is drawn in colour, or white if omitted.";
const char doc_fillroundrect[] = "(fill-round-rect x y w h radius [colour])\n"
"Draws a filled rounded rectangle with its top left corner at (x,y), with width w,\n"
"height h, and corner radius radius. The outline is drawn in colour, or white if omitted.";
const char doc_drawtriangle[] = "(draw-triangle x0 y0 x1 y1 x2 y2 [colour])\n"
"Draws an outline triangle between (x1,y1), (x2,y2), and (x3,y3).\n"
"The outline is drawn in colour, or white if omitted.";
const char doc_filltriangle[] = "(fill-triangle x0 y0 x1 y1 x2 y2 [colour])\n"
"Draws a filled triangle between (x1,y1), (x2,y2), and (x3,y3).\n"
"The outline is drawn in colour, or white if omitted.";
const char doc_drawchar[] = "(draw-char x y char [colour background size])\n"
"Draws the character char with its top left corner at (x,y).\n"
"The character is drawn in a 5 x 7 pixel font in colour against background,\n"
"which default to white and black respectively.\n"
"The character can optionally be scaled by size.";
const char doc_setcursor[] = "(set-cursor x y)\n"
"Sets the start point for text plotting to (x, y).";
const char doc_settextcolor[] = "(set-text-color colour [background])\n"
"Sets the text colour for text plotted using (with-gfx ...).";
const char doc_settextsize[] = "(set-text-size scale)\n"
"Scales text by the specified size, default 1.";
const char doc_settextwrap[] = "(set-text-wrap boolean)\n"
"Specified whether text wraps at the right-hand edge of the display; the default is t.";
const char doc_fillscreen[] = "(fill-screen [colour])\n"
"Fills or clears the screen with colour, default black.";
const char doc_setrotation[] = "(set-rotation option)\n"
"Sets the display orientation for subsequent graphics commands; values are 0, 1, 2, or 3.";
const char doc_invertdisplay[] = "(invert-display boolean)\n"
"Mirror-images the display.";
const char doc_readpixel[] = "(read-pixel x y)\n"
"Returns the colour of the pixel at x,y.";
const char doc_touchscreen[] = "(touchscreen)\n"
"Returns a list of (x y) to give the touch position, or nil if no touch.";
const char doc_displaysize[] = "(display-size)\n"
"Returns a list of (width height) to give the display dimensions in the current orientation.";
const char doc_display[] = "(display)\n"
"Needed on OLED displays to update the display from the memory buffer.";

// Built-in symbol lookup table
const tbl_entry_t lookup_table[] = {
  { "nil", NULL, 0000, doc_nil },
  { "t", NULL, 0000, doc_tee },
  { "nothing", NULL, 0000, doc_nothing },
  { "&optional", NULL, 0000, doc_optional },
  { "*features*", NULL, 0000, doc_features },
  { ":initial-element", NULL, 0000, NULL },
  { ":element-type", NULL, 0000, NULL },
  { ":test", NULL, 0000, NULL },
  { ":a", NULL, 0000, NULL },
  { ":b", NULL, 0000, NULL },
  { ":c", NULL, 0000, NULL },
  { "bit", NULL, 0000, NULL },
  { "&rest", NULL, 0000, doc_amprest },
  { "lambda", NULL, 0017, doc_lambda },
  { "let", NULL, 0017, doc_let },
  { "let*", NULL, 0017, doc_letstar },
  { "closure", NULL, 0017, NULL },
  { "*pc*", NULL, 0007, NULL },
  { "quote", sp_quote, 0311, NULL },
  { "defun", sp_defun, 0327, doc_defun },
  { "defvar", sp_defvar, 0313, doc_defvar },
  { "bind", sp_bind, 0327, doc_bind },
  { "eq", fn_eq, 0222, doc_eq },
  { "car", fn_car, 0211, doc_car },
  { "first", fn_car, 0211, doc_first },
  { "cdr", fn_cdr, 0211, doc_cdr },
  { "rest", fn_cdr, 0211, doc_rest },
  { "nth", fn_nth, 0222, doc_nth },
  { "aref", fn_aref, 0227, doc_aref },
  { "char", fn_char, 0222, doc_char },
  { "string", fn_stringfn, 0211, doc_stringfn },
  { "pinmode", fn_pinmode, 0222, doc_pinmode },
  { "digitalwrite", fn_digitalwrite, 0222, doc_digitalwrite },
  { "analogread", fn_analogread, 0211, doc_analogread },
  { "register", fn_register, 0212, doc_register },
  { "format", fn_format, 0227, doc_format },
  { "or", sp_or, 0307, doc_or },
  { "setq", sp_setq, 0327, doc_setq },
  { "loop", sp_loop, 0307, doc_loop },
  { "push", sp_push, 0322, doc_push },
  { "pop", sp_pop, 0311, doc_pop },
  { "incf", sp_incf, 0312, doc_incf },
  { "decf", sp_decf, 0312, doc_decf },
  { "setf", sp_setf, 0327, doc_setf },
  { "dolist", sp_dolist, 0317, doc_dolist },
  { "dotimes", sp_dotimes, 0317, doc_dotimes },
  { "do", sp_do, 0327, doc_do },
  { "do*", sp_dostar, 0317, doc_dostar },
  { "trace", sp_trace, 0301, doc_trace },
  { "untrace", sp_untrace, 0301, doc_untrace },
  { "for-millis", sp_formillis, 0317, doc_formillis },
  { "time", sp_time, 0311, doc_time },
  { "with-output-to-string", sp_withoutputtostring, 0317, doc_withoutputtostring },
  { "with-serial", sp_withserial, 0317, doc_withserial },
  { "with-i2c", sp_withi2c, 0317, doc_withi2c },
  { "with-spi", sp_withspi, 0317, doc_withspi },
  { "with-sd-card", sp_withsdcard, 0327, doc_withsdcard },
  { "progn", tf_progn, 0107, doc_progn },
  { "if", tf_if, 0123, doc_if },
  { "cond", tf_cond, 0107, doc_cond },
  { "when", tf_when, 0117, doc_when },
  { "unless", tf_unless, 0117, doc_unless },
  { "case", tf_case, 0117, doc_case },
  { "and", tf_and, 0107, doc_and },
  { "not", fn_not, 0211, doc_not },
  { "null", fn_not, 0211, doc_nullfn },
  { "cons", fn_cons, 0222, doc_cons },
  { "atom", fn_atom, 0211, doc_atom },
  { "listp", fn_listp, 0211, doc_listp },
  { "consp", fn_consp, 0211, doc_consp },
  { "symbolp", fn_symbolp, 0211, doc_symbolp },
  { "arrayp", fn_arrayp, 0211, doc_arrayp },
  { "boundp", fn_boundp, 0211, doc_boundp },
  { "keywordp", fn_keywordp, 0211, doc_keywordp },
  { "set", fn_setfn, 0227, doc_setfn },
  { "streamp", fn_streamp, 0211, doc_streamp },
  { "equal", fn_equal, 0222, doc_equal },
  { "caar", fn_caar, 0211, doc_caar },
  { "cadr", fn_cadr, 0211, doc_cadr },
  { "second", fn_cadr, 0211, doc_second },
  { "cdar", fn_cdar, 0211, doc_cdar },
  { "cddr", fn_cddr, 0211, doc_cddr },
  { "caaar", fn_caaar, 0211, doc_caaar },
  { "caadr", fn_caadr, 0211, doc_caadr },
  { "cadar", fn_cadar, 0211, doc_cadar },
  { "caddr", fn_caddr, 0211, doc_caddr },
  { "third", fn_caddr, 0211, doc_third },
  { "cdaar", fn_cdaar, 0211, doc_cdaar },
  { "cdadr", fn_cdadr, 0211, doc_cdadr },
  { "cddar", fn_cddar, 0211, doc_cddar },
  { "cdddr", fn_cdddr, 0211, doc_cdddr },
  { "length", fn_length, 0211, doc_length },
  { "array-dimensions", fn_arraydimensions, 0211, doc_arraydimensions },
  { "list", fn_list, 0207, doc_list },
  { "copy-list", fn_copylist, 0211, doc_copylist },
  { "make-array", fn_makearray, 0215, doc_makearray },
  { "reverse", fn_reverse, 0211, doc_reverse },
  { "assoc", fn_assoc, 0224, doc_assoc },
  { "member", fn_member, 0224, doc_member },
  { "apply", fn_apply, 0227, doc_apply },
  { "funcall", fn_funcall, 0217, doc_funcall },
  { "append", fn_append, 0207, doc_append },
  { "mapc", fn_mapc, 0227, doc_mapc },
  { "mapl", fn_mapl, 0227, doc_mapl },
  { "mapcar", fn_mapcar, 0227, doc_mapcar },
  { "mapcan", fn_mapcan, 0227, doc_mapcan },
  { "maplist", fn_maplist, 0227, doc_maplist },
  { "mapcon", fn_mapcon, 0227, doc_mapcon },
  { "+", fn_add, 0207, doc_add },
  { "-", fn_subtract, 0217, doc_subtract },
  { "*", fn_multiply, 0207, doc_multiply },
  { "/", fn_divide, 0217, doc_divide },
  { "mod", fn_mod, 0222, doc_mod },
  { "rem", fn_rem, 0222, doc_rem },
  { "1+", fn_oneplus, 0211, doc_oneplus },
  { "1-", fn_oneminus, 0211, doc_oneminus },
  { "abs", fn_abs, 0211, doc_abs },
  { "random", fn_random, 0211, doc_random },
  { "max", fn_maxfn, 0217, doc_maxfn },
  { "min", fn_minfn, 0217, doc_minfn },
  { "/=", fn_noteq, 0217, doc_noteq },
  { "=", fn_numeq, 0217, doc_numeq },
  { "<", fn_less, 0217, doc_less },
  { "<=", fn_lesseq, 0217, doc_lesseq },
  { ">", fn_greater, 0217, doc_greater },
  { ">=", fn_greatereq, 0217, doc_greatereq },
  { "plusp", fn_plusp, 0211, doc_plusp },
  { "minusp", fn_minusp, 0211, doc_minusp },
  { "zerop", fn_zerop, 0211, doc_zerop },
  { "oddp", fn_oddp, 0211, doc_oddp },
  { "evenp", fn_evenp, 0211, doc_evenp },
  { "integerp", fn_integerp, 0211, doc_integerp },
  { "numberp", fn_numberp, 0211, doc_numberp },
  { "float", fn_floatfn, 0211, doc_floatfn },
  { "floatp", fn_floatp, 0211, doc_floatp },
  { "sin", fn_sin, 0211, doc_sin },
  { "cos", fn_cos, 0211, doc_cos },
  { "tan", fn_tan, 0211, doc_tan },
  { "asin", fn_asin, 0211, doc_asin },
  { "acos", fn_acos, 0211, doc_acos },
  { "atan", fn_atan, 0212, doc_atan },
  { "sinh", fn_sinh, 0211, doc_sinh },
  { "cosh", fn_cosh, 0211, doc_cosh },
  { "tanh", fn_tanh, 0211, doc_tanh },
  { "exp", fn_exp, 0211, doc_exp },
  { "sqrt", fn_sqrt, 0211, doc_sqrt },
  { "log", fn_log, 0212, doc_log },
  { "expt", fn_expt, 0222, doc_expt },
  { "ceiling", fn_ceiling, 0212, doc_ceiling },
  { "floor", fn_floor, 0212, doc_floor },
  { "truncate", fn_truncate, 0212, doc_truncate },
  { "round", fn_round, 0212, doc_round },
  { "char-code", fn_charcode, 0211, doc_charcode },
  { "code-char", fn_codechar, 0211, doc_codechar },
  { "characterp", fn_characterp, 0211, doc_characterp },
  { "stringp", fn_stringp, 0211, doc_stringp },
  { "string=", fn_stringeq, 0222, doc_stringeq },
  { "string<", fn_stringless, 0222, doc_stringless },
  { "string>", fn_stringgreater, 0222, doc_stringgreater },
  { "string/=", fn_stringnoteq, 0222, doc_stringnoteq },
  { "string<=", fn_stringlesseq, 0222, doc_stringlesseq },
  { "string>=", fn_stringgreatereq, 0222, doc_stringgreatereq },
  { "sort", fn_sort, 0222, doc_sort },
  { "concatenate", fn_concatenate, 0217, doc_concatenate },
  { "subseq", fn_subseq, 0223, doc_subseq },
  { "search", fn_search, 0224, doc_search },
  { "read-from-string", fn_readfromstring, 0211, doc_readfromstring },
  { "princ-to-string", fn_princtostring, 0211, doc_princtostring },
  { "prin1-to-string", fn_prin1tostring, 0211, doc_prin1tostring },
  { "logand", fn_logand, 0207, doc_logand },
  { "logior", fn_logior, 0207, doc_logior },
  { "logxor", fn_logxor, 0207, doc_logxor },
  { "lognot", fn_lognot, 0211, doc_lognot },
  { "ash", fn_ash, 0222, doc_ash },
  { "logbitp", fn_logbitp, 0222, doc_logbitp },
  { "eval", fn_eval, 0211, doc_eval },
  { "return", fn_return, 0201, doc_return },
  { "globals", fn_globals, 0200, doc_globals },
  { "locals", fn_locals, 0200, doc_locals },
  { "makunbound", fn_makunbound, 0211, doc_makunbound },
  { "break", fn_break, 0200, doc_break },
  { "read", fn_read, 0201, doc_read },
  { "prin1", fn_prin1, 0212, doc_prin1 },
  { "print", fn_print, 0212, doc_print },
  { "princ", fn_princ, 0212, doc_princ },
  { "terpri", fn_terpri, 0201, doc_terpri },
  { "read-byte", fn_readbyte, 0202, doc_readbyte },
  { "read-line", fn_readline, 0201, doc_readline },
  { "write-byte", fn_writebyte, 0212, doc_writebyte },
  { "write-string", fn_writestring, 0212, doc_writestring },
  { "write-line", fn_writeline, 0212, doc_writeline },
  { "restart-i2c", fn_restarti2c, 0212, doc_restarti2c },
  { "gc", fn_gc, 0201, doc_gc },
  { "room", fn_room, 0200, doc_room },
  { "backtrace", fn_backtrace, 0201, doc_backtrace },
  { "save-image", fn_saveimage, 0201, doc_saveimage },
  { "load-image", fn_loadimage, 0201, doc_loadimage },
  { "cls", fn_cls, 0200, doc_cls },
  { "digitalread", fn_digitalread, 0211, doc_digitalread },
  { "analogreadresolution", fn_analogreadresolution, 0211, doc_analogreadresolution },
  { "analogwrite", fn_analogwrite, 0222, doc_analogwrite },
  { "delay", fn_delay, 0211, doc_delay },
  { "millis", fn_millis, 0200, doc_millis },
  { "sleep", fn_sleep, 0201, doc_sleep },
  { "note", fn_note, 0203, doc_note },
  { "edit", fn_edit, 0211, doc_edit },
  { "pprint", fn_pprint, 0212, doc_pprint },
  { "pprintall", fn_pprintall, 0201, doc_pprintall },
  { "require", fn_require, 0211, doc_require },
  { "list-library", fn_listlibrary, 0200, doc_listlibrary },
  { "?", sp_help, 0311, doc_help },
  { "documentation", fn_documentation, 0212, doc_documentation },
  { "apropos", fn_apropos, 0211, doc_apropos },
  { "apropos-list", fn_aproposlist, 0211, doc_aproposlist },
  { "unwind-protect", sp_unwindprotect, 0307, doc_unwindprotect },
  { "ignore-errors", sp_ignoreerrors, 0307, doc_ignoreerrors },
  { "error", sp_error, 0317, doc_error },
  { "directory", fn_directory, 0200, doc_directory },
  { "with-client", sp_withclient, 0317, doc_withclient },
  { "available", fn_available, 0211, doc_available },
  { "wifi-server", fn_wifiserver, 0200, doc_wifiserver },
  { "wifi-softap", fn_wifisoftap, 0204, doc_wifisoftap },
  { "connected", fn_connected, 0211, doc_connected },
  { "wifi-localip", fn_wifilocalip, 0200, doc_wifilocalip },
  { "wifi-connect", fn_wificonnect, 0203, doc_wificonnect },
  { "get-time", fn_gettime, 0200, doc_gettime },
  { "with-gfx", sp_withgfx, 0317, doc_withgfx },
  { "draw-pixel", fn_drawpixel, 0223, doc_drawpixel },
  { "draw-line", fn_drawline, 0245, doc_drawline },
  { "draw-rect", fn_drawrect, 0245, doc_drawrect },
  { "fill-rect", fn_fillrect, 0245, doc_fillrect },
  { "draw-circle", fn_drawcircle, 0234, doc_drawcircle },
  { "fill-circle", fn_fillcircle, 0234, doc_fillcircle },
  { "draw-round-rect", fn_drawroundrect, 0256, doc_drawroundrect },
  { "fill-round-rect", fn_fillroundrect, 0256, doc_fillroundrect },
  { "draw-triangle", fn_drawtriangle, 0267, doc_drawtriangle },
  { "fill-triangle", fn_filltriangle, 0267, doc_filltriangle },
  { "draw-char", fn_drawchar, 0236, doc_drawchar },
  { "set-cursor", fn_setcursor, 0222, doc_setcursor },
  { "set-text-color", fn_settextcolor, 0212, doc_settextcolor },
  { "set-text-size", fn_settextsize, 0211, doc_settextsize },
  { "set-text-wrap", fn_settextwrap, 0211, doc_settextwrap },
  { "fill-screen", fn_fillscreen, 0201, doc_fillscreen },
  { "set-rotation", fn_setrotation, 0211, doc_setrotation },
  { "invert-display", fn_invertdisplay, 0211, doc_invertdisplay },
  { "read-pixel", fn_readpixel, 0222, doc_readpixel },
  { "touchscreen", fn_touchscreen, 0201, doc_touchscreen },
  { "display-size", fn_displaysize, 0200, doc_displaysize },
  { "display", fn_display, 0200, doc_display },
  { ":led-builtin", (fn_ptr_type)LED_BUILTIN, 0, NULL },
  { ":high", (fn_ptr_type)HIGH, DIGITALWRITE, NULL },
  { ":low", (fn_ptr_type)LOW, DIGITALWRITE, NULL },
  { ":input", (fn_ptr_type)INPUT, PINMODE, NULL },
  { ":input-pullup", (fn_ptr_type)INPUT_PULLUP, PINMODE, NULL },
  { ":input-pulldown", (fn_ptr_type)INPUT_PULLDOWN, PINMODE, NULL },
  { ":output", (fn_ptr_type)OUTPUT, PINMODE, NULL },
};

#if !defined(extensions)
// Table cross-reference functions

tbl_entry_t *tables[] = {lookup_table, NULL};
const unsigned int tablesizes[] = { arraysize(lookup_table), 0 };

const tbl_entry_t *table (int n) {
  return tables[n];
}

unsigned int tablesize (int n) {
  return tablesizes[n];
}
#endif

// Table lookup functions

builtin_t lookupbuiltin (char* c) {
  unsigned int start = tablesize(0);
  for (int n=1; n>=0; n--) {
    int entries = tablesize(n);
    for (int i=0; i<entries; i++) {
      if (strcasecmp(c, (char*)(table(n)[i].string)) == 0)
        return (builtin_t)(start + i);
    }
    start = 0;
  }
  return ENDFUNCTIONS;
}

intptr_t lookupfn (builtin_t name) {
  bool n = name<tablesize(0);
  return (intptr_t)table(n?0:1)[n?name:name-tablesize(0)].fptr;
}

uint8_t getminmax (builtin_t name) {
  bool n = name<tablesize(0);
  return table(n?0:1)[n?name:name-tablesize(0)].minmax;
}

void checkminmax (builtin_t name, int nargs) {
  if (!(name < ENDFUNCTIONS)) error2("not a builtin");
  volatile uint8_t minmax = getminmax(name); // Fix bug on Wio Terminal
  if (nargs<((minmax >> 3) & 0x07)) error2(toofewargs);
  if ((minmax & 0x07) != 0x07 && nargs>(minmax & 0x07)) error2(toomanyargs);
}

char *lookupdoc (builtin_t name) {
  bool n = name<tablesize(0);
  return (char*)table(n?0:1)[n?name:name-tablesize(0)].doc;
}

bool findsubstring (char *part, builtin_t name) {
  bool n = name<tablesize(0);
  return (strstr(table(n?0:1)[n?name:name-tablesize(0)].string, part) != NULL);
}

void testescape () {
  static unsigned long n;
  if (millis()-n < 500) return;
  n = millis();
  if (Serial.available() && Serial.read() == '~') error2("escape!");
}

bool colonp (symbol_t name) {
  if (!longnamep(name)) return false;
  object *form = (object *)name;
  if (form == NULL) return false;
  return (((form->chars)>>((sizeof(int)-1)*8) & 0xFF) == ':');
}

bool keywordp (object *obj) {
  if (!(symbolp(obj) && builtinp(obj->name))) return false;
  builtin_t name = builtin(obj->name);
  bool n = name<tablesize(0);
  const char *s = table(n?0:1)[n?name:name-tablesize(0)].string;
  char c = s[0];
  return (c == ':');
}

void backtrace (symbol_t name) {
  Backtrace[TraceTop] = (name == sym(NIL)) ? sym(LAMBDA) : name;
  TraceTop = modbacktrace(TraceTop+1);
  if (TraceStart == TraceTop) TraceStart = modbacktrace(TraceStart+1);
}

// Main evaluator

object *eval (object *form, object *env) {
  bool stackpos;
  int TC=0;
  EVAL:
  // Enough space?
  // Serial.println((uint32_t)StackBottom - (uint32_t)&stackpos); // Find best MAX_STACK value
  if ((uint32_t)StackBottom - (uint32_t)&stackpos > MAX_STACK) { Context = NIL; error2("stack overflow"); }
  if (Freespace <= WORKSPACESIZE>>4) gc(form, env);
  // Escape
  if (tstflag(ESCAPE)) { clrflag(ESCAPE); error2("escape!");}
  if (!tstflag(NOESC)) testescape();

  if (form == NULL) return nil;

  if (form->type >= NUMBER && form->type <= STRING) return form;

  if (symbolp(form)) {
    symbol_t name = form->name;
    if (colonp(name)) return form; // Keyword
    object *pair = value(name, env);
    if (pair != NULL) return cdr(pair);
    pair = value(name, GlobalEnv);
    if (pair != NULL) return cdr(pair);
    else if (builtinp(name)) {
      if (name == sym(FEATURES)) return features();
      return form;
    }
    Context = NIL;
    error("undefined", form);
  }

  // It's a list
  object *function = car(form);
  object *args = cdr(form);

  if (function == NULL) error(illegalfn, function);
  if (!listp(args)) error("can't evaluate a dotted pair", args);

  // List starts with a builtin symbol?
  if (symbolp(function) && builtinp(function->name)) {
    builtin_t name = builtin(function->name);

    if ((name == LET) || (name == LETSTAR)) {
      if (args == NULL) error2(noargument);
      object *assigns = first(args);
      if (!listp(assigns)) error(notalist, assigns);
      object *forms = cdr(args);
      object *newenv = env;
      protect(newenv);
      while (assigns != NULL) {
        object *assign = car(assigns);
        if (!consp(assign)) push(cons(assign,nil), newenv);
        else if (cdr(assign) == NULL) push(cons(first(assign),nil), newenv);
        else push(cons(first(assign), eval(second(assign),env)), newenv);
        car(GCStack) = newenv;
        if (name == LETSTAR) env = newenv;
        assigns = cdr(assigns);
      }
      env = newenv;
      unprotect();
      form = tf_progn(forms,env);
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
      return cons(bsymbol(CLOSURE), cons(envcopy,args));
    }

    switch(fntype(name)) {    
      case SPECIAL_FORMS:
        Context = name;
        checkargs(args);
        return ((fn_ptr_type)lookupfn(name))(args, env);
  
      case TAIL_FORMS:
        Context = name;
        checkargs(args);
        form = ((fn_ptr_type)lookupfn(name))(args, env);
        TC = 1;
        goto EVAL;
    }
  }

  // Evaluate the parameters - result in head
  int TCstart = TC;
  object *head;
  if (consp(function) && !(isbuiltin(car(function), LAMBDA) || isbuiltin(car(function), CLOSURE)
    || car(function)->type == CODE)) { Context = NIL; error(illegalfn, function); }
  if (symbolp(function)) {
    object *pair = findpair(function, env);
    if (pair != NULL) head = cons(cdr(pair), NULL); else head = cons(function, NULL);
  } else head = cons(eval(function, env), NULL);
  protect(head); // Don't GC the result list
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

  object *fname = function;
  function = car(head);
  args = cdr(head);

  if (symbolp(function)) {
    if (!builtinp(function->name)) { Context = NIL; error(illegalfn, function); }
    builtin_t bname = builtin(function->name);
    Context = bname;
    checkminmax(bname, nargs);
    intptr_t call = lookupfn(bname);
    if (call == 0) error(illegalfn, function);
    object *result = ((fn_ptr_type)call)(args, env);
    unprotect();
    return result;
  }

  if (consp(function)) {
    symbol_t name = sym(NIL);
    if (!listp(fname)) name = fname->name;

    if (isbuiltin(car(function), LAMBDA)) { 
      if (tstflag(BACKTRACE)) backtrace(name);
      form = closure(TCstart, name, function, args, &env);
      unprotect();
      int trace = tracing(name);
      if (trace || tstflag(BACKTRACE)) {
        object *result = eval(form, env);
        if (trace) {
          indent((--(TraceDepth[trace-1]))<<1, ' ', pserial);
          pint(TraceDepth[trace-1], pserial);
          pserial(':'); pserial(' ');
          printobject(fname, pserial); pfstring(" returned ", pserial);
          printobject(result, pserial); pln(pserial);
        }
        if (tstflag(BACKTRACE)) TraceTop = modbacktrace(TraceTop-1);
        return result;
      } else {
        TC = 1;
        goto EVAL;
      }
    }

    if (isbuiltin(car(function), CLOSURE)) {
      function = cdr(function);
      if (tstflag(BACKTRACE)) backtrace(name);
      form = closure(TCstart, name, function, args, &env);
      unprotect();
      if (tstflag(BACKTRACE)) {
        object *result = eval(form, env);
        TraceTop = modbacktrace(TraceTop-1);
        return result;
      } else {
        TC = 1;
        goto EVAL;
      }
    }

  }
  error(illegalfn, fname); return nil;
}

// Print functions

void pserial (char c) {
  LastPrint = c;
  if (c == '\n') Serial.write('\r');
  Serial.write(c);
}

const char ControlCodes[] = "Null\0SOH\0STX\0ETX\0EOT\0ENQ\0ACK\0Bell\0Backspace\0Tab\0Newline\0VT\0"
"Page\0Return\0SO\0SI\0DLE\0DC1\0DC2\0DC3\0DC4\0NAK\0SYN\0ETB\0CAN\0EM\0SUB\0Escape\0FS\0GS\0RS\0US\0Space\0";

void pcharacter (uint8_t c, pfun_t pfun) {
  if (!tstflag(PRINTREADABLY)) pfun(c);
  else {
    pfun('#'); pfun('\\');
    if (c <= 32) {
      const char *p = ControlCodes;
      while (c > 0) {p = p + strlen(p) + 1; c--; }
      pfstring(p, pfun);
    } else if (c < 127) pfun(c);
    else pint(c, pfun);
  }
}

void pstring (char *s, pfun_t pfun) {
  while (*s) pfun(*s++);
}

void plispstring (object *form, pfun_t pfun) {
  plispstr(form->name, pfun);
}

void plispstr (symbol_t name, pfun_t pfun) {
  object *form = (object *)name;
  while (form != NULL) {
    int chars = form->chars;
    for (int i=(sizeof(int)-1)*8; i>=0; i=i-8) {
      char ch = chars>>i & 0xFF;
      if (tstflag(PRINTREADABLY) && (ch == '"' || ch == '\\')) pfun('\\');
      if (ch) pfun(ch);
    }
    form = car(form);
  }
}

void printstring (object *form, pfun_t pfun) {
  if (tstflag(PRINTREADABLY)) pfun('"');
  plispstr(form->name, pfun);
  if (tstflag(PRINTREADABLY)) pfun('"');
}

void pbuiltin (builtin_t name, pfun_t pfun) {
  int n = name<tablesize(0);
  const char *s = table(n?0:1)[n?name:name-tablesize(0)].string;
  while (1) {
    char c = *s++;
    if (c == 0) return;
    pfun(c);
  }
}

void pradix40 (symbol_t name, pfun_t pfun) {
  uint32_t x = untwist(name);
  for (int d=102400000; d>0; d = d/40) {
    uint32_t j = x/d;
    char c = fromradix40(j);
    if (c == 0) return;
    pfun(c); x = x - j*d;
  }
}

void printsymbol (object *form, pfun_t pfun) {
  psymbol(form->name, pfun);
}

void psymbol (symbol_t name, pfun_t pfun) {
  if (longnamep(name)) plispstr(name, pfun);
  else {
    uint32_t value = untwist(name);
    if (value < PACKEDS) error2("invalid symbol");
    else if (value >= BUILTINS) pbuiltin((builtin_t)(value-BUILTINS), pfun);
    else pradix40(name, pfun);
  }
}

void pfstring (const char *s, pfun_t pfun) {
  while (1) {
    char c = *s++;
    if (c == 0) return;
    pfun(c);
  }
}

void pint (int i, pfun_t pfun) {
  uint32_t j = i;
  if (i<0) { pfun('-'); j=-i; }
  pintbase(j, 10, pfun);
}

void pintbase (uint32_t i, uint8_t base, pfun_t pfun) {
  int lead = 0; uint32_t p = 1000000000;
  if (base == 2) p = 0x80000000; else if (base == 16) p = 0x10000000;
  for (uint32_t d=p; d>0; d=d/base) {
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
  if (isnan(f)) { pfstring("NaN", pfun); return; }
  if (f == 0.0) { pfun('0'); return; }
  if (isinf(f)) { pfstring("Inf", pfun); return; }
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
    testescape();
    form = cdr(form);
  }
  if (form != NULL) {
    pfstring(" . ", pfun);
    printobject(form, pfun);
  }
  pfun(')');
}

void pstream (object *form, pfun_t pfun) {
  pfun('<');
  nstream_t nstream = (form->integer)>>8;
  bool n = nstream<USERSTREAMS;
  const char *streamname = streamtable(n?0:1)[n?nstream:nstream-USERSTREAMS].streamname;
  pfstring(streamname, pfun);
  pfstring("-stream ", pfun);
  pint(form->integer & 0xFF, pfun);
  pfun('>');
}

void printobject (object *form, pfun_t pfun) {
  if (form == NULL) pfstring("nil", pfun);
  else if (listp(form) && isbuiltin(car(form), CLOSURE)) pfstring("<closure>", pfun);
  else if (listp(form)) plist(form, pfun);
  else if (integerp(form)) pint(form->integer, pfun);
  else if (floatp(form)) pfloat(form->single_float, pfun);
  else if (symbolp(form)) { if (form->name != sym(NOTHING)) printsymbol(form, pfun); }
  else if (characterp(form)) pcharacter(form->chars, pfun);
  else if (stringp(form)) printstring(form, pfun);
  else if (arrayp(form)) printarray(form, pfun);
  else if (streamp(form)) pstream(form, pfun);
  else error2("error in print");
}

void prin1object (object *form, pfun_t pfun) {
  flags_t temp = Flags;
  clrflag(PRINTREADABLY);
  printobject(form, pfun);
  Flags = temp;
}

// Read functions

int glibrary () {
  char c = LispLibrary[GlobalStringIndex++];
  return (c != 0) ? c : -1; // -1?
}

void loadfromlibrary (object *env) {
  GlobalStringIndex = 0;
  object *line = readmain(glibrary);
  while (line != NULL) {
    protect(line);
    eval(line, env);
    unprotect();
    line = readmain(glibrary);
  }
}

#if defined(lineeditor)
// For line editor
const char BEEP = 7;
const char REPEATLINE = '\t';   // Tab
const char KEY_ESC = 27;
const char KEY_BS = 8;
const char AUTOCOMPLETE = '\t'; // Tab
const int TerminalWidth = 80;
int WritePtr = 0, ReadPtr = 0, LastWritePtr = 0;
const int KybdBufSize = 333;    // 42*8 - 3
char KybdBuf[KybdBufSize];
uint8_t KybdAvailable = 0;

bool reset_autocomplete = false;

void autocomplete () {
  static int bufIndex = 0, matchLen = 0, LastKeywordLenDif = 0;
  static unsigned int i = 0;
  int gap = 0;
  
  // Only update what we're matching if we're not already looking through the buffer
  if (reset_autocomplete == true) { 
    i = 0; // Reset the search
    LastKeywordLenDif = 0;
    reset_autocomplete = false;
    bufIndex = WritePtr;
    for (matchLen = 0; matchLen < 32; matchLen++) {
      int bufLoc = WritePtr - matchLen; //scan the buffer backwards away from the last character written
      if ((KybdBuf[bufLoc] == ' ') || (KybdBuf[bufLoc] == '(') || (KybdBuf[bufLoc] == '\n')) {
        if (matchLen > 0) { //if the first character is one of those then we don't have to keep looking
          // if we found these characters then go forward to the previous character
          bufIndex = bufLoc + 1;
          matchLen--;
        }
        break;
      }
      // Do this test here in case the first character in the buffer is one of the characters we test for
      else if (bufLoc == 0) { 
        bufIndex = bufLoc; 
        break; 
      } 
    }
  }

  if (matchLen > 0) {
    // Erase the previously shown keyword
    for (int n=0; n<LastKeywordLenDif; n++) processkey(KEY_BS);

    // Scan the table for keywords that start with the match buffer
    int entries = tablesize(0) + tablesize(1);
    while (true) {
      bool n = i<tablesize(0);
      const char *k = table(n?0:1)[n?i:i-tablesize(0)].string;
      i = (i + 1) % entries; // Wrap
      if (*k == KybdBuf[bufIndex]) {
        if (strncmp(k, &KybdBuf[bufIndex], matchLen) == 0) {
          // Skip the letters we're matching because they're already there
          LastKeywordLenDif = strlen(k) - matchLen;
          while (*(k + matchLen)) processkey(*(k++ + matchLen));
          return;
        }
      }
      gap++; 
      if (gap == entries) return; // No keywords with this letter
    }
  }
}

// Parenthesis highlighting
void esc (int p, char c) {
  Serial.write('\e'); Serial.write('[');
  Serial.write((char)('0'+ p/100));
  Serial.write((char)('0'+ (p/10) % 10));
  Serial.write((char)('0'+ p % 10));
  Serial.write(c);
}

void highlight (int p, int wp, bool invert) {
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
    if (invert) Serial.print(VT100_HIGHLIGHT);
    Serial.write('('); Serial.write('\b');
    // Go back
    if (up) esc(up, 'B'); // Down
    if (col > targetcol) esc(left, 'C'); else esc(-left, 'D');
    Serial.write('\b'); Serial.write(')');
    if (invert) Serial.print("\e[0m"); // Reset
  }
}

void processkey (char c) {
  if (c == KEY_ESC) { setflag(ESCAPE); return; }
  #if defined(vt100)
    static int parenthesis = 0, wp = 0;
    static bool string = false;
    // Undo previous parenthesis highlight
    highlight(parenthesis, wp, false);
    parenthesis = 0;
  #endif
  // Edit buffer
  if (c == '\n' || c == '\r') {
    pserial('\n');
    KybdAvailable = 1;
    ReadPtr = 0; LastWritePtr = WritePtr;
    return;
  }
  if (c == KEY_BS || c == 0x7f) { // Backspace key
    if (WritePtr > 0) {
      WritePtr--;
      Serial.write(KEY_BS); Serial.write(' '); Serial.write(KEY_BS);
      if (WritePtr) c = KybdBuf[WritePtr-1];
    }
  } else if (c == REPEATLINE && WritePtr == 0) {
    for (int i = 0; i < LastWritePtr; i++) Serial.write(KybdBuf[i]);
    WritePtr = LastWritePtr;
  } else if (WritePtr < KybdBufSize) {
    KybdBuf[WritePtr++] = c;
    Serial.write(c);
  }
  #if defined(vt100)
    // Do parenthesis highlight
    if (c == ')' && !string) {
      int search = WritePtr-1, level = 0; bool string2 = false;
      while (search >= 0 && parenthesis == 0) {
        c = KybdBuf[search--];
        if (c == '"') string2 = !string2;
        if (c == ')' && !string2) level++;
        if (c == '(' && !string2) {
          level--;
          if (level == 0) {parenthesis = WritePtr-search-1; wp = WritePtr; }
        }
      }
      highlight(parenthesis, wp, true);
    }
  #endif
  return;
}
#endif

void gserial_flush () {
  #if defined (serialmonitor)
  Serial.flush();
  #endif
  #if defined(lineeditor)
  KybdAvailable = 0;
  WritePtr = 0;
  #endif
}

int gserial () {
#if defined(lineeditor)
  while (!KybdAvailable) {
    while (!Serial.available());
    char c = Serial.read();
    if (c == AUTOCOMPLETE && WritePtr != 0) autocomplete(); else { processkey(c); reset_autocomplete = true; }
  }
  if (ReadPtr != WritePtr) return KybdBuf[ReadPtr++];
  KybdAvailable = 0;
  WritePtr = 0;
  return '\n';
#else
  unsigned long start = millis();
  while (!Serial.available()) { delay(1); if (millis() - start > 1000) clrflag(NOECHO); }
  char c = Serial.read();
  if (c != '\n' && !tstflag(NOECHO)) pserial(c);
  return c;
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
  if (ch == '"') return readstring('"', true, gfun);

  // Parse symbol, character, or number
  int index = 0, base = 10, sign = 1;
  char buffer[BUFFERSIZE];
  int bufmax = BUFFERSIZE-3; // Max index
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
      if (issp(ch) || isbr(ch)) return character(ch);
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
    else error2("illegal character after #");
    ch = gfun();
  }
  int valid; // 0=undecided, -1=invalid, +1=valid
  if (ch == '.') valid = 0; else if (digitvalue(ch)<base) valid = 1; else valid = -1;
  bool isexponent = false;
  int exponent = 0, esign = 1;
  float divisor = 10.0;

  while(!issp(ch) && !isbr(ch) && index < bufmax) {
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
  if (isbr(ch)) LastChar = ch;
  if (isfloat && valid == 1) return makefloat(fresult * sign * pow(10, exponent * esign));
  else if (valid == 1) {
    if (base == 10 && result > ((unsigned int)INT_MAX+(1-sign)/2))
      return makefloat((float)result*sign);
    return number(result*sign);
  } else if (base == 0) {
    if (index == 1) return character(buffer[0]);
    const char *p = ControlCodes; char c = 0;
    while (c < 33) {
      if (strcasecmp(buffer, p) == 0) return character(c);
      p = p + strlen(p) + 1; c++;
    }
    if (index == 3) return character((buffer[0]*10+buffer[1])*10+buffer[2]-5328);
    error2("unknown character");
  }

  builtin_t x = lookupbuiltin(buffer);
  if (x == NIL) return nil;
  if (x != ENDFUNCTIONS) return bsymbol(x);
  if (index <= 6 && valid40(buffer)) return intern(twist(pack40(buffer)));
  return internlong(buffer);
}

object *readrest (gfun_t gfun) {
  object *item = nextitem(gfun);
  object *head = NULL;
  object *tail = NULL;

  while (item != (object *)KET) {
    if (item == (object *)BRA) {
      item = readrest(gfun);
    } else if (item == (object *)QUO) {
      item = cons(bsymbol(QUOTE), cons(read(gfun), NULL));
    } else if (item == (object *)DOT) {
      tail->cdr = read(gfun);
      if (readrest(gfun) != NULL) error2("malformed list");
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

gfun_t GFun;

int glast () {
  if (LastChar) {
    char temp = LastChar;
    LastChar = 0;
    return temp;
  }
  return GFun();
}

object *readmain (gfun_t gfun) {
  GFun = gfun;
  LastChar = 0;
  return read(glast);
}

object *read (gfun_t gfun) {
  object *item = nextitem(gfun);
  if (item == (object *)KET) error2("incomplete list");
  if (item == (object *)BRA) return readrest(gfun);
  if (item == (object *)DOT) return read(gfun);
  if (item == (object *)QUO) return cons(bsymbol(QUOTE), cons(read(gfun), NULL));
  return item;
}

// Setup

void initenv () {
  GlobalEnv = NULL;
  tee = bsymbol(TEE);
}

void initgfx () {
  #if defined(gfxsupport)
    #if defined(ARDUINO_M5STACK_TAB5)
      M5.begin();
      tft.fillScreen(TFT_BLACK);      
      tft.setRotation(3); // Origin top left
    #elif defined(ARDUINO_TTGO_LoRa32_v21new)
      Wire.begin();
      tft.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      tft.fillScreen(COLOR_BLACK);
      tft.display();
    #else
      tft.init(135, 240);
      #if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT)
        pinMode(TFT_I2C_POWER, OUTPUT);
        digitalWrite(TFT_I2C_POWER, HIGH);
        tft.setRotation(3);
      #else
        tft.setRotation(1);
      #endif
      tft.fillScreen(ST77XX_BLACK);
      pinMode(TFT_BACKLITE, OUTPUT);
      digitalWrite(TFT_BACKLITE, HIGH);
    #endif
  #endif
}

void setup () {
  Serial.begin(9600);
  int start = millis();
  while ((millis() - start) < 5000) { if (Serial) break; }
  #if defined(BOARD_HAS_PSRAM)
  if (!psramInit()) { Serial.print("the PSRAM couldn't be initialized"); for(;;); }
  Workspace = (object*) ps_malloc(WORKSPACESIZE*8);
  if (!Workspace) { Serial.print("the Workspace couldn't be allocated"); for(;;); }
  #endif
  int stackhere = 0; StackBottom = &stackhere;
  initworkspace();
  initenv();
  initsleep();
  initgfx();
  pfstring(PSTR("uLisp 4.9 "), pserial); pln(pserial);
}

// Read/Evaluate/Print loop

void repl (object *env) {
  for (;;) {
    randomSeed(micros());
    #if defined(printfreespace)
    if (!tstflag(NOECHO)) gc(NULL, env);
    pint(Freespace+1, pserial);
    #endif
    if (BreakLevel) {
      pfstring(" : ", pserial);
      pint(BreakLevel, pserial);
    }
    pserial('>'); pserial(' ');
    Context = NIL;
    object *line = readmain(gserial);
    // Break handling
    if (BreakLevel) {
      if (line == nil || line == bsymbol(COLONC)) {
        pln(pserial); return;
      } else if (line == bsymbol(COLONA)) {
        pln(pserial); pln(pserial);
        GCStack = NULL;
        longjmp(*handler, 1);
      } else if (line == bsymbol(COLONB)) {
        pln(pserial); printbacktrace();
        line = bsymbol(NOTHING);
      }
    }
    if (line == (object *)KET) error2("unmatched right bracket");
    protect(line);
    pfl(pserial);
    line = eval(line, env);
    pfl(pserial);
    printobject(line, pserial);
    unprotect();
    pfl(pserial);
    pln(pserial);
  }
}

void loop () {
  if (!setjmp(toplevel_handler)) {
    #if defined(resetautorun)
    volatile int autorun = 12; // Fudge to keep code size the same
    #else
    volatile int autorun = 13;
    #endif
    if (autorun == 12) autorunimage();
  }
  ulisperror();
  repl(NULL);
}

void ulisperror () {
  // Come here after error
  delay(100); while (Serial.available()) Serial.read();
  clrflag(NOESC); BreakLevel = 0; TraceStart = 0; TraceTop = 0;
  GlobalStringStreamTail = NULL;
  for (int i=0; i<TRACEMAX; i++) TraceDepth[i] = 0;
  #if defined(sdcardsupport)
  SDpfile.close(); SDgfile.close();
  #endif
  #if defined(lisplibrary)
  if (!tstflag(LIBRARYLOADED)) { setflag(LIBRARYLOADED); loadfromlibrary(NULL); clrflag(NOECHO); }
  #endif
  client.stop();
}
