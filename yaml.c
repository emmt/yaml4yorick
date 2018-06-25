/*
 * yaml.c --
 *
 * Implements interface to YAML files.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2018: Éric Thiébaut <eric.thiebaut@univ-lyon1.fr>
 *
 * See LICENSE.md for details.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <yaml.h>

#include <pstdlib.h>
#include <play.h>
#include <yapi.h>

/* Define some macros to get rid of some GNU extensions when not compiling
   with GCC. */
#if ! (defined(__GNUC__) && __GNUC__ > 1)
#   define __attribute__(x)
#   define __inline__
#   define __FUNCTION__        ""
#   define __PRETTY_FUNCTION__ ""
#endif

#define FALSE 0
#define TRUE  1

#define NEW(type) ((type*)calloc(1, sizeof(type)))

PLUG_API void y_error(const char *) __attribute__ ((noreturn));

static void push_string(const char* str);
static void push_ustring(const yaml_char_t* str);
static void define_int_const(const char* name, int value);

/* Parse literal integer, return NULL on error, pointer to next unparsed
 * character else. */
static const char* parse_integer(const char* str, long* ptr,
                                 unsigned int flags);

/* Flags for parse_integer. */
#define TRIM_LEFT  (1U << 0)
#define TRIM_RIGHT (1U << 1)
#define NO_SIGN    (1U << 2)

/*---------------------------------------------------------------------------*/
/* UTILITIES */

static void
define_int_const(const char* name, int value)
{
  ypush_int(value);
  yput_global(yget_global(name, 0), 0);
  yarg_drop(1);
}

static int initialized = FALSE;

static long anchor_index = -1L;
static long encoding_index = -1L;
static long implicit_index = -1L;
static long plain_implicit_index = -1L;
static long quoted_implicit_index = -1L;
static long style_index = -1L;
static long tag_index = -1L;
static long value_index = -1L;
static long version_index = -1L;

static void
initialize()
{
  /* Initialize all keyword indexes. */
#define INIT(s) if (s##_index == -1L) s##_index = yget_global(#s, 0)
  INIT(anchor);
  INIT(encoding);
  INIT(implicit);
  INIT(plain_implicit);
  INIT(quoted_implicit);
  INIT(style);
  INIT(tag);
  INIT(value);
  INIT(version);
#undef INIT

#define DEFINE_INT_CONST(c)  define_int_const(#c, c)
  /* YStream encoding. */
  DEFINE_INT_CONST(YAML_ANY_ENCODING);
  DEFINE_INT_CONST(YAML_UTF8_ENCODING);
  DEFINE_INT_CONST(YAML_UTF16LE_ENCODING);
  DEFINE_INT_CONST(YAML_UTF16BE_ENCODING);

  /* Line break types. */
  DEFINE_INT_CONST(YAML_ANY_BREAK);
  DEFINE_INT_CONST(YAML_CR_BREAK);
  DEFINE_INT_CONST(YAML_LN_BREAK);
  DEFINE_INT_CONST(YAML_CRLN_BREAK);

  /* Error codes. */
  DEFINE_INT_CONST(YAML_NO_ERROR);
  DEFINE_INT_CONST(YAML_MEMORY_ERROR);
  DEFINE_INT_CONST(YAML_READER_ERROR);
  DEFINE_INT_CONST(YAML_SCANNER_ERROR);
  DEFINE_INT_CONST(YAML_PARSER_ERROR);
  DEFINE_INT_CONST(YAML_COMPOSER_ERROR);
  DEFINE_INT_CONST(YAML_WRITER_ERROR);
  DEFINE_INT_CONST(YAML_EMITTER_ERROR);

  /* Scalar styles. */
  DEFINE_INT_CONST(YAML_ANY_SCALAR_STYLE);
  DEFINE_INT_CONST(YAML_PLAIN_SCALAR_STYLE);
  DEFINE_INT_CONST(YAML_SINGLE_QUOTED_SCALAR_STYLE);
  DEFINE_INT_CONST(YAML_DOUBLE_QUOTED_SCALAR_STYLE);
  DEFINE_INT_CONST(YAML_LITERAL_SCALAR_STYLE);
  DEFINE_INT_CONST(YAML_FOLDED_SCALAR_STYLE);

  /* Sequence styles. */
  DEFINE_INT_CONST(YAML_ANY_SEQUENCE_STYLE);
  DEFINE_INT_CONST(YAML_BLOCK_SEQUENCE_STYLE);
  DEFINE_INT_CONST(YAML_FLOW_SEQUENCE_STYLE);

  /* Mapping styles. */
  DEFINE_INT_CONST(YAML_ANY_MAPPING_STYLE);
  DEFINE_INT_CONST(YAML_BLOCK_MAPPING_STYLE);
  DEFINE_INT_CONST(YAML_FLOW_MAPPING_STYLE);

  /* Token types. */
  DEFINE_INT_CONST(YAML_NO_TOKEN);
  DEFINE_INT_CONST(YAML_STREAM_START_TOKEN);
  DEFINE_INT_CONST(YAML_STREAM_END_TOKEN);
  DEFINE_INT_CONST(YAML_VERSION_DIRECTIVE_TOKEN);
  DEFINE_INT_CONST(YAML_TAG_DIRECTIVE_TOKEN);
  DEFINE_INT_CONST(YAML_DOCUMENT_START_TOKEN);
  DEFINE_INT_CONST(YAML_DOCUMENT_END_TOKEN);
  DEFINE_INT_CONST(YAML_BLOCK_SEQUENCE_START_TOKEN);
  DEFINE_INT_CONST(YAML_BLOCK_MAPPING_START_TOKEN);
  DEFINE_INT_CONST(YAML_BLOCK_END_TOKEN);
  DEFINE_INT_CONST(YAML_FLOW_SEQUENCE_START_TOKEN);
  DEFINE_INT_CONST(YAML_FLOW_SEQUENCE_END_TOKEN);
  DEFINE_INT_CONST(YAML_FLOW_MAPPING_START_TOKEN);
  DEFINE_INT_CONST(YAML_FLOW_MAPPING_END_TOKEN);
  DEFINE_INT_CONST(YAML_BLOCK_ENTRY_TOKEN);
  DEFINE_INT_CONST(YAML_FLOW_ENTRY_TOKEN);
  DEFINE_INT_CONST(YAML_KEY_TOKEN);
  DEFINE_INT_CONST(YAML_VALUE_TOKEN);
  DEFINE_INT_CONST(YAML_ALIAS_TOKEN);
  DEFINE_INT_CONST(YAML_ANCHOR_TOKEN);
  DEFINE_INT_CONST(YAML_TAG_TOKEN);
  DEFINE_INT_CONST(YAML_SCALAR_TOKEN);

  /* Event types. */
  DEFINE_INT_CONST(YAML_NO_EVENT);
  DEFINE_INT_CONST(YAML_STREAM_START_EVENT);
  DEFINE_INT_CONST(YAML_STREAM_END_EVENT);
  DEFINE_INT_CONST(YAML_DOCUMENT_START_EVENT);
  DEFINE_INT_CONST(YAML_DOCUMENT_END_EVENT);
  DEFINE_INT_CONST(YAML_ALIAS_EVENT);
  DEFINE_INT_CONST(YAML_SCALAR_EVENT);
  DEFINE_INT_CONST(YAML_SEQUENCE_START_EVENT);
  DEFINE_INT_CONST(YAML_SEQUENCE_END_EVENT);
  DEFINE_INT_CONST(YAML_MAPPING_START_EVENT);
  DEFINE_INT_CONST(YAML_MAPPING_END_EVENT);

  /* Node types. */
  DEFINE_INT_CONST(YAML_NO_NODE);
  DEFINE_INT_CONST(YAML_SCALAR_NODE);
  DEFINE_INT_CONST(YAML_SEQUENCE_NODE);
  DEFINE_INT_CONST(YAML_MAPPING_NODE);

  /* Parser states. */
  DEFINE_INT_CONST(YAML_PARSE_STREAM_START_STATE);
  DEFINE_INT_CONST(YAML_PARSE_IMPLICIT_DOCUMENT_START_STATE);
  DEFINE_INT_CONST(YAML_PARSE_DOCUMENT_START_STATE);
  DEFINE_INT_CONST(YAML_PARSE_DOCUMENT_CONTENT_STATE);
  DEFINE_INT_CONST(YAML_PARSE_DOCUMENT_END_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_NODE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_NODE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_SEQUENCE_ENTRY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_MAPPING_KEY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_BLOCK_MAPPING_VALUE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_SEQUENCE_ENTRY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_MAPPING_FIRST_KEY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_MAPPING_KEY_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_MAPPING_VALUE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE);
  DEFINE_INT_CONST(YAML_PARSE_END_STATE);

  /* Emitter states. */
  DEFINE_INT_CONST(YAML_EMIT_STREAM_START_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FIRST_DOCUMENT_START_STATE);
  DEFINE_INT_CONST(YAML_EMIT_DOCUMENT_START_STATE);
  DEFINE_INT_CONST(YAML_EMIT_DOCUMENT_CONTENT_STATE);
  DEFINE_INT_CONST(YAML_EMIT_DOCUMENT_END_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_SEQUENCE_FIRST_ITEM_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_SEQUENCE_ITEM_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_MAPPING_FIRST_KEY_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_MAPPING_KEY_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_MAPPING_SIMPLE_VALUE_STATE);
  DEFINE_INT_CONST(YAML_EMIT_FLOW_MAPPING_VALUE_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_SEQUENCE_FIRST_ITEM_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_SEQUENCE_ITEM_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_MAPPING_FIRST_KEY_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_MAPPING_KEY_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_MAPPING_SIMPLE_VALUE_STATE);
  DEFINE_INT_CONST(YAML_EMIT_BLOCK_MAPPING_VALUE_STATE);
  DEFINE_INT_CONST(YAML_EMIT_END_STATE);

#undef DEFINE_INT_CONST

  initialized = TRUE;
}

static void
push_string(const char* str)
{
  ypush_q(NULL)[0] = p_strcpy(str);
}

static void
push_ustring(const yaml_char_t* str)
{
  push_string((const char*)str);
}

static const char*
parse_integer(const char* str, long* ptr, unsigned int flags)
{
  long value;
  int c, sign = 0;

  if (str == NULL) {
    return NULL;
  }
  if ((flags & TRIM_LEFT) != 0) {
    while (isspace(str[0])) {
      ++str;
    }
  }
  c = str[0];
  if ((flags & NO_SIGN) == 0) {
    if (c == '+') {
      sign = +1;
      c = *++str;
    } else if (c == '-') {
      sign = -1;
      c = *++str;
    }
  }
  if (c < '0' || c > '9') {
    return NULL;
  }
  value = c - '0';
  while (TRUE) {
    c = *++str;
    if (c < '0' || c > '9') {
      break;
    }
    value = 10*value + (c - '0');
  }
  if ((flags & TRIM_RIGHT) != 0) {
    while (isspace(str[0])) {
      ++str;
    }
  }
  if (ptr != NULL) {
    *ptr = (sign < 0 ? -value : value);
  }
  return str;
}

/*---------------------------------------------------------------------------*/
/* YAML EVENT OBJECT */

static void    free_event(void* ptr);
static void   print_event(void* ptr);
static void    eval_event(void* ptr, int argc);
static void extract_event(void* ptr, char* name);

static y_userobj_t event_type = {
  /* type_name:  */   "yaml_event",
  /* on_free:    */    free_event,
  /* on_print:   */   print_event,
  /* on_eval:    */    eval_event,
  /* on_extract: */ extract_event,
  /* uo_ops:     */ (void *)0
};

typedef struct _event_t event_t;
struct _event_t {
  yaml_event_t event; /* event data */
  int init; /* contents has been initialized? */
};

static event_t*
push_event()
{
  event_t* obj = (event_t*)ypush_obj(&event_type, sizeof(event_t));
  obj->init = FALSE;
  return obj;
}

static void free_event(void* ptr)
{
  event_t* obj = (event_t*)ptr;
  if (obj->init) {
    yaml_event_delete(&obj->event);
  }
}

static void print_event(void* ptr)
{
  event_t* obj = (event_t*)ptr;
  if (obj->init) {
    y_print("initialized YAML event", 1);
  } else {
    y_print("uninitialized YAML event", 1);
  }
}

static void eval_event(void* ptr, int argc)
{
  y_error("not a callable object");
}

/* Yields event type.  Always successful. */
static yaml_event_type_t
get_event_type(const event_t* obj)
{
  return (obj != NULL && obj->init) ? obj->event.type : YAML_NO_EVENT;
}

static void
extract_mark(const yaml_mark_t* mark, const char* name)
{
  if (strcmp(name, "_index") == 0) {
    ypush_long(mark->index);
  } else if (strcmp(name, "_line") == 0) {
    ypush_long(mark->line);
  } else if (strcmp(name, "_column") == 0) {
    ypush_long(mark->column);
  } else {
    y_error("unknown member");
  }
}

#define EXTRACT(push, memb, expr)               \
  do {                                          \
    if (strcmp(name, #memb) == 0) {             \
      push(obj->expr);                          \
      return;                                   \
    }                                           \
  } while (0)

#define EXTRACT_INT(memb, expr)  EXTRACT(ypush_int,    memb, expr)
#define EXTRACT_STR(memb, expr)  EXTRACT(push_ustring, memb, expr)
#define EXTRACT_LONG(memb, expr) EXTRACT(ypush_long,   memb, expr)

static void
extract_event(void* ptr, char* name)
{
  char buffer[64];
  event_t* obj = (event_t*)ptr;

  if (strcmp(name, "type") == 0) {
    ypush_int(get_event_type(obj));
    return;
  }
  if (obj->init) {
    switch (obj->event.type) {
    case YAML_NO_EVENT:
      break;

    case YAML_STREAM_START_EVENT:
      EXTRACT_INT(encoding, event.data.stream_start.encoding);
      break;

    case YAML_STREAM_END_EVENT:
      break;

    case YAML_DOCUMENT_START_EVENT:
      /* FIXME: missing: tag_directives */
      EXTRACT_INT(implicit, event.data.document_start.implicit);
      if (strcmp(name, "version") == 0) {
        sprintf(buffer, "%d.%d",
                obj->event.data.document_start.version_directive->major,
                obj->event.data.document_start.version_directive->minor);
        push_string(buffer);
        return;
      }
      break;

    case YAML_DOCUMENT_END_EVENT:
      EXTRACT_INT(implicit, event.data.document_end.implicit);
      break;

    case YAML_ALIAS_EVENT:
      EXTRACT_STR(anchor, event.data.alias.anchor);
      break;

    case YAML_SCALAR_EVENT:
      EXTRACT_STR(anchor,          event.data.scalar.anchor);
      EXTRACT_STR(tag,             event.data.scalar.tag);
      EXTRACT_STR(value,           event.data.scalar.value);
      EXTRACT_LONG(length,         event.data.scalar.length);
      EXTRACT_INT(plain_implicit,  event.data.scalar.plain_implicit);
      EXTRACT_INT(quoted_implicit, event.data.scalar.quoted_implicit);
      EXTRACT_INT(style,           event.data.scalar.style);
      break;

    case YAML_SEQUENCE_START_EVENT:
      EXTRACT_STR(anchor,   event.data.sequence_start.anchor);
      EXTRACT_STR(tag,      event.data.sequence_start.tag);
      EXTRACT_INT(implicit, event.data.sequence_start.implicit);
      EXTRACT_INT(style,    event.data.sequence_start.style);
      break;

    case YAML_SEQUENCE_END_EVENT:
      break;

    case YAML_MAPPING_START_EVENT:
      EXTRACT_STR(anchor,   event.data.mapping_start.anchor);
      EXTRACT_STR(tag,      event.data.mapping_start.tag);
      EXTRACT_INT(implicit, event.data.mapping_start.implicit);
      EXTRACT_INT(style,    event.data.mapping_start.style);
      break;

    case YAML_MAPPING_END_EVENT:
      break;
    }
    if (strncmp(name, "start", 5) == 0) {
      extract_mark(&obj->event.start_mark, name + 5);
      return;
    }
    if (strncmp(name, "end", 3) == 0) {
      extract_mark(&obj->event.end_mark, name + 3);
      return;
    }
    y_error("unknown YAML event member");
  } else {
    y_error("uninitialized YAML event");
  }
}

/*---------------------------------------------------------------------------*/
/* YAML PARSER OBJECT */

static void    free_parser(void* ptr);
static void   print_parser(void* ptr);
static void    eval_parser(void* ptr, int argc);
static void extract_parser(void* ptr, char* name);

static y_userobj_t parser_type = {
  /* type_name:  */   "yaml_parser",
  /* on_free:    */    free_parser,
  /* on_print:   */   print_parser,
  /* on_eval:    */    eval_parser,
  /* on_extract: */ extract_parser,
  /* uo_ops:     */ (void *)0
};

/* An application must not alternate the calls of yaml_parser_scan() with
 * the calls of yaml_parser_parse() or yaml_parser_load(). Doing this will
 * break the parser.
 */
typedef enum _parsing {
  ANY,   /* parser type not yet decided */
  SCAN,  /* only calls yaml_parser_scan() are allowed */
  PARSE, /* only calls yaml_parser_parse() are allowed */
  LOAD   /* only calls yaml_parser_load() are allowed */
} parsing_t;

static const char*
parsing_name(parsing_t parsing)
{
  switch (parsing) {
  case ANY:   return "any";
  case SCAN:  return "scan";
  case PARSE: return "parse";
  case LOAD:  return "load";
  default:    return "unknown";
  }
}

typedef struct _parser_t parser_t;
struct _parser_t {
  yaml_parser_t parser; /* parser data */
  int init; /* parser has been initialized? */
  parsing_t parsing;
  FILE* input; /* input file */
};

static parser_t* push_parser()
{
  parser_t* obj = (parser_t*)ypush_obj(&parser_type, sizeof(parser_t));
  obj->init = FALSE;
  obj->parsing = ANY;
  return obj;
}

static void free_parser(void* ptr)
{
  parser_t* obj = (parser_t*)ptr;
  if (obj->init) {
    yaml_parser_delete(&obj->parser);
  }
  if (obj->input != NULL && obj->input != stdin) {
    fclose(obj->input);
  }
}

static void print_parser(void* ptr)
{
  parser_t* obj = (parser_t*)ptr;
  if (obj->init) {
    y_print("initialized YAML parser (parsing=", 0);
    y_print(parsing_name(obj->parsing), 0);
    y_print(")", 1);
  } else {
    y_print("uninitialized YAML parser", 1);
  }
}

static void eval_parser(void* ptr, int argc)
{
  y_error("not a callable object");
}

static void extract_parser(void* ptr, char* name)
{
  parser_t* obj = (parser_t*)ptr;
  if (obj->init) {
    y_error("unknown YAML parser member");
  } else {
    y_error("uninitialized YAML parser");
  }
}

/*---------------------------------------------------------------------------*/
/* YAML EMITTER OBJECT */

static void    free_emitter(void* ptr);
static void   print_emitter(void* ptr);
static void    eval_emitter(void* ptr, int argc);
static void extract_emitter(void* ptr, char* name);

static y_userobj_t emitter_type = {
  /* type_name:  */   "yaml_emitter",
  /* on_free:    */    free_emitter,
  /* on_print:   */   print_emitter,
  /* on_eval:    */    eval_emitter,
  /* on_extract: */ extract_emitter,
  /* uo_ops:     */ (void *)0
};

typedef struct _emitter_t emitter_t;
struct _emitter_t {
  yaml_emitter_t emitter; /* emitter data */
  int init; /* emitter has been initialized? */
  int open; /* output file is open and has to be closed */
  FILE* output; /* output file */
};

static emitter_t* push_emitter()
{
  emitter_t* obj = (emitter_t*)ypush_obj(&emitter_type, sizeof(emitter_t));
  obj->init = FALSE;
  obj->open = FALSE;
  return obj;
}

static void free_emitter(void* ptr)
{
  emitter_t* obj = (emitter_t*)ptr;
  if (obj->init) {
    yaml_emitter_delete(&obj->emitter);
  }
  if (obj->output != NULL && obj->open) {
    fclose(obj->output);
  }
}

static void print_emitter(void* ptr)
{
  emitter_t* obj = (emitter_t*)ptr;
  if (obj->init) {
    y_print("initialized YAML emitter", 1);
  } else {
    y_print("uninitialized YAML emitter", 1);
  }
}

static void eval_emitter(void* ptr, int argc)
{
  y_error("not a callable object");
}

static void extract_emitter(void* ptr, char* name)
{
  emitter_t* obj = (emitter_t*)ptr;
  if (obj->init) {
    y_error("unknown YAML emitter member");
  } else {
    y_error("uninitialized YAML emitter");
  }
}

/*---------------------------------------------------------------------------*/

static void
debug(int argc)
{
  int iarg;
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    const char* name = yget_obj(iarg, NULL);
    fprintf(stderr, "arg[%d] -> %s\n", iarg, name);
  }
}

void
Y_yaml_debug(int argc)
{
  debug(argc);
}

void
Y___yaml_init__(int argc)
{
  initialize();
}

void
Y_yaml_version(int argc)
{
  push_string(yaml_get_version_string());
}

void
Y_yaml_open(int argc)
{
  const char* filename;
  const char* mode;

  if (argc < 1 || argc > 2) {
    y_error("expecting one or two arguments");
  }
  filename = ygets_q(argc - 1); /* FIXME: parse tilde */
  if (argc >= 2) {
    mode = ygets_q(argc - 2);
  } else {
    mode = "r";
  }
  if (mode[0] == 'r' && mode[1] == '\0') {
    /* Create a parser. */
    parser_t* obj = push_parser();
    obj->input = fopen(filename, "r");
    if (obj->input == NULL) {
      y_error("failed to open file for reading");
    }
    if (! yaml_parser_initialize(&obj->parser)) {
      y_error("failed to initialize parser");
    }
    obj->init = TRUE;
    yaml_parser_set_input_file(&obj->parser, obj->input);
  } else if ((mode[0] == 'r' || mode [0] == 'a') && mode[1] == '\0') {
    /* Create an emitter. */
    emitter_t* obj = push_emitter();
    if (filename == NULL || filename[0] == '\0') {
      obj->output = stdout;
      obj->open = FALSE;
    } else {
      obj->output = fopen(filename, mode);
      if (obj->output == NULL) {
        y_error("failed to open file for writing");
      }
      obj->open = TRUE;
    }
    if (! yaml_emitter_initialize(&obj->emitter)) {
      y_error("failed to initialize emitter");
    }
    obj->init = TRUE;
    yaml_emitter_set_output_file(&obj->emitter, obj->output);
  } else {
    y_error("invalid file access mode");
  }
}

#define NIL_OK (1U << 0)
#define FRESH  (1U << 1)

static event_t*
get_event(int iarg, unsigned int flags)
{
  event_t* obj;

  if ((flags & NIL_OK) != 0 && yarg_nil(iarg)) {
    obj = push_event();
  } else {
    obj = (event_t*)yget_obj(iarg, &event_type);
    if ((flags & FRESH) != 0 && obj->init) {
      obj->init = FALSE;
      yaml_event_delete(&obj->event);
    }
  }
  return  obj;
}


void
Y_yaml_parse(int argc)
{
  parser_t* src = NULL;
  event_t* dst = NULL;

  if (argc < 1 || argc > 2) {
    y_error("expecting one or two arguments");
  }
  src = yget_obj(argc - 1, &parser_type);
  if (src->parsing == ANY) {
    src->parsing = PARSE;
  } else if (src->parsing != PARSE) {
    /*
     * An application must not alternate the calls of yaml_parser_scan() with
     * the calls of yaml_parser_parse() or yaml_parser_load(). Doing this will
     * break the parser.
     */
    y_error("not an event-based parser");
  }
  if (argc >= 2) {
    /* Re-use existing event. */
    dst = yget_obj(argc - 2, &event_type);
    if (dst->init) {
      dst->init = FALSE;
      yaml_event_delete(&dst->event);
    }
  } else {
    /* Create new event. */
    dst = push_event();
  }
  if (! yaml_parser_parse(&src->parser, &dst->event)) {
    y_error("parser error");
  }
  dst->init = TRUE;
}

void
Y_yaml_emit(int argc)
{
  emitter_t* dst = NULL;
  int iarg;

  if (argc < 1) {
    y_error("expecting at least one argument");
  }
  iarg = argc;
  dst = yget_obj(--iarg, &emitter_type);

  /*
   * Emit the event(s).  The emitter takes the responsibility for the event
   * object and destroys its content after it is emitted. The event object is
   * destroyed even if the function fails.
   */
  while (iarg > 0) {
    event_t* src = yget_obj(--iarg, &event_type);
    if (! src->init) {
      y_error("unintialized event");
    }
    src->init = FALSE; /* before calling yaml_emitter_emit() */
    if (! yaml_emitter_emit(&dst->emitter, &src->event)) {
      y_error("emitter error");
    }
  }

  /* Return nothing. */
  ypush_nil();
}

void
Y_yaml_stream_start_event(int argc)
{
  event_t* obj = NULL;
  yaml_encoding_t encoding = YAML_ANY_ENCODING;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == encoding_index) {
        encoding = ygets_i(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_stream_start_event_initialize(&obj->event, encoding)) {
    y_error("failed to initialize STREAM-START event");
  }
  obj->init = TRUE;
}

void
Y_yaml_stream_end_event(int argc)
{
  event_t* obj;

  if (argc != 1) {
    y_error("expecting exactly one argument");
  }
  obj = get_event(0, NIL_OK|FRESH);
  if (! yaml_stream_end_event_initialize(&obj->event)) {
    y_error("failed to initialize STREAM-END event");
  }
  obj->init = TRUE;
}

void
Y_yaml_document_start_event(int argc)
{
  event_t* obj = NULL;
  yaml_version_directive_t version_directive;
  yaml_version_directive_t* version = NULL;
  int implicit = TRUE;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == implicit_index) {
        implicit = yarg_true(--iarg);
      } else if (index == version_index) {
        --iarg;
        if (! yarg_nil(iarg)) {
          long major, minor;
          const char* str = ygets_q(iarg);
          if (str == NULL) {
          bad_version:
            y_error("bad version number");
          }
          str = parse_integer(str, &major, NO_SIGN);
          if (str == NULL || str[0] != '.') {
            goto bad_version;
          }
          str = parse_integer(str + 1, &minor, NO_SIGN);
          if (str == NULL || str[0] != '\0') {
            goto bad_version;
          }
          version_directive.major = major;
          version_directive.minor = minor;
          version = &version_directive;
        }
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_document_start_event_initialize(&obj->event, version,
                                             NULL, NULL, implicit)) {
    y_error("failed to initialize DOCUMENT-START event");
  }
  obj->init = TRUE;
}

void
Y_yaml_document_end_event(int argc)
{
  event_t* obj = NULL;
  int implicit = TRUE;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == implicit_index) {
        implicit = yarg_true(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_document_end_event_initialize(&obj->event, implicit)) {
    y_error("failed to initialize DOCUMENT-END event");
  }
  obj->init = TRUE;
}

void
Y_yaml_alias_event(int argc)
{
  event_t* obj = NULL;
  yaml_char_t* anchor = NULL;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == anchor_index) {
        anchor = (yaml_char_t*)ygets_q(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_alias_event_initialize(&obj->event, anchor)) {
    y_error("failed to initialize ALIAS event");
  }
  obj->init = TRUE;
}

void
Y_yaml_scalar_event(int argc)
{
  char buffer[64];
  event_t* obj = NULL;
  yaml_char_t* anchor = NULL;
  yaml_char_t* tag = NULL;
  yaml_char_t* value = NULL;
  int plain_implicit = TRUE;
  int quoted_implicit = TRUE;
  yaml_scalar_style_t style = YAML_ANY_SCALAR_STYLE;
  int iarg, drop = 0, number, length = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == anchor_index) {
        anchor = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == tag_index) {
        tag = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == value_index) {
        --iarg;
        number = yarg_number(iarg);
        if (number == 1) {
          /* Integer. */
          long val = ygets_l(iarg);
          sprintf(buffer, "%ld", val);
          value = (yaml_char_t*)buffer;
        } else if (number == 2) {
          /* Floating-point. */
          double val = ygets_d(iarg);
          sprintf(buffer, "%g", val);
          value = (yaml_char_t*)buffer;
        } else if (number == 3) {
          /* Complex. */
          double* arr = ygeta_z(iarg, NULL, NULL);
          sprintf(buffer, "%g %s %gim", arr[0], (arr[1] >= 0 ? "+" : "-"),
                  fabs(arr[1]));
          value = (yaml_char_t*)buffer;
        } else if (yarg_string(iarg)) {
          value = (yaml_char_t*)ygets_q(iarg);
        } else {
          y_error("value must be a string or a numerical scalar");
        }
      } else if (index == plain_implicit_index) {
        plain_implicit = yarg_true(--iarg);
      } else if (index == quoted_implicit_index) {
        quoted_implicit = yarg_true(--iarg);
      } else if (index == style_index) {
        style = ygets_i(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }
  if (value == NULL || value[0] == '\0') {
    length = 0;
  } else {
    size_t len = strlen((const char*)value);
    length = (int)len;
    if (length != len) {
      y_error("integer overflow");
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_scalar_event_initialize(&obj->event, anchor, tag, value, length,
                                     plain_implicit, quoted_implicit, style)) {
    y_error("failed to initialize SCALAR event");
  }
  obj->init = TRUE;
}

void
Y_yaml_sequence_start_event(int argc)
{
  event_t* obj = NULL;
  yaml_char_t* anchor = NULL;
  yaml_char_t* tag = NULL;
  int implicit = TRUE;
  yaml_sequence_style_t style = YAML_ANY_SEQUENCE_STYLE;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == anchor_index) {
        anchor = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == tag_index) {
        tag = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == style_index) {
        style = ygets_i(--iarg);
      } else if (index == implicit_index) {
        implicit = yarg_true(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_sequence_start_event_initialize(&obj->event, anchor, tag,
                                             implicit, style)) {
    y_error("failed to initialize SEQUENCE-START event");
  }
  obj->init = TRUE;
}

void
Y_yaml_sequence_end_event(int argc)
{
  event_t* obj;

  if (argc != 1) {
    y_error("expecting exactly one argument");
  }
  obj = get_event(0, NIL_OK|FRESH);
  if (! yaml_sequence_end_event_initialize(&obj->event)) {
    y_error("failed to initialize SEQUENCE-END event");
  }
  obj->init = TRUE;
}

void
Y_yaml_mapping_start_event(int argc)
{
  event_t* obj = NULL;
  yaml_char_t* anchor = NULL;
  yaml_char_t* tag = NULL;
  int implicit = TRUE;
  yaml_mapping_style_t style = YAML_ANY_MAPPING_STYLE;
  int iarg, drop = 0;

  if (! initialized) {
    initialize();
  }
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    long index = yarg_key(iarg);
    if (index < 0) {
      /* Positional argument. */
      if (obj == NULL) {
        obj = get_event(iarg, NIL_OK|FRESH);
        drop = iarg;
      } else {
        y_error("too many arguments");
      }
    } else {
      /* Keyword argument. */
      if (index == anchor_index) {
        anchor = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == tag_index) {
        tag = (yaml_char_t*)ygets_q(--iarg);
      } else if (index == style_index) {
        style = ygets_i(--iarg);
      } else if (index == implicit_index) {
        implicit = yarg_true(--iarg);
      } else {
        y_error("unknown keyword");
      }
    }
  }

  /* Manage to push/let the result on top of the stack. */
  if (obj == NULL) {
    obj = push_event();
  } else if (drop > 0) {
    yarg_drop(drop);
  }
  if (! yaml_mapping_start_event_initialize(&obj->event, anchor, tag,
                                             implicit, style)) {
    y_error("failed to initialize MAPPING-START event");
  }
  obj->init = TRUE;
}

void
Y_yaml_mapping_end_event(int argc)
{
  event_t* obj;

  if (argc != 1) {
    y_error("expecting exactly one argument");
  }
  obj = get_event(0, NIL_OK|FRESH);
  if (! yaml_mapping_end_event_initialize(&obj->event)) {
    y_error("failed to initialize MAPPING-END event");
  }
  obj->init = TRUE;
}
