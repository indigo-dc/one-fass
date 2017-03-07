/*
 * Fass.h
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */
#ifndef INFLUXDB_H_
#define INFLUXDB_H_

#include "Log.h"

#include <stdlib.h>
#include <string>
//#include <sstream>


using namespace std;

/** This is the class handling the InfluxDb database */

class InfluxDb
{
public:

    static InfluxDb& instance()
    {
        static InfluxDb influxd;

        return influxd;
    };


private:

    /// Constructors and = are private to only access the class through instance. 

    InfluxDb()
    {
        // TODO: initialization goes here
    };


    ~InfluxDb()
    {
        // TODO: some delete goes here 
    };


    InfluxDb& operator=(InfluxDb const&){return *this;};

};

#endif /*INFLUXDB_H_*/
