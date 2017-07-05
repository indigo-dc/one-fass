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

#ifndef VM_OBJECT_H_
#define VM_OBJECT_H_

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

class VMObject: protected ObjectXML
{
public:

    // constructs an object using a XML node, 
    // the node is copied to the new object
    VMObject(const xmlNodePtr node): ObjectXML() {
        init(node);
        init_attributes();
    };

    ~VMObject(){};

    const string dump_node(){

        string xml_string = dump(xml);
    
        // remove annoying header
        size_t pos = xml_string.find("<VM>");
        xml_string.erase(0,pos);
        pos = xml_string.find("</VM>"); 
        xml_string.erase(pos+5);
        return xml_string;
    } 


    // get methods 
    int get_oid() const
    {
        return oid;
    };

    int get_uid() const
    {
        return uid;
    };

    int get_gid() const
    {
        return gid;
    };

    int64_t get_start() const
    {
        return start;
    };

    int64_t get_birth() const
    {
        return birth;
    };

    int get_static_vm() const
    {
        return static_vm;
    };

    // float get_prio() const
    // {
    //    return prio;
    // };

    // returns VM usage requirements
    void get_requirements(int& cpu, int& memory); 
    void get_requirements(float& cpu, int& memory) {
        cpu = this->cpu;
        memory = this->memory;   
    }; 

    // returns the requirements of this VM (as is) and reset them
    void reset_requirements(float& cpu, int& memory);

    // adds usage requirements to this VM
    void add_requirements(float c, int m);

private:
    
    void init_attributes();

    // VM attributes 
    int   oid;
    int   uid;
    int   gid;
    int64_t start;
    int64_t birth;
    int   memory;
    float cpu;
    int static_vm;
    // float prio;

};

#endif 

