/**
 * BasicPlugin.h
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#include "InitialShares.h"

using namespace std;

class BasicPlugin
{
public:
	BasicPlugin(){};
	
	~BasicPlugin(){};

protected:
	
	void update_prio(int oid, int uid, int gid, int vm_cpu, int vm_memory, list<user >list_of_users, float &vm_prio);
}







