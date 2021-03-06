/* $Id$ */

/* Please see the LICENSE file for copyright and distribution information */

#include "ruby_libxml.h"
#include "ruby_xml_sax_parser.h"

/*
 * Document-class: LibXML::XML::SaxParser
 *
 * XML::SaxParser provides a callback based API for parsing documents,
 * in contrast to XML::Parser's tree based API and XML::Reader's stream
 * based API.
 *
 * The XML::SaxParser API is fairly complex, not well standardized,
 * and does not directly support validation making entity, namespace and
 * base processing relatively hard.
 *
 * To use the XML::SaxParser, register a callback class via the
 * XML::SaxParser#callbacks=.  It is easiest to include the
 * XML::SaxParser::Callbacks module in your class and override
 * the methods as needed.
 *
 * Basic example:
 *
 *   class MyCallbacks
 *     include XML::SaxParser::Callbacks
 *     def on_start_element(element, attributes)
 *       puts #Element started: #{element}"
 *     end
 *   end
 *
 *   parser = XML::SaxParser.string(my_string)
 *   parser.callbacks = MyCallbacks.new
 *   parser.parse
 *
 * You can also parse strings (see XML::SaxParser.string) and
 * io objects (see XML::SaxParser.io).
 */

VALUE cXMLSaxParser;
static ID CALLBACKS_ATTR;
static ID CONTEXT_ATTR;


/* ======  Parser  =========== */

/*static int rxml_sax_parser_parse_io(VALUE self, VALUE input)
{
  VALUE handler = rb_ivar_get(self, CALLBACKS_ATTR);
  VALUE io = rb_ivar_get(input, IO_ATTR);
  VALUE encoding = rb_ivar_get(input, ENCODING_ATTR);
  xmlCharEncoding xmlEncoding = NUM2INT(encoding);
  xmlParserCtxtPtr ctxt =
      xmlCreateIOParserCtxt((xmlSAXHandlerPtr) &rxml_sax_handler,
          (void *) handler, (xmlInputReadCallback) rxml_read_callback, NULL,
          (void *) io, xmlEncoding);
  return xmlParseDocument(ctxt);
}*/


/*
 * call-seq:
 *    parser.initialize(context) -> XML::Parser
 *
 * Creates a new XML::Parser from the specified 
 * XML::Parser::Context.
 */
static VALUE rxml_sax_parser_initialize(int argc, VALUE *argv, VALUE self)
{

  xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt((xmlSAXHandlerPtr)&xmlDefaultSAXHandler,
                                                  NULL,
                                                  "",
                                                  0,
                                                  "input.xml");
  VALUE context = Data_Wrap_Struct(cXMLParserContext, NULL, xmlFreeParserCtxt, ctxt);
  rb_ivar_set(self, CONTEXT_ATTR, context);
  return self;
}

/*
 * call-seq:
 *    parser.parse -> (true|false)
 *
 * Parse the input XML, generating callbacks to the object
 * registered via the +callbacks+ attributesibute.
 */
static VALUE rxml_sax_parser_parse(VALUE self, VALUE chunk)
{
  int status;
  VALUE context = rb_ivar_get(self, CONTEXT_ATTR);
  xmlParserCtxtPtr ctxt;
  Data_Get_Struct(context, xmlParserCtxt, ctxt);

  ctxt->sax2 = 1;
  ctxt->userData = (void*)rb_ivar_get(self, CALLBACKS_ATTR);

  if (ctxt->sax != (xmlSAXHandlerPtr) &xmlDefaultSAXHandler)
    xmlFree(ctxt->sax);
    
  ctxt->sax = (xmlSAXHandlerPtr)&rxml_sax_handler;
    
  status = xmlParseChunk(ctxt, RSTRING_PTR(chunk), RSTRING_LEN(chunk), 0);

  /* IMPORTANT - null the handle to our sax handler
     so libxml doesn't try to free it.*/
  ctxt->sax = NULL;
  
  /* Now check the parsing result*/
  if (status == -1 || !ctxt->wellFormed)
  {
    if (ctxt->myDoc)
      xmlFreeDoc(ctxt->myDoc);

    rxml_raise(&ctxt->lastError);
  }
  return Qtrue;
}

// Rdoc needs to know
#ifdef RDOC_NEVER_DEFINED
mLibXML = rb_define_module("LibXML");
mXML = rb_define_module_under(mLibXML, "XML");
#endif

void ruby_init_xml_sax_parser(void)
{
  /* SaxParser */
  cXMLSaxParser = rb_define_class_under(mXML, "SaxParser", rb_cObject);

  /* Atributes */
  CALLBACKS_ATTR = rb_intern("@callbacks");
  CONTEXT_ATTR = rb_intern("@context");
  rb_define_attr(cXMLSaxParser, "callbacks", 1, 1);

  /* Instance Methods */
  rb_define_method(cXMLSaxParser, "initialize", rxml_sax_parser_initialize, -1);
  rb_define_method(cXMLSaxParser, "parse", rxml_sax_parser_parse, 1);
}
