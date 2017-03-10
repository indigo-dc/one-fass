#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <typeinfo>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdexcept>
#include <assert.h>

using namespace std;

xmlDocPtr   xml;
xmlXPathContextPtr ctx;

int get_nodes(const string& xpath_expr,
        std::vector<xmlNodePtr>& content)
{
    // copiato da ONE
    xmlXPathObjectPtr obj;

    obj = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(xpath_expr.c_str()), ctx);

    if (obj == 0)
    {
        return 0;
    }

    if (obj->nodesetval == 0)
    {
        xmlXPathFreeObject(obj);
        return 0;
    }

    xmlNodeSetPtr ns = obj->nodesetval;
    int           size = ns->nodeNr;
    int           num_nodes = 0;
    xmlNodePtr    cur;

    for(int i = 0; i < size; ++i)
    {
        cur = xmlCopyNode(ns->nodeTab[i],1);

        if ( cur == 0 || cur->type != XML_ELEMENT_NODE )
        {
            xmlFreeNode(cur);
            continue;
        }

        content.push_back(cur);
        num_nodes++;
    }

    xmlXPathFreeObject(obj);

    return num_nodes;
}


void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;
    
    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;
    
    fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
	assert(nodes->nodeTab[i]);
	
	if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
	    xmlNsPtr ns;
	    
	    ns = (xmlNsPtr)nodes->nodeTab[i];
	    cur = (xmlNodePtr)ns->next;
	    if(cur->ns) { 
	        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n", 
		    ns->prefix, ns->href, cur->ns->href, cur->name);
	    } else {
	        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n", 
		    ns->prefix, ns->href, cur->name);
	    }
	} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
	    cur = nodes->nodeTab[i];   	    
	    if(cur->ns) { 
    	        fprintf(output, "= element node \"%s:%s\"\n", 
		    cur->ns->href, cur->name);
	    } else {
    	        fprintf(output, "= element node \"%s\"\n", 
		    //cur->name);
		    xmlNodeGetContent(cur));
	    }
	} else {
	    cur = nodes->nodeTab[i];    
	    fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
	}
    }
}

void xml_parse(const string &xml_doc){

    // copied from ONE ObjectXML
    
    xml = xmlReadMemory (xml_doc.c_str(),xml_doc.length(),0,0,XML_PARSE_HUGE);
    if (xml == 0)
    {
        throw runtime_error("Error parsing XML Document");
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0)
    {
        xmlFreeDoc(xml);
        throw runtime_error("Unable to create new XPath context");
    }


}

static void print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

int main(int argc, char **argv){

    vector<xmlrpc_c::value> values;
   
    try{
        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result; 

        // construct list of parameters
        xmlrpc_c::paramList plist;
        plist.add(xmlrpc_c::value_string("oneadmin:nebula"));
        plist.add(xmlrpc_c::value_int(-2));
        plist.add(xmlrpc_c::value_int(-1));
        plist.add(xmlrpc_c::value_int(-1));
        plist.add(xmlrpc_c::value_int(-1));

        myClient.call("http://localhost:2633/RPC2", "one.vmpool.info",plist, &result);

    	values = xmlrpc_c::value_array(result).vectorValueValue();

    	bool   success = xmlrpc_c::value_boolean(values[0]);
        
        // one says failure
	if (!success){
    	    string message = xmlrpc_c::value_string(values[1]); 

            cout << "Oned returned failure... Error: " << message << "\n";
            
            }
        
    } catch (exception const& e){

        cout << "Cannot contact oned... Error: " << e.what() << "\n";
        
        cout << "Message type is: " << values[1].type() << "\n";
        }
   
    // l'output del metodo one.vmpool.info e' sempre una stringa 
    string vmlist(static_cast<string>(xmlrpc_c::value_string(values[1])));
    //cout <<  vmlist << "\n";
   
    // parsing
    xmlInitParser();
    xml_parse(vmlist);
    std::vector<xmlNodePtr> content;   
    get_nodes("/VM_POOL/VM[STATE=1 or ((LCM_STATE=3 or LCM_STATE=16) and RESCHED=1)]", content); 

    // itero sui nodi
    int oid;
    xpath(oid, "/VM/ID", -1); 



    // get root 
    //xmlNode *root_element = xmlDocGetRootElement(xml);
    // loop over nodes
    //print_element_names(root_element); 
    //xmlXPathObjectPtr xpathObj; 
    //const xmlChar* path = (const xmlChar*)"/VM_POOL/VM/ID";
    //xpathObj = xmlXPathEvalExpression(path, ctx);
    //print_xpath_nodes(xpathObj->nodesetval, stdout);


    // free the document
    xmlFreeDoc(xml);   
    // clean global variables that might have been allocated by the parser
    xmlCleanupParser();  
};   
