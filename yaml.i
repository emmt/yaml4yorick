/*
 * yaml.i --
 *
 * Interface to YAML files.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2018: Éric Thiébaut <eric.thiebaut@univ-lyon1.fr>
 *
 * See LICENSE.md for details.
 *
 */

if (is_func(plug_in)) plug_in, "yaml";

func yaml_print(filename)
{
  true = 1n;
  false = 0n;
  parser = yaml_parser(filename);
  done = false;
  while (! done) {
    event = yaml_next_event(parser);
    type = event.type;
    if (type == YAML_NO_EVENT) {
      str = "No event!";
    } else if (type == YAML_STREAM_START_EVENT) {
      str = "<b>Start Stream</b>";
    } else if (type == YAML_STREAM_END_EVENT) {
      str = "<b>End Stream</b>";
      done = true;
    } else if (type == YAML_DOCUMENT_START_EVENT) {
      str = "<b>Start Document</b>";
    } else if (type == YAML_DOCUMENT_END_EVENT) {
      str = "<b>End Document</b>";
    } else if (type == YAML_SEQUENCE_START_EVENT) {
      str = "<b>Start Sequence</b>";
    } else if (type == YAML_SEQUENCE_END_EVENT) {
      str = "<b>End Sequence</b>";
    } else if (type == YAML_MAPPING_START_EVENT) {
      str = "<b>Start Mapping</b>";
    } else if (type == YAML_MAPPING_END_EVENT) {
      str = "<b>End Mapping</b>";
    } else if (type == YAML_ALIAS_EVENT) {
      str = swrite(format="Got alias (anchor %s)", event.anchor);
    } else if (type == YAML_SCALAR_EVENT) {
      str = swrite(format="Got scalar (value %s)", event.value);
    } else  {
      str = "<b>Unknown Event</b>";
    }
    write, format="%s\n", str;
  }
}

/*extern yaml_event;*/

extern yaml_debug;
extern yaml_parser;
/* DOCUMENT parser = yaml_parser(filename);

   SEE ALSO:
 */

extern yaml_next_event;
/* DOCUMENT event = yaml_next_event(parser);
         or event = yaml_next_event(parser, event);

     Yields next YAML event from parser PARSER.  The returned value is
     an instance of YAML event.  Second argument may be an existing
     YAML event instance which is reused (and returned).

   SEE ALSO:
 */

/*---------------------------------------------------------------------------*/
/* ENUM TYPES */

local YAML_ANY_ENCODING, YAML_UTF8_ENCODING, YAML_UTF16LE_ENCODING, YAML_UTF16BE_ENCODING;
/* DOCUMENT YAML stream encoding.

     YAML_ANY_ENCODING:     Let the parser choose the encoding.
     YAML_UTF8_ENCODING:    The default UTF-8 encoding.
     YAML_UTF16LE_ENCODING: The UTF-16-LE encoding with BOM.
     YAML_UTF16BE_ENCODING: The UTF-16-BE encoding with BOM.

   SEE ALSO:
*/

local YAML_ANY_BREAK, YAML_CR_BREAK, YAML_LN_BREAK, YAML_CRLN_BREAK;
/* DOCUMENT YAML line break types.

     YAML_ANY_BREAK:  Let the parser choose the break type.
     YAML_CR_BREAK:   Use CR for line breaks (Mac style).
     YAML_LN_BREAK:   Use LN for line breaks (Unix style).
     YAML_CRLN_BREAK: Use CR LN for line breaks (DOS style).

   SEE ALSO:
*/

local YAML_NO_ERROR, YAML_MEMORY_ERROR, YAML_READER_ERROR, YAML_SCANNER_ERROR, YAML_PARSER_ERROR, YAML_COMPOSER_ERROR, YAML_WRITER_ERROR, YAML_EMITTER_ERROR;
/* DOCUMENT YAML error codes.

     YAML_NO_ERROR:       No error is produced.
     YAML_MEMORY_ERROR:   Cannot allocate or reallocate a block of memory.
     YAML_READER_ERROR:   Cannot read or decode the input stream.
     YAML_SCANNER_ERROR:  Cannot scan the input stream.
     YAML_PARSER_ERROR:   Cannot parse the input stream.
     YAML_COMPOSER_ERROR: Cannot compose a YAML document.
     YAML_WRITER_ERROR:   Cannot write to the output stream.
     YAML_EMITTER_ERROR:  Cannot emit a YAML stream.

   SEE ALSO:
*/

local YAML_ANY_SCALAR_STYLE, YAML_PLAIN_SCALAR_STYLE, YAML_SINGLE_QUOTED_SCALAR_STYLE, YAML_DOUBLE_QUOTED_SCALAR_STYLE, YAML_LITERAL_SCALAR_STYLE, YAML_FOLDED_SCALAR_STYLE;
/* DOCUMENT YAML scalar styles.

    YAML_ANY_SCALAR_STYLE:    Let the emitter choose the style.
    YAML_PLAIN_SCALAR_STYLE:    The plain scalar style.
    YAML_SINGLE_QUOTED_SCALAR_STYLE:    The single-quoted scalar style.
    YAML_DOUBLE_QUOTED_SCALAR_STYLE:    The double-quoted scalar style.
    YAML_LITERAL_SCALAR_STYLE:    The literal scalar style.
    YAML_FOLDED_SCALAR_STYLE:    The folded scalar style.

   SEE ALSO:
*/

local YAML_ANY_SEQUENCE_STYLE, YAML_BLOCK_SEQUENCE_STYLE, YAML_FLOW_SEQUENCE_STYLE;
/* DOCUMENT YAML sequence styles.

    YAML_ANY_SEQUENCE_STYLE:    Let the emitter choose the style.
    YAML_BLOCK_SEQUENCE_STYLE:    The block sequence style.
    YAML_FLOW_SEQUENCE_STYLE:    The flow sequence style.

   SEE ALSO:
*/

local YAML_ANY_MAPPING_STYLE, YAML_BLOCK_MAPPING_STYLE, YAML_FLOW_MAPPING_STYLE;
/* DOCUMENT YAML mapping styles.

    YAML_ANY_MAPPING_STYLE:    Let the emitter choose the style.
    YAML_BLOCK_MAPPING_STYLE:    The block mapping style.
    YAML_FLOW_MAPPING_STYLE:    The flow mapping style.

   SEE ALSO:
*/

local YAML_NO_TOKEN, YAML_STREAM_START_TOKEN, YAML_STREAM_END_TOKEN, YAML_VERSION_DIRECTIVE_TOKEN, YAML_TAG_DIRECTIVE_TOKEN, YAML_DOCUMENT_START_TOKEN, YAML_DOCUMENT_END_TOKEN, YAML_BLOCK_SEQUENCE_START_TOKEN, YAML_BLOCK_MAPPING_START_TOKEN, YAML_BLOCK_END_TOKEN, YAML_FLOW_SEQUENCE_START_TOKEN, YAML_FLOW_SEQUENCE_END_TOKEN, YAML_FLOW_MAPPING_START_TOKEN, YAML_FLOW_MAPPING_END_TOKEN, YAML_BLOCK_ENTRY_TOKEN, YAML_FLOW_ENTRY_TOKEN, YAML_KEY_TOKEN, YAML_VALUE_TOKEN, YAML_ALIAS_TOKEN, YAML_ANCHOR_TOKEN, YAML_TAG_TOKEN, YAML_SCALAR_TOKEN;
/* DOCUMENT YAML token types.

     YAML_NO_TOKEN:                   An empty token.
     YAML_STREAM_START_TOKEN:         A STREAM-START token.
     YAML_STREAM_END_TOKEN:           A STREAM-END token.
     YAML_VERSION_DIRECTIVE_TOKEN:    A VERSION-DIRECTIVE token.
     YAML_TAG_DIRECTIVE_TOKEN:        A TAG-DIRECTIVE token.
     YAML_DOCUMENT_START_TOKEN:       A DOCUMENT-START token.
     YAML_DOCUMENT_END_TOKEN:         A DOCUMENT-END token.
     YAML_BLOCK_SEQUENCE_START_TOKEN: A BLOCK-SEQUENCE-START token.
     YAML_BLOCK_MAPPING_START_TOKEN:  A BLOCK-SEQUENCE-END token.
     YAML_BLOCK_END_TOKEN:            A BLOCK-END token.
     YAML_FLOW_SEQUENCE_START_TOKEN:  A FLOW-SEQUENCE-START token.
     YAML_FLOW_SEQUENCE_END_TOKEN:    A FLOW-SEQUENCE-END token.
     YAML_FLOW_MAPPING_START_TOKEN:   A FLOW-MAPPING-START token.
     YAML_FLOW_MAPPING_END_TOKEN:     A FLOW-MAPPING-END token.
     YAML_BLOCK_ENTRY_TOKEN:          A BLOCK-ENTRY token.
     YAML_FLOW_ENTRY_TOKEN:           A FLOW-ENTRY token.
     YAML_KEY_TOKEN:                  A KEY token.
     YAML_VALUE_TOKEN:                A VALUE token.
     YAML_ALIAS_TOKEN:                An ALIAS token.
     YAML_ANCHOR_TOKEN:               An ANCHOR token.
     YAML_TAG_TOKEN:                  A TAG token.
     YAML_SCALAR_TOKEN:               A SCALAR token.

   SEE ALSO:
*/

local YAML_NO_EVENT, YAML_STREAM_START_EVENT, YAML_STREAM_END_EVENT, YAML_DOCUMENT_START_EVENT, YAML_DOCUMENT_END_EVENT, YAML_ALIAS_EVENT, YAML_SCALAR_EVENT, YAML_SEQUENCE_START_EVENT, YAML_SEQUENCE_END_EVENT, YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT;
/* DOCUMENT YAML event types.

     YAML_NO_EVENT:             An empty event.
     YAML_STREAM_START_EVENT:   A STREAM-START event.
     YAML_STREAM_END_EVENT:     A STREAM-END event.
     YAML_DOCUMENT_START_EVENT: A DOCUMENT-START event.
     YAML_DOCUMENT_END_EVENT:   A DOCUMENT-END event.
     YAML_ALIAS_EVENT:          An ALIAS event.
     YAML_SCALAR_EVENT:         A SCALAR event.
     YAML_SEQUENCE_START_EVENT: A SEQUENCE-START event.
     YAML_SEQUENCE_END_EVENT:   A SEQUENCE-END event.
     YAML_MAPPING_START_EVENT:  A MAPPING-START event.
     YAML_MAPPING_END_EVENT:    A MAPPING-END event.

   SEE ALSO:
*/

local YAML_NO_NODE, YAML_SCALAR_NODE, YAML_SEQUENCE_NODE, YAML_MAPPING_NODE;
/* DOCUMENT YAML node types.

     YAML_NO_NODE:       An empty node.
     YAML_SCALAR_NODE:   A scalar node.
     YAML_SEQUENCE_NODE: A sequence node.
     YAML_MAPPING_NODE:  A mapping node.

   SEE ALSO:
*/

local YAML_PARSE_STREAM_START_STATE, YAML_PARSE_IMPLICIT_DOCUMENT_START_STATE, YAML_PARSE_DOCUMENT_START_STATE, YAML_PARSE_DOCUMENT_CONTENT_STATE, YAML_PARSE_DOCUMENT_END_STATE, YAML_PARSE_BLOCK_NODE_STATE, YAML_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE, YAML_PARSE_FLOW_NODE_STATE, YAML_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE, YAML_PARSE_BLOCK_SEQUENCE_ENTRY_STATE, YAML_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE, YAML_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE, YAML_PARSE_BLOCK_MAPPING_KEY_STATE, YAML_PARSE_BLOCK_MAPPING_VALUE_STATE, YAML_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE, YAML_PARSE_FLOW_SEQUENCE_ENTRY_STATE, YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE, YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE, YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE, YAML_PARSE_FLOW_MAPPING_FIRST_KEY_STATE, YAML_PARSE_FLOW_MAPPING_KEY_STATE, YAML_PARSE_FLOW_MAPPING_VALUE_STATE, YAML_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE, YAML_PARSE_END_STATE;
/* DOCUMENT YAML parser states.

     YAML_PARSE_STREAM_START_STATE:                      Expect STREAM-START.
     YAML_PARSE_IMPLICIT_DOCUMENT_START_STATE:           Expect the beginning of an implicit document.
     YAML_PARSE_DOCUMENT_START_STATE:                    Expect DOCUMENT-START.
     YAML_PARSE_DOCUMENT_CONTENT_STATE:                  Expect the content of a document.
     YAML_PARSE_DOCUMENT_END_STATE:                      Expect DOCUMENT-END.
     YAML_PARSE_BLOCK_NODE_STATE:                        Expect a block node.
     YAML_PARSE_BLOCK_NODE_OR_INDENTLESS_SEQUENCE_STATE: Expect a block node or indentless sequence.
     YAML_PARSE_FLOW_NODE_STATE:                         Expect a flow node.
     YAML_PARSE_BLOCK_SEQUENCE_FIRST_ENTRY_STATE:        Expect the first entry of a block sequence.
     YAML_PARSE_BLOCK_SEQUENCE_ENTRY_STATE:              Expect an entry of a block sequence.
     YAML_PARSE_INDENTLESS_SEQUENCE_ENTRY_STATE:         Expect an entry of an indentless sequence.
     YAML_PARSE_BLOCK_MAPPING_FIRST_KEY_STATE:           Expect the first key of a block mapping.
     YAML_PARSE_BLOCK_MAPPING_KEY_STATE:                 Expect a block mapping key.
     YAML_PARSE_BLOCK_MAPPING_VALUE_STATE:               Expect a block mapping value.
     YAML_PARSE_FLOW_SEQUENCE_FIRST_ENTRY_STATE:         Expect the first entry of a flow sequence.
     YAML_PARSE_FLOW_SEQUENCE_ENTRY_STATE:               Expect an entry of a flow sequence.
     YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_KEY_STATE:   Expect a key of an ordered mapping.
     YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_VALUE_STATE: Expect a value of an ordered mapping.
     YAML_PARSE_FLOW_SEQUENCE_ENTRY_MAPPING_END_STATE:   Expect the and of an ordered mapping entry.
     YAML_PARSE_FLOW_MAPPING_FIRST_KEY_STATE:            Expect the first key of a flow mapping.
     YAML_PARSE_FLOW_MAPPING_KEY_STATE:                  Expect a key of a flow mapping.
     YAML_PARSE_FLOW_MAPPING_VALUE_STATE:                Expect a value of a flow mapping.
     YAML_PARSE_FLOW_MAPPING_EMPTY_VALUE_STATE:          Expect an empty value of a flow mapping.
     YAML_PARSE_END_STATE:                               Expect nothing.

   SEE ALSO:
*/

local YAML_EMIT_STREAM_START_STATE, YAML_EMIT_FIRST_DOCUMENT_START_STATE, YAML_EMIT_DOCUMENT_START_STATE, YAML_EMIT_DOCUMENT_CONTENT_STATE, YAML_EMIT_DOCUMENT_END_STATE, YAML_EMIT_FLOW_SEQUENCE_FIRST_ITEM_STATE, YAML_EMIT_FLOW_SEQUENCE_ITEM_STATE, YAML_EMIT_FLOW_MAPPING_FIRST_KEY_STATE, YAML_EMIT_FLOW_MAPPING_KEY_STATE, YAML_EMIT_FLOW_MAPPING_SIMPLE_VALUE_STATE, YAML_EMIT_FLOW_MAPPING_VALUE_STATE, YAML_EMIT_BLOCK_SEQUENCE_FIRST_ITEM_STATE, YAML_EMIT_BLOCK_SEQUENCE_ITEM_STATE, YAML_EMIT_BLOCK_MAPPING_FIRST_KEY_STATE, YAML_EMIT_BLOCK_MAPPING_KEY_STATE, YAML_EMIT_BLOCK_MAPPING_SIMPLE_VALUE_STATE, YAML_EMIT_BLOCK_MAPPING_VALUE_STATE, YAML_EMIT_END_STATE;
/* DOCUMENT YAML emitter states.

     YAML_EMIT_STREAM_START_STATE:               Expect STREAM-START.
     YAML_EMIT_FIRST_DOCUMENT_START_STATE:       Expect the first DOCUMENT-START or STREAM-END.
     YAML_EMIT_DOCUMENT_START_STATE:             Expect DOCUMENT-START or STREAM-END.
     YAML_EMIT_DOCUMENT_CONTENT_STATE:           Expect the content of a document.
     YAML_EMIT_DOCUMENT_END_STATE:               Expect DOCUMENT-END.
     YAML_EMIT_FLOW_SEQUENCE_FIRST_ITEM_STATE:   Expect the first item of a flow sequence.
     YAML_EMIT_FLOW_SEQUENCE_ITEM_STATE:         Expect an item of a flow sequence.
     YAML_EMIT_FLOW_MAPPING_FIRST_KEY_STATE:     Expect the first key of a flow mapping.
     YAML_EMIT_FLOW_MAPPING_KEY_STATE:           Expect a key of a flow mapping.
     YAML_EMIT_FLOW_MAPPING_SIMPLE_VALUE_STATE:  Expect a value for a simple key of a flow mapping.
     YAML_EMIT_FLOW_MAPPING_VALUE_STATE:         Expect a value of a flow mapping.
     YAML_EMIT_BLOCK_SEQUENCE_FIRST_ITEM_STATE:  Expect the first item of a block sequence.
     YAML_EMIT_BLOCK_SEQUENCE_ITEM_STATE:        Expect an item of a block sequence.
     YAML_EMIT_BLOCK_MAPPING_FIRST_KEY_STATE:    Expect the first key of a block mapping.
     YAML_EMIT_BLOCK_MAPPING_KEY_STATE:          Expect the key of a block mapping.
     YAML_EMIT_BLOCK_MAPPING_SIMPLE_VALUE_STATE: Expect a value for a simple key of a block mapping.
     YAML_EMIT_BLOCK_MAPPING_VALUE_STATE:        Expect a value of a block mapping.
     YAML_EMIT_END_STATE:                        Expect nothing.

   SEE ALSO:
*/

/*---------------------------------------------------------------------------*/
extern __yaml_init__;
__yaml_init__;
