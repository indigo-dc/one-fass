/**
 * PriorityManager.h 
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */


#ifndef PRIORITY_MANAGER_H
#define PRIORITY_MANAGER_H_

#include "Log.h"
//#include "HostPoolXML.h"
//#include "VMGroupPoolXML.h"
//#include "UserPoolXML.h"
//#include "ClusterPoolXML.h"
//#include "DatastorePoolXML.h"
#include "VMPool.h"
//#include "SchedulerPolicy.h"
//#include "ActionManager.h"
//#include "AclXML.h"

using namespace std;

class PriorityManager
{
public:

	PriorityManager(
        const string _one_xmlrpc,
        int _message_size,
        int _timeout,
        int _max_vm,
        int _max_dispatch,
        int _live_rescheds);

	~PriorityManager(){};	


    bool start();


private:

	string one_xmlrpc;
	int message_size;
	int timeout;
	int max_vm;
	int max_dispatch;
	int live_rescheds;	

	int set_up_pools();
};

#endif
