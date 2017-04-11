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
#include <vector>
#include <list>

#include "XMLRPCClient.h"
#include "ObjectXML.h"
#include "AcctObject.h"
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
        flush();
    };

    // evaluate historical usage and store it in User object 
    int eval_usage(list<User> *user_list,
                          int64_t &time_start,
                          int64_t &time_stop,
                          int64_t &period,
                          int &num_periods);

    int set_up(vector<int> const &uids);

    // gets an object from the pool
    
    list<AcctObject*>   get(int oid) const {
        list<AcctObject*> retval;

	map<int, list<AcctObject*> >::const_iterator it;

        it = objects.find(oid);

        if ( it == objects.end() )
        {
            return retval;
        }
        else
        {
            return static_cast<list<AcctObject*> > (it->second);
        }
    };

private:

    // deletes pool objects 
    void flush(); 

    // adds an object to the pool
    void make_user_object(int uid, vector<xmlNodePtr> node);
 
    // gets list of VMs from ONE
    int load_acct(xmlrpc_c::value &result);

    // class variables

    XMLRPCClient *  client;

    // hash map contains the suitable [id, list] pairs
    map<int, list<AcctObject*> > objects; 

};

#endif
