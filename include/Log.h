/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#include <fstream>
//#include <stdexcept>

using namespace std;

/* Log class: interface used by Fass components to log messages */

class Log
{
public:
    enum MessageType {
        ERROR   = 0,
        WARNING = 1,
        INFO    = 2,
        DEBUG   = 3,
        DDEBUG  = 4,
        DDDEBUG = 5
    };

    static const char error_names[];

    Log(const MessageType _level = DDDEBUG):log_level(_level){};

    virtual ~Log(){};

    MessageType get_log_level()
    {
        return log_level;
    }

    // Logger interface
    virtual void log(
        const char *            module,
        const MessageType       type,
        const char *            message) = 0;

protected:
    
    //Minimum log level for the messages
    MessageType log_level;
};

/* FileLog class: writes messages to a log file */

class FileLog : public Log
{
public:
    FileLog(const string&       file_name,
            const MessageType   level    = WARNING,
            ios_base::openmode  mode     = ios_base::app);

    virtual ~FileLog();

    virtual void log(
        const char *            module,
        const MessageType       type,
        const char *            message);

private:
    string log_file_name;
};

/* FileLogTS class: thread safe.
   Implements mutex since different services log to the same file. */

class FileLogTS : public FileLog
{
public:
    FileLogTS(const string&       file_name,
                    const MessageType   level    = WARNING,
                    ios_base::openmode  mode     = ios_base::app)
                     :FileLog(file_name,level,mode)
    {
        pthread_mutex_init(&log_mutex,0);
    }

    ~FileLogTS()
    {
        pthread_mutex_destroy(&log_mutex);
    }

    void log(
        const char *            module,
        const MessageType       type,
        const char *            message)
    {
        pthread_mutex_lock(&log_mutex);
        FileLog::log(module,type,message);
        pthread_mutex_unlock(&log_mutex);
    }

private:
    pthread_mutex_t log_mutex;
};

#endif /* _LOG_H_ */
