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

#include "VirtualMachine.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

void VirtualMachine::init(const xmlNodePtr node) {
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

    // initialize VM attributes
    init_attributes();
}

void VirtualMachine::init_attributes() {
    vector<xmlNodePtr> nodes;

    int rc;
    int action;

    string automatic_requirements;

    xpath(oid, "/VM/ID", -1);
    xpath(uid, "/VM/UID", -1);
    xpath(gid, "/VM/GID", -1);

    xpath(memory, "/VM/TEMPLATE/MEMORY", 0);
    xpath<float>(cpu, "/VM/TEMPLATE/CPU", 0);

}

void VirtualMachine::add_requirements(float c, int m) { 
    cpu    += c;
    memory += m;
}

void VirtualMachine::reset_requirements(float& c, int& m) {
    c = cpu;
    m = memory;

    cpu    = 0;
    memory = 0;
}

void VirtualMachine::get_requirements(int& cpu, int& memory) {

    if (this->memory == 0 || this->cpu == 0) {
        cpu    = 0;
        memory = 0;

        return;
    }

    cpu    = static_cast<int>(this->cpu * 100);  // now in 100%
    memory = this->memory * 1024;      // now in Kilobytes
}
