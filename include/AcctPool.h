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

#ifndef ACCT_POOL_H_
#define ACCT_POOL_H_

#include <map>
#include <list>

#include "XMLRPCClient.h"
#include "ObjectXML.h"
#include "User.h"

using namespace std;

class AcctPool: protected ObjectXML 
{
public:

    AcctPool(XMLRPCClient *  _client):
                ObjectXML(),
		client(_client) {
    };

    ~AcctPool() {
        objects.clear();
    };

    // retrieves the accounting information
    int set_up(list<user> user_list);

    // gets an object from the pool
    xmlNodePtr get(int oid) const
    {

	map<int, xmlNodePtr >::const_iterator it;

        it = objects.find(oid);

        if ( it == objects.end() )
        {
            return 0;
        }
        else
        {
            return static_cast<xmlNodePtr > (it->second);
        }
        
    };

private:

    // adds an object to the pool
    void add_object(int uid, xmlNodePtr node);
 
    // gets list of VMs from ONE
    int load_acct(xmlrpc_c::value &result);

    // class variables

    XMLRPCClient *  client;
    //unsigned int   machines_limit;
    //bool live_resched;

    // hash map contains the suitable [id, object] pairs
    map<int, xmlNodePtr> objects; 
};


#endif
