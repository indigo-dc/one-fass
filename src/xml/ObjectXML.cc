/**
 * Copyright © 2017 INFN Torino - INDIGO-DataCloud
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ObjectXML.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <assert.h>
#include <stdexcept>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>

void ObjectXML::init(const xmlNodePtr node) {
    // constructs the object
    xml = xmlNewDoc(reinterpret_cast<const xmlChar *>("1.0"));

    if (xml == 0) {
        throw("Error allocating XML Document");
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0) {
        xmlFreeDoc(xml);
        throw("Unable to create new XPath context");
    }

    xmlNodePtr root_node = xmlDocCopyNode(node, xml, 1);

    if (root_node == 0) {
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(xml);
        throw("Unable to allocate node");
    }

    xmlDocSetRootElement(xml, root_node);

}

bool ObjectXML::xml_parse(const string &xml_doc) {
    // copied from ONE ObjectXML
    // xmlDocPtr xml = xmlReadMemory(xml_doc.c_str(), xml_doc.length(),
    //xmlInitParser();
    xml = xmlReadMemory(xml_doc.c_str(), xml_doc.length(),
                                  0, 0, XML_PARSE_HUGE);
    if (xml == 0) {
        throw runtime_error("Error parsing XML Document");
        return false;
    }

   
    ctx = xmlXPathNewContext(xml);

    if (ctx == 0) {
        xmlFreeDoc(xml);
        throw runtime_error("Unable to create new XPath context");
        return false;
    }
    
    //xmlCleanupParser();
    return true;
}

int ObjectXML::get_nodes(const string& xpath_expr,
                      std::vector<xmlNodePtr>& content) {
    // copied from ONE
    xmlXPathObjectPtr obj;

    obj = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(xpath_expr.c_str()), ctx);

        if (obj == 0) {
        return 0;
    }

    if (obj->nodesetval == 0) {
        xmlXPathFreeObject(obj);
        return 0;
    }

    xmlNodeSetPtr ns = obj->nodesetval;
    int           size = ns->nodeNr;
    int           num_nodes = 0;
    xmlNodePtr    cur;

    for (int i = 0; i < size; ++i) {
        cur = xmlCopyNode(ns->nodeTab[i], 1);

        if ( cur == 0 || cur->type != XML_ELEMENT_NODE ) {
            xmlFreeNode(cur);
            continue;
        }

        content.push_back(cur);
        num_nodes++;
    }

    xmlXPathFreeObject(obj);

    return num_nodes;
}

void ObjectXML::xpaths(std::vector<std::string>& content,
                            const char * expr) {
    xmlXPathObjectPtr obj;

    std::ostringstream oss;
    xmlNodePtr    cur;
    xmlChar *     str_ptr;

    obj = xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(expr), ctx);

    if (obj == 0) {
        return;
    }

    switch (obj->type) {
        case XPATH_NUMBER:
            oss << obj->floatval;

            content.push_back(oss.str());
            break;

        case XPATH_NODESET:
            for (int i = 0; i < obj->nodesetval->nodeNr; ++i) {
                cur = obj->nodesetval->nodeTab[i];

                if ( cur == 0 || cur->type != XML_ELEMENT_NODE ) {
                    continue;
                }

                str_ptr = xmlNodeGetContent(cur);

                if (str_ptr != 0) {
                    std::string ncontent = reinterpret_cast<char *>(str_ptr);

                    content.push_back(ncontent);

                    xmlFree(str_ptr);
                }
            }
            break;

        case XPATH_UNDEFINED:
        case XPATH_BOOLEAN:
        case XPATH_STRING:
        case XPATH_POINT:
        case XPATH_RANGE:
        case XPATH_LOCATIONSET:
        case XPATH_USERS:
        case XPATH_XSLT_TREE:
            break;
    }
    xmlXPathFreeObject(obj);
}

int ObjectXML::xpath(string& value,
           const char * xpath_expr,
                  const char * def) {
    vector<string> values;
    int rc = 0;

    xpaths(values, xpath_expr);

    if ( values.empty() == true ) {
        value = def;
        rc    = -1;
    } else {
        value = values[0];
    }
    return rc;
}
