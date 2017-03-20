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

#ifndef FASSDB_H_
#define FASSDB_H_

#include "Log.h"

#include <stdlib.h>
#include <string>
#include<vector>
//#include <sstream>


using namespace std;

/** This is the general class handling the back-end database */

class FassDb
{

public:

    // default constructor
    FassDb()
    {
    };


    virtual ~FassDb()
    {
        // TODO: some delete goes here 
    };


    FassDb& operator=(FassDb const&){return *this;};

    // methods below need to be implemented in the inheriting clases
    virtual bool write_initial_shares(const float share, const string user, const string group, const long int timestamp) = 0;
    // timestamp should be the same for all entries in a PM loop
    virtual bool write_queue(const int priority, const string user, const string group, const int vmid,  const float cpus, const float memory, const long int starttime, const long int timestamp) = 0;
    // for the usage records we should also tag the entries with the start time at which we start to integrate the usage  
    virtual bool write_usage(const float cpu_usage, const float memory_usage, const string user, const string group, const long int since, const long int timestamp) = 0;

};

/** This is the InfluxDb implementation */
// TODO: authentication

class InfluxDb: public FassDb
{
public:

    static InfluxDb& instance()
    {
        static InfluxDb influxdb;

        return influxdb;
    };

    // default constructor
    InfluxDb():FassDb()
    {
       _endpoint = "localhost";
       _port = 8086;
       _dbname = "fassdb";

       init_db();
    };
 
    // actual constructor
    InfluxDb(const string endpoint, const int port, const string dbname):FassDb()
    {
       _endpoint = endpoint;
       _port = port;
       _dbname = dbname;
       
       init_db();
    };


     ~InfluxDb()
    {
        // TODO: some delete goes here 
        //delete _endpoint;
        //delete _port;
        //delete _dbname;
    };

    // methods to be called by the priority manager
    // the first parameter is the actual measurement, other parameters are tags
    bool write_initial_shares(const float share, const string user, const string group, const long int timestamp);
    bool write_queue(const int priority, const string user, const string group, const int vmid,  const float cpus, const float memory, const long int starttime, const long int timestamp);
    // the first two parameters are the actual measurements, other parameters are tags
    bool write_usage(const float cpu_usage, const float memory_usage, const string user, const string group, const long int since, const long int timestamp);
private:
    
 
    InfluxDb& operator=(InfluxDb const&){return *this;};
    
    // initialization
    bool init_db();

    // generic query
    bool query_db(string method, string q, string &retval);

    // ping query, to check if the database is uo and running
    bool ping_db();

    // create a new database, the action has no effect if the database already exists 
    bool create_db();

    string _endpoint;
    int _port;
    string _dbname;

};
#endif /*FASSDB_H_*/
