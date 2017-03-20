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

#ifndef _FASS_LOG_H_
#define _FASS_LOG_H_

#include "Log.h"
#include <sstream>

using namespace std;


/** The Logger class for the Fass components */

class FassLog
{
public:

    /// Init
    static void init_log_system(
        Log::MessageType    clevel,
        const char *        filename,
        ios_base::openmode  mode,
        const string&       daemon)
    {

        FassLog::logger = new FileLogTS(filename, clevel, mode);
    };
    
    /// Finalize
    static void finalize_log_system()
    {
        delete logger;
    }

    /// Log stuff
    static void log(
        const char *           module,
        const Log::MessageType type,
        const char *           message)
    {
        logger->log(module,type,message);
    };

    static void log(
        const char *           module,
        const Log::MessageType type,
        const ostringstream&   message)
    {
        logger->log(module,type,message.str().c_str());
    };

    static void log(
        const char *           module,                                                           
        const Log::MessageType type,                                                             
        const string&          message)                                                          
    {                                                                                            
        logger->log(module,type,message.c_str());                                                
    };                                                                                           
                                                                                                 
    /// Get log level   
    static Log::MessageType log_level()
    {
        return logger->get_log_level();
    };

private:
    FassLog(){};

    ~FassLog(){};

    static Log *   logger;
};

#endif
