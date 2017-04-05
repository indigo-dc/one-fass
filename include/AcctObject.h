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

#ifndef ACCT_OBJECT_H_
#define ACCT_OBJECT_H_

#include <sstream>
#include <string>
#include <vector>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <map>

#include "ObjectXML.h"

using namespace std;

class AcctObject: protected ObjectXML
{
public:

    // constructs an object using a XML node, 
    // the node is copied to the new object
    AcctObject(const xmlNodePtr node):
                         ObjectXML() {
        init(node);
        init_attributes();
    };

    ~AcctObject(){};

/*
    const string dump_node(){

        string xml_string = dump(xml);
    
        // remove annoying header
        size_t pos = xml_string.find("<VM>");
        xml_string.erase(0,pos);
        pos = xml_string.find("</VM>"); 
        xml_string.erase(pos+5);
        return xml_string;
    } 
*/

    // get methods 
    int get_oid() const
    {
        return oid;
    };

    int get_uid() const
    {
        return uid;
    };


private:

    // object attributes
    int   oid;
    int   uid;
    int   vmid;
    int   memory; 
    float cpu;
    long int start_time; 
    long int stop_time; 

    void init_attributes();

};

#endif 

