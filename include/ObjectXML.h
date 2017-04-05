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

#ifndef OBJECT_XML_H_
#define OBJECT_XML_H_

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <map>

#include "XMLRPCClient.h"


using namespace std;

class ObjectXML
{
public:

    ObjectXML() {
        xml = 0;
        ctx = 0;
    };

    ~ObjectXML() {

        if (ctx != 0) {
            xmlXPathFreeContext(ctx);
        }

        if (xml != 0) {
            xmlFreeDoc(xml);
        }
        xmlCleanupParser();
    };

    const string dump(xmlDocPtr xml_doc){
        xmlChar *s;
        int size;

        xmlDocDumpMemory((xmlDocPtr)xml_doc, &s, &size);

        string xml_string = (char *)s;
        xmlFree(s);

        return xml_string;
        }        

    void init(const xmlNodePtr node);

protected:

    // parse the XML response from ONE into a xmlXPathContextPtr
    bool xml_parse(const string &xml_doc);  

    // get XML nodes corresponding to objects
    int get_nodes(const string& xpath_expr, std::vector<xmlNodePtr>& content);

    // clears XMl node pointers
    void free_nodes(std::vector<xmlNodePtr>& content) const {

        std::vector<xmlNodePtr>::iterator it;

        for (it = content.begin(); it < content.end(); it++) {
            xmlFreeNode(*it);
        }
    };


    // gets elements by xpath:
    //     param values vector with the element values
    //     param expr of the xml element
    template<typename T>
    void xpaths(std::vector<T>& values, const char * expr)
    {
        xmlXPathObjectPtr obj;

        xmlNodePtr cur;
        xmlChar *  str_ptr;

        obj=xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(expr),ctx);

        if (obj == 0)
        {
            return;
        }

        switch (obj->type)
        {
            case XPATH_NUMBER:
                values.push_back(static_cast<T>(obj->floatval));
                break;

            case XPATH_NODESET:
                for(int i = 0; i < obj->nodesetval->nodeNr ; ++i)
                {
                    cur = obj->nodesetval->nodeTab[i];

                    if ( cur == 0 || cur->type != XML_ELEMENT_NODE )
                    {
                        continue;
                    }

                    str_ptr = xmlNodeGetContent(cur);

                    if (str_ptr != 0)
                    {
                        std::istringstream iss(reinterpret_cast<char *>(str_ptr));
                        T val;

                        iss >> std::dec >> val;

                        if (!iss.fail())
                        {
                            values.push_back(val);
                        }

                        xmlFree(str_ptr);
                    }
                }
                break;

            default:
                break;

        }

        xmlXPathFreeObject(obj);
    };

    void xpaths(std::vector<std::string>& values, const char * xpath_expr);

    // gets a xpath attribute, if the attribute is not found a default is used:
    // (this function only returns the first element)
    //    param value of the element
    //    param xpath_expr of the xml element
    //    param def default value if the element is not found
    //    return -1 if default was set
    template<typename T>
    int xpath(T& value, const char * xpath_expr, const T& def)
    {
        std::vector<std::string> values;

        xpaths(values, xpath_expr);

        if (values.empty() == true)
        {
            value = def;
            return -1;
        }

        std::istringstream iss(values[0]);

        iss >> std::dec >> value;

        if (iss.fail() == true)
        {
            value = def;
            return -1;
        }

        return 0;
    }

    int xpath(std::string& value, const char * xpath_expr, const char * def);

    // class variables
    xmlXPathContextPtr ctx; // XML xpath context pointer
    xmlDocPtr xml;

};


#endif
