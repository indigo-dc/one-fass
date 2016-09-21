/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef _FASS_LOG_H_
#define _FASS_LOG_H_

#include "Log.h"
#include <sstream>

using namespace std;


/* The Logger class for the Fass components */

class FassLog
{
public:

    // Init
    static void init_log_system(
        Log::MessageType    clevel,
        const char *        filename,
        ios_base::openmode  mode,
        const string&       daemon)
    {

        FassLog::logger = new FileLogTS(filename, clevel, mode);
    };
    
    // Finalize
    static void finalize_log_system()
    {
        delete logger;
    }

    // Log stuff
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
                                                                                                 
    // Get log level   
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
