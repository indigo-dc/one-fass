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

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include "Request.h"

using namespace std;

class RequestQueue: public Request
{
protected:
    RequestQueue( const string& method_name,
                       const string& format_str,
                       const string& help,
                       const string& params)
        :Request(method_name,format_str,params,help)
    {};

    ~RequestQueue(){};


    virtual void request_execute(xmlrpc_c::paramList const& _paramList,
                                 RequestAttributes& att) = 0;
};

class ReorderedQueue : public RequestQueue
{
public:
    ReorderedQueue( const string& format_str ):
        RequestQueue("ReorderedQueue",
                          format_str, 
                          "Returns the reordered queue",
                          "A:s"){};

    ~ReorderedQueue(){};

    void request_execute(xmlrpc_c::paramList const& _paramList,
                         RequestAttributes& att);
};

#endif
