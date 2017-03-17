/**
 * VirtualMachine.h
 *
 *      Author: Sara Vallero 
 *      Author: Valentina Zaccolo
 */

#ifndef VIRTUAL_MACHINE_H_
#define VIRTUAL_MACHINE_H_

#include <sstream>
#include <string>
#include <vector>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <map>
//#include "ObjectXML.h"
//#include "HostPoolXML.h"
//#include "Resource.h"

//#include "VirtualMachineTemplate.h"

//class ImageDatastorePoolXML;

using namespace std;

class VirtualMachine
{
public:

//    VirtualMachine(const string &xml_doc)
//    {
//        init_attributes();
//    };

    /**
    *  Constructs an object using a XML Node. The node is copied to the new
    *  object
    */
    VirtualMachine(const xmlNodePtr node);

    ~VirtualMachine(){
        if (xml != 0){
            xmlFreeDoc(xml);
        }

        if ( ctx != 0){
            xmlXPathFreeContext(ctx);
        }

    };

  /*  {
        if (vm_template != 0)
        {
            delete vm_template;
        }

        if (user_template != 0)
        {
            delete user_template;
        }
    }
*/
 
 /**
     *  Gets elements by xpath.
     *    @param values vector with the element values.
     *    @param expr of the xml element
     */
    template<typename T>
    void xpaths(std::vector<T>& values, const char * expr)
    {
        xmlXPathObjectPtr obj;

        xmlNodePtr cur;
        xmlChar *  str_ptr;

        obj=xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(expr),ctx);

        if (obj == 0)
        {
            return;
        }

        switch (obj->type)
        {
            case XPATH_NUMBER:
                values.push_back(static_cast<T>(obj->floatval));
                break;

            case XPATH_NODESET:
                for(int i = 0; i < obj->nodesetval->nodeNr ; ++i)
                {
                    cur = obj->nodesetval->nodeTab[i];

                    if ( cur == 0 || cur->type != XML_ELEMENT_NODE )
                    {
                        continue;
                    }

                    str_ptr = xmlNodeGetContent(cur);

                    if (str_ptr != 0)
                    {
                        std::istringstream iss(reinterpret_cast<char *>(str_ptr));
                        T val;

                        iss >> std::dec >> val;

                        if (!iss.fail())
                        {
                            values.push_back(val);
                        }

                        xmlFree(str_ptr);
                    }
                }
                break;

            default:
                break;

        }

        xmlXPathFreeObject(obj);
    };

    void xpaths(std::vector<std::string>& values, const char * xpath_expr);


    /**
     *  Gets a xpath attribute, if the attribute is not found a default is used.
     *  This function only returns the first element
     *    @param value of the element
     *    @param xpath_expr of the xml element
     *    @param def default value if the element is not found
     *
     *    @return -1 if default was set
     */
    template<typename T>
    int xpath(T& value, const char * xpath_expr, const T& def)
    {
        std::vector<std::string> values;

        xpaths(values, xpath_expr);

        if (values.empty() == true)
        {
            value = def;
            return -1;
        }

        std::istringstream iss(values[0]);

        iss >> std::dec >> value;

        if (iss.fail() == true)
        {
            value = def;
            return -1;
        }

        return 0;
    }

    int xpath(std::string& value, const char * xpath_expr, const char * def);

    //--------------------------------------------------------------------------
    // Get Methods for VirtualMachineXML class
    //--------------------------------------------------------------------------
    int get_oid() const
    {
        return oid;
    };

    int get_uid() const
    {
        return uid;
    };

    int get_gid() const
    {
        return gid;
    };

    int get_hid() const
    {
        return hid;
    };

    int get_dsid() const
    {
        return dsid;
    };

    bool is_resched() const
    {
        return (resched == 1);
    }

    bool is_resume() const
    {
        return resume;
    }

    const string& get_rank()
    {
        return rank;
    };

    const string& get_ds_rank()
    {
        return ds_rank;
    };

    const string& get_requirements()
    {
        return requirements;
    };

    const string& get_ds_requirements()
    {
        return ds_requirements;
    }

    /**
     *  Return VM usage requirements
     */
    void get_requirements(int& cpu, int& memory); //, long long& disk,
        //vector<VectorAttribute *> &pci);

    /**
     *  Return the requirements of this VM (as is) and reset them
     *    @param cpu in unit
     *    @param memory in kb
     *    @param disk in mb (system ds usage)
     */
    void reset_requirements(float& cpu, int& memory); //, long long& disk);

    /**
     *  @return the usage requirements in image ds.
     */
    //map<int,long long> get_storage_usage();

    /**
     * Checks if the VM can be deployed in a public cloud provider
     * @return true if the VM can be deployed in a public cloud provider
     */
    /*bool is_public_cloud() const
    {
        return public_cloud;
    };
*/
    /**
     *   Adds usage requirements to this VM
     *     @param cpu in unit form
     *     @param m memory in kb
     *     @param d in mb (system ds usage)
     */
    void add_requirements(float c, int m); //, long long d);

    /**
     *  Adds (logical AND) new placement requirements to the current ones
     *    @param reqs additional requirements
     */
    void add_requirements(const string& reqs)
    {
        if ( reqs.empty() )
        {
            return;
        }
        else if ( requirements.empty() )
        {
            requirements = reqs;
        }
        else
        {
            requirements += " & (" + reqs + ")";
        }
    }

    /**
     *  Check if the VM is ACTIVE state
     */
    bool is_active() const
    {
        return state == 3;
    }

    //--------------------------------------------------------------------------
    // Matched Resources Interface
    //--------------------------------------------------------------------------
    /**
     *  Adds a matching host if it is not equal to the actual one
     *    @param oid of the host
     */
    /*void add_match_host(int oid)
    {
        if (( resched == 1 && hid != oid ) || ( resched == 0 ))
        {
            match_hosts.add_resource(oid);
        }
    };
*/
    /**
     *  Adds a matching datastore
     *    @param oid of the datastore
     */
  /*  void add_match_datastore(int oid)
    {
        match_datastores.add_resource(oid);
    }
*/
    /**
     *  Returns a vector of matched hosts
     */
  /*  const vector<Resource *> get_match_hosts()
    {
        return match_hosts.get_resources();
    }
*/
    /**
     *  Returns a vector of matched datastores
     */
  /*  const vector<Resource *> get_match_datastores()
    {
        return match_datastores.get_resources();
    }
*/
    /**
     *  Sort the matched hosts for the VM
     */
/*    void sort_match_hosts()
    {
        match_hosts.sort_resources();
    }
*/
    /**
     *  Sort the matched datastores for the VM
     */
/*    void sort_match_datastores()
    {
        match_datastores.sort_resources();
    }
*/
    /**
     *  Removes the matched hosts
     */
/*    void clear_match_hosts()
    {
        match_hosts.clear();
    }
*/
    /**
     *  Removes the matched datastores
     */
/*    void clear_match_datastores()
    {
        match_datastores.clear();
    }
*/
    /**
     * Marks the VM to be only deployed on public cloud hosts
     */
 //   void set_only_public_cloud();

    /**
     * Returns true is the VM can only be deployed in public cloud hosts
     * @return true is the VM can only be deployed in public cloud hosts
     */
   // bool is_only_public_cloud() const;

    /**
     *  Add a VM to the set of affined VMs. This is used for the VM leader
     *  when scheduling a group.
     *
     *  @param vmid of the affined vm
     *
     */
 /*   void add_affined(int vmid)
    {
        affined_vms.insert(vmid);
    }

    const set<int>& get_affined_vms() const
    {
        return affined_vms;
    }
*/
    //--------------------------------------------------------------------------
    // Capacity Interface
    //--------------------------------------------------------------------------

    /**
     *  Tests if the Image DS have enough free space to host the VM
     *    @param img_datastores Image Datastores
     *    @param error_msg error reason
     *    @return true if the Image Datastores can host the VM
     */
  //  bool test_image_datastore_capacity(
  //          ImageDatastorePoolXML * img_dspool, string & error_msg) const;

    /**
     *  Adds the VM disk requirements to each Image Datastore counter
     *    @param img_datastores Image Datastores
     */
    //void add_image_datastore_capacity(ImageDatastorePoolXML * img_dspool);

    //--------------------------------------------------------------------------
    // Action Interface
    //--------------------------------------------------------------------------

    /**
     *  Get the user template of the VM
     *    @return the template as a XML string
     */
  //  string& get_template(string& xml_str)
  /*  {
        if (user_template != 0)
        {
            user_template->to_xml(xml_str);
        }
        else
        {
            xml_str = "";
        }

        return xml_str;
    }
*/
    /**
     * Removes (but does not delete) the scheduled actions of the VM
     *
     * @param attributes to hold the VM actions
     */
/*    void get_actions(vector<Attribute *>& attributes) const
    {
        attributes.clear();

        user_template->remove("SCHED_ACTION", attributes);
    }
*/
    /**
     * Sets an attribute in the VM Template, it must be allocated in the heap
     *
     * @param attributes to hold the VM actions
     */
 /*   void set_attribute(Attribute* att)
    {
        return user_template->set(att);
    }
*/
    /**
     *  Checks the action to be performed and returns the corresponding XML-RPC
     *  method name.
     *    @param action_st, the action to be performed. The XML-RPC name is
     *    returned here
     *    @return 0 on success.
     */
    static int parse_action_name(string& action_st);

    /**
     *  Function to write a Virtual Machine in an output stream
     */
    friend ostream& operator<<(ostream& os, VirtualMachine& vm);

    /**
     * Adds a message to the VM's USER_TEMPLATE/SCHED_MESSAGE attribute
     *   @param st Message to set
     */
    //void log(const string &st);

    /**
     * Clears the VM's USER_TEMPLATE/SCHED_MESSAGE attribute
     * @return true if the template was modified, false if SCHED_MESSAGE did not
     * need to be deleted
     */
    //bool clear_log();

private:

    /**
     *  For constructors
     */

    // TODO: not sure if the 2 below are needed
    const char **paths;	         // array of paths to look for attributes in search method
    int num_paths;               // number of elements in paths array

    
    xmlDocPtr   xml;             // XML representation of the Object
    xmlXPathContextPtr ctx;      // XPath Context to access Object elements
    
 
    void init_attributes();

//    void init_storage_usage();

    /* ------------------- SCHEDULER INFORMATION --------------------------- */
/*
    ResourceMatch match_hosts;

    ResourceMatch match_datastores;

    bool only_public_cloud;

    set<int> affined_vms;
*/
    /* ----------------------- VIRTUAL MACHINE ATTRIBUTES ------------------- */
    int   oid;

    int   uid;
    int   gid;

    int   hid;
    int   dsid;

    int   resched;
    bool  resume;

    int   state;

    int         memory;
    float       cpu;
    long long   system_ds_usage;

    map<int,long long> ds_usage;

    bool   public_cloud;

    string rank;
    string requirements;

    string ds_requirements;
    string ds_rank;

  //  VirtualMachineTemplate * vm_template;   /**< The VM template */
  //  VirtualMachineTemplate * user_template; /**< The VM user template */

};

#endif /* VM_XML_H_ */

