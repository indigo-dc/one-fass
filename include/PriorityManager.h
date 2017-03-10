/**
 * PriorityManager.h 
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */


#ifndef PRIORITY_MANAGER_H
#define PRIORITY_MANAGER_H_

#include "VMPool.h"
#include <time.h>

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
    VMPool * vmpool;

private:

	string one_xmlrpc;
	int message_size;
	int timeout;
	int max_vm;
	int max_dispatch;
	int live_rescheds;	

	bool set_up_pools();
	void do_schedule();

};

#endif
