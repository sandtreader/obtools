//==========================================================================
// ObTools::SOAP: fault.cc
//
// Implementation of SOAP Fault message
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//--------------------------------------------------------------------------
// Constructor for outgoing faults
// Reason is the English (xml:lang="en") version - use add_reason for more
Fault::Fault(Code code, const string& reason): Message()
{
  XML::Element& fault = add_body(new XML::Element("env:Fault"));
  string c = "UNKNOWN!";

  switch (code)
  {
    case CODE_VERSION_MISMATCH:
      c = "env:VersionMismatch";
      break;

    case CODE_MUST_UNDERSTAND:
      c = "env:MustUnderstand";
      break;

    case CODE_DATA_ENCODING_UNKNOWN:
      c = "env:DataEncodingUnknown";
      break;

    case CODE_SENDER:
      c = "env:Sender";
      break;

    case CODE_RECEIVER:
      c = "env:Receiver";
      break;

    default:;
  }

  fault.add("env:Code").add("env:Value", c);

  fault.add("env:Reason", "xmlns:xml", "http://www.w3.org/XML/1998/namespace")
    .add("env:Text", "xml:lang", "en", reason);
}

//--------------------------------------------------------------------------
// Set a subcode
// According to SOAP 1.2: 5.4.1.3, the value should be a qualified name
// We only allow one level here!
void Fault::set_subcode(const string& value)
{
  get_body().make_child("env:Code").make_child("env:Subcode")
    .add("env:Value", value);
}

//--------------------------------------------------------------------------
// Add a reason
// Use this for non-English reasons - pass the English reason in the
// constructor, above
void Fault::add_reason(const string& text, const string& lang)
{
  get_body().make_child("env:Reason").add("env:Text", "xml:lang", lang, text);
}

//--------------------------------------------------------------------------
// Set the Node value
// According to SOAP 1.2: 5.4.3, this should be a URI identifying the node
// There should only be one (but this routine doesn't check this)
void Fault::set_node(const string& uri)
{
  get_body().add("env:Node", uri);
}

//--------------------------------------------------------------------------
// Set the Role value
// According to SOAP 1.2: 5.4.4, this should be a URI identifying the role
// the node was operating in when the fault occurred
// There should only be one (but this routine doesn't check this)
void Fault::set_role(const string& uri)
{
  get_body().add("env:Role", uri);
}

//--------------------------------------------------------------------------
// Add a detail entry
// Detail entries can be more or less anything
void Fault::add_detail(XML::Element *detail)
{
  get_body().make_child("env:Detail").add(detail);
}

//--------------------------------------------------------------------------
// Get code string from incoming fault
// Returns empty string if no code found
string Fault::get_code_string() const
{
  return get_body("env:Fault").get_child("env:Code").get_child("env:Value").
    get_content();
}

//--------------------------------------------------------------------------
// Get code from incoming fault
Fault::Code Fault::get_code() const
{
  string cs = get_code_string();
  if (cs == "env:VersionMismatch")
    return CODE_VERSION_MISMATCH;
  else if (cs == "env::MustUnderstand")
    return CODE_MUST_UNDERSTAND;
  else if (cs == "env:DataEncodingUnknown")
    return CODE_DATA_ENCODING_UNKNOWN;
  else if (cs == "env:Sender")
    return CODE_SENDER;
  else if (cs == "env:Receiver")
    return CODE_RECEIVER;
  else
    return CODE_UNKNOWN;
}

//--------------------------------------------------------------------------
// Get reason from incoming fault with language code given
string Fault::get_reason(const string& lang) const
{
  XML::Element& re = get_body("env:Fault").get_child("env:Reason");
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(te, re, "env:Text")
    if (te["xml:lang"] == lang) return te.get_content();
  OBTOOLS_XML_ENDFOR

  return "";
}

//==========================================================================
// SOAP VersionMismatch Fault Message
// Adds recommended headers (SOAP1.2: 5.4.7), indicating support for
// ONLY SOAP1.2

//--------------------------------------------------------------------------
// Constructor for outgoing faults
VersionMismatchFault::VersionMismatchFault():
  Fault(CODE_VERSION_MISMATCH, "Version Mismatch")
{
  add_header("env:Upgrade")
    .add("env:SupportedEnvelope", "qname", "ns1:Envelope")
    .set_attr("xmlns:ns1", NS_ENVELOPE_1_2);
}

//==========================================================================
// SOAP MustUnderstand Fault Message
// Adds recommended headers (SOAP1.2: 5.4.8), indicating non-understood
// elements

//--------------------------------------------------------------------------
// Constructor for outgoing faults
MustUnderstandFault::MustUnderstandFault():
  Fault(CODE_MUST_UNDERSTAND, "Mandatory header blocks not understood")
{
}

//--------------------------------------------------------------------------
// Add a NotUnderstood block
// attr/value indicate a namespace
void MustUnderstandFault::add_not_understood(const string& qname,
                                             const string& attr,
                                             const string& value)
{
  XML::Element& nu = add_header("env:NotUnderstood");
  nu.set_attr("qname", qname);
  nu.set_attr(attr, value);
}


}} // namespaces
