/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#include "Log.h"

//#include <string.h>
//#include <stdlib.h>
#include <stdexcept>
//#include <sstream>
//#include <iostream>

//#include <sys/types.h>
//#include <unistd.h>

using namespace std;

const char Log::error_names[] ={ 'E', 'W', 'I', 'D', 'D', 'D' };

FileLog::FileLog(const string&   file_name,
                 const MessageType   level,
                 ios_base::openmode  mode)
        :Log(level), log_file_name(file_name)
{
    ofstream file;

    file.open(log_file_name.c_str(), mode);

    if (file.fail() == true)
    {
        throw runtime_error("Could not open log file");
    }

    if ( file.is_open() == true )
    {
        file.close();
    }
}

/* -------------------------------------------------------------------------- */

FileLog::~FileLog() { }

/* -------------------------------------------------------------------------- */

void FileLog::log(
    const char *            module,
    const MessageType       type,
    const char *            message)
{
    char        str[26];
    time_t      the_time;
    ofstream    file;

    if( type <= log_level)
    {
        file.open(log_file_name.c_str(), ios_base::app);

        if (file.fail() == true)
        {
            return;
        }

        the_time = time(NULL);

        ctime_r(&(the_time),str);

        // Get rid of final enter character
        str[24] = '\0';

        file << str << " ";
        file << "[" << module << "]";
        file << "[" << error_names[type] << "]: ";
        file << message;
        file << endl;

        file.flush();

        file.close();
    }
}

