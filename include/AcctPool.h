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

    // retrieves the accounting information
    // evaluate historical usage and store it as AcctObject 
    /*
    int eval_usage(vector<int> const &uids,
                          long int &time_start,
                          long int &time_stop,
                          long int &period,
                          int &n_periods);
    */
    int set_up(vector<int> const &uids);

    // gets an object from the pool
    /*
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
*/
private:

    // deletes pool objects 
    void flush(); 

    // adds an object to the pool
    void make_object(int uid, xmlNodePtr node);
 
    // gets list of VMs from ONE
    int load_acct(xmlrpc_c::value &result);

    // class variables

    XMLRPCClient *  client;

    // hash map contains the suitable [id, object] pairs
    map<int, AcctObject*> objects; 
    //map<int, list<AcctObject*> > objects; 

};

#endif
