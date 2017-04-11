
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

#ifndef USER_H_
#define USER_H_

#include <map>
#include <vector>

struct Usage 
{
    int64_t usage;
    int64_t start_time;
    int64_t stop_time;
   
    Usage(int64_t usage, int64_t start_time, int64_t stop_time):
                                  usage(usage),
                                  start_time(start_time),
                                  stop_time(stop_time)
                                  {};  
    
};

class User
{
public:

    User() {};
    User(unsigned short userID,
      unsigned short groupID,
      float share):
        userID(userID),
        groupID(groupID),
        share(share)
        {};

      ~User(){};

    // setters
    void set_cpu_usage(int entry, struct Usage usage){
            
        cpu_usage.insert(pair<int, struct Usage>(entry, usage));   
    };

    void set_memory_usage(int entry, struct Usage usage){
            
        memory_usage.insert(pair<int, struct Usage>(entry, usage));   
    };

    // getters
    const map<int, struct Usage >& get_cpu_usage() const {

        return cpu_usage;
    };
 
    const map<int, struct Usage >& get_memory_usage() const {

        return memory_usage;
    };

    // clear usage objects
    void flush_usage(){
        cpu_usage.clear();
        memory_usage.clear();
    };

    unsigned short userID;
    unsigned short groupID;
    float share;

    // hash map contains [period_id, usage]
    map<int, struct Usage > cpu_usage; 
    map<int, struct Usage > memory_usage; 
};
 
#endif
