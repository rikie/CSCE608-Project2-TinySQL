// Special includes
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <ctime>
#include <string>
#include <list>
#include "Block.h"
#include "Config.h"
#include "Disk.h"
#include "Field.h"
#include "MainMemory.h"
#include "Relation.h"
#include "Schema.h"
#include "SchemaManager.h"
#include "Tuple.h"

// our includes
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "evalWhereConds.h"
#include "helper.h"
#include "helpInsert.h"
#include "helpSelect.h"
#include "project2.h"
#include "deleteUtil.h"
#include "dropUtil.h"

using namespace std;

//TODO -> clear the MM before each major operation, or each line of query
Schema* processCreate(vector<string>);

int main (int argc,char *argv[]) {
  //=======================Initialization=========================
  cout << "=======================Initialization=========================" << endl;
  // Initialize the memory, disk and the schema manager
  MainMemory mem;
  Disk disk;
  //cout << "The memory contains " << mem.getMemorySize() << " blocks" << endl;
  //cout << mem << endl << endl;
  SchemaManager schema_manager(&mem,&disk);
  Schema *schema;
  // Call our function to make a list of free blocks of main memory
  resetFreeBlocks(free_blocks);
  /*
  disk.resetDiskIOs();
  disk.resetDiskTimer();
  // Another way to time
  clock_t start_time;
  start_time=clock();
  */
  //=========================Initialization ends ==========================
  string line;
  vector<string> strings;

  ifstream myfile (argv[1]);
  //ifstream myfile ("TinySQL_linux.txt");
  //ifstream myfile("test.txt");
  if(myfile.is_open()) {
    while(getline (myfile,line)) {
      //cout << line << '\n';
      istringstream f(line);
      string s;    
      while(getline(f, s, ' ')) {
        //cout << s << endl;
        strings.push_back(s);
      }
      if(strings[0].compare("CREATE") == 0) {
        disk.resetDiskIOs();
        disk.resetDiskTimer();
        // Another way to time
        clock_t start_time;
        start_time=clock();
        cout << endl;
        cout << line << endl;
        // Creating a schema for the create statement.
        schema = processCreate(strings);
        //printSchema(*schema);
        // Creating the relation with the schema above using schema manager.
        string relation_name=strings[2];
        //cout << "Creating table " << relation_name << endl;
        Relation* relation_ptr=schema_manager.createRelation(relation_name, *schema);
        //printRelation(relation_ptr);
        cout << endl << "Printing the DISK IO Cost: " << endl;
        cout << "Real elapse time = " << ((double)(clock()-start_time)/CLOCKS_PER_SEC*1000) << " ms" << endl;
        cout << "Calculated elapse time = " << disk.getDiskTimer() << " ms" << endl;
        cout << "Calculated Disk I/Os = " << disk.getDiskIOs() << endl << endl;;
	    }
      else if(strings[0].compare("INSERT") == 0) {
        disk.resetDiskIOs();
        disk.resetDiskTimer();
        // Another way to time
        clock_t start_time;
        start_time=clock();
        cout << endl;
        cout << line << endl;
        //cout << "It is an INSERT statement " << endl;
        processInsert(line, strings, schema_manager, mem);
        cout << endl << "Printing the DISK IO Cost: " << endl;
        cout << "Real elapse time = " << ((double)(clock()-start_time)/CLOCKS_PER_SEC*1000) << " ms" << endl;
        cout << "Calculated elapse time = " << disk.getDiskTimer() << " ms" << endl;
        cout << "Calculated Disk I/Os = " << disk.getDiskIOs() << endl << endl;;
      }
      else if(strings[0].compare("SELECT") == 0) {
        disk.resetDiskIOs();
        disk.resetDiskTimer();
        // Another way to time
        clock_t start_time;
        start_time=clock();        
        cout << endl;
        cout << line << endl;
        //vector<Tuple> outputTuple;
        //cout << "It is a SELECT statement " << endl;
        processSelect(line, strings, schema_manager, mem, false);
        cout << endl << "Printing the DISK IO Cost: " << endl;
        cout << "Real elapse time = " << ((double)(clock()-start_time)/CLOCKS_PER_SEC*1000) << " ms" << endl;
        cout << "Calculated elapse time = " << disk.getDiskTimer() << " ms" << endl;
        cout << "Calculated Disk I/Os = " << disk.getDiskIOs() << endl << endl;;
      }
      else if(strings[0].compare("DELETE") == 0) {
        disk.resetDiskIOs();
        disk.resetDiskTimer();
        // Another way to time
        clock_t start_time;
        start_time=clock();
        cout << endl;
        cout << line << endl;
        //cout << "It is a DELETE statement " << endl;
        processDelete(line, strings, schema_manager, mem);
        cout << endl << "Printing the DISK IO Cost: " << endl;
        cout << "Real elapse time = " << ((double)(clock()-start_time)/CLOCKS_PER_SEC*1000) << " ms" << endl;
        cout << "Calculated elapse time = " << disk.getDiskTimer() << " ms" << endl;
        cout << "Calculated Disk I/Os = " << disk.getDiskIOs() << endl << endl;
      }
      else if(strings[0].compare("DROP") == 0) {
        disk.resetDiskIOs();
        disk.resetDiskTimer();
        // Another way to time
        clock_t start_time;
        start_time=clock();
        cout << endl;
        cout << line << endl;
        //cout << "It is a DROP statement " << endl;
        processDrop(line, strings[2], schema_manager, mem);
        cout << endl << "Printing the DISK IO Cost: " << endl;
        cout << "Real elapse time = " << ((double)(clock()-start_time)/CLOCKS_PER_SEC*1000) << " ms" << endl;
        cout << "Calculated elapse time = " << disk.getDiskTimer() << " ms" << endl;
        cout << "Calculated Disk I/Os = " << disk.getDiskIOs() << endl << endl;
      }
    strings.clear();  
    }
    myfile.close();
  }
  else cout << "Unable to open file";

  cout << endl << endl;
  cout << "End of the input file" << endl; 
  free(schema);  
  //cout << "In the end, the memory contains: " << endl;
  //cout << mem << endl;
  return 0;
}

Schema *processCreate(vector<string> cmdStr) {
  vector<string>::iterator it = cmdStr.begin();
  advance(it, 3);
  // Create the schema
  // cout << "Creating a schema" << endl;
  vector<string> field_names;
  vector<enum FIELD_TYPE> field_types;
  int count = 0;
  for(;it!=cmdStr.end();) {
    if (count == 0){
	  // cout << (*it).substr(1) << endl;
	  field_names.push_back((*it).substr(1));
	  count++;
	}
	else{
	  //cout << *it << endl;
	  field_names.push_back(*it);
	}
	++it;
	//cout << (*it).substr(0,(*it).size()-1) << endl;
	if((*it).substr(0,(*it).size()-1).compare("INT") == 0)
	  field_types.push_back(INT);
	else
	  field_types.push_back(STR20);
	++it;
  }
  Schema *schema = new Schema(field_names,field_types);
  return schema;
  //return;
}
