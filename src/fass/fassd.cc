/* ------------------------------------ */
/* HEADER                               */
/*                                      */
/* Mailto: svallero AT to.infn.it       */
/*                                      */
/* ------------------------------------ */

#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <unistd.h>
//#include <time.h>
//#include <sys/types.h>
//#include <sys/stat.h>

#include "Fass.h"

using namespace std;

/* -------------------------------------------------------------------------- */

static void print_license()
{
    cout<< "Some INDIGO stuff should go here!\n"
        << Fass::version() << " TODO. \n";
}

/* -------------------------------------------------------------------------- */

static void print_usage(ostream& str)
{
    str << "Usage: fassd [-h] [-v] [-f] [-i]\n";
}

/* -------------------------------------------------------------------------- */

static void print_help()
{
    print_usage(cout);

    cout << "\n"
         << "SYNOPSIS\n"
         << "  Starts the One Fass daemon\n\n"
         << "OPTIONS\n"
         << "  -v, --verbose\toutput version information and exit\n"
         << "  -h, --help\tdisplay this help and exit\n"
         << "  -f, --foreground\tforeground, do not fork the fassd daemon\n"
         << "  -i, --init-db\tinitialize the dabase and exit\n";
}

/* ------------------------------------------------------------------------- */

static void fassd_init()
{
    try
    {
        Fass& fd  = Fass::instance();
        fd.bootstrap_db();
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;

        return;
    }
}

/* -------------------------------------------------------------------------- */

static void fassd_main()
{
    try
    {
        Fass& fd  = Fass::instance();
        fd.start();
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;

        return;
    }
}

/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    int             opt;
    bool            foreground = false;
    const char *    fl;
    int             fd;
    pid_t           pid,sid;
    string          wd;
    int             rc;

    static struct option long_options[] = {
        {"version",    no_argument, 0, 'v'},
        {"help",       no_argument, 0, 'h'},
        {"foreground", no_argument, 0, 'f'},
        {"init-db",    no_argument, 0, 'i'},
        {0,            0,           0, 0}
    };

    int long_index = 0;

    while ((opt = getopt_long(argc, argv, "vhif",
                    long_options, &long_index)) != -1)
    {
        switch(opt)
        {
            case 'v':
                print_license();
                exit(0);
                break;
            case 'h':
                print_help();
                exit(0);
                break;
            case 'i':
                fassd_init();
                exit(0);
                break;
            case 'f':
                foreground = true;
                break;
            default:
                print_usage(cerr);
                exit(-1);
                break;
        }
    }

    // Check if another fassd is running
    string lockfile;
    string var_location;

    fl = getenv("FASS_LOCATION");

    if (fl == 0) // Fass in root of FSH
    {
        var_location = "/var/lib/fass/";
        lockfile     = "/var/lock/fass/fass";
    }
    else
    {
        var_location = fl;
        var_location += "/var/";

        lockfile = var_location + ".lock";
    }

    fd = open(lockfile.c_str(), O_CREAT|O_EXCL, 0640);

    if( fd == -1)
    {
        cerr<< "Error: Cannot start fassd, opening lock file " << lockfile
            << endl;

        exit(-1);
    }

    close(fd);

    // Fork & exit main process
    if (foreground == true)
    {
        pid = 0; // do not fork
    }
    else
    {
        // this call returns zero in child and PID of child in the parent
        pid = fork();
    }


    switch (pid){
        case -1: // Error
            cerr << "Error: Unable to fork.\n";
            exit(-1);


        case 0: // Child process

            rc  = chdir(var_location.c_str());

            if (rc != 0)
            {
                goto error_chdir;
            }

            if (foreground == false)
            {
                // to run the process in new session and have a new group
                sid = setsid();

                if (sid == -1)
                {
                    goto error_sid;
                }
            }

            fassd_main();
            
            // delete the file if no process has it open
            unlink(lockfile.c_str());
            break;

        default: // Parent process
            break;
    }

    return 0;

error_chdir:
    cerr << "Error: cannot change to dir " << wd << "\n";
    unlink(lockfile.c_str());
    exit(-1);

error_sid:
    cerr << "Error: creating new session\n";
    unlink(lockfile.c_str());
    exit(-1);
}
