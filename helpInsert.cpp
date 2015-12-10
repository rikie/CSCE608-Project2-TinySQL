#include <iostream>
#include <iterator>
#include <cstdlib>
#include <ctime>
#include <string>
#include <list>
#include <algorithm>
#include "Block.h"
#include "Config.h"
#include "Disk.h"
#include "Field.h"
#include "MainMemory.h"
#include "Relation.h"
#include "Schema.h"
#include "SchemaManager.h"
#include "Tuple.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "helpInsert.h"
#include "helper.h"
#include "evalWhereConds.h"
#include "helpSelect.h"

// Function to process an INSERT Statement, and append tuple to a relation
void processInsert(string line, vector<string> cmdStr, SchemaManager schema_manager,
                   MainMemory &mem) {
  //cout << "the cmd str is " << endl;
  //printVector(cmdStr);
  //cout << endl;
  bool callFromInsert = false;
  vector<Tuple> outputTuples;
  vector<string>::iterator vit;
  vector<string>::iterator attrIT;
  vector<Tuple>::iterator tupleIT;
  vector<string> strings;
  string selectStmt;
  string tableName = cmdStr[2];
  Relation* relation_ptr = schema_manager.getRelation(tableName);
  //cout << "Inside the INSERT function" << endl;
  int memory_block_index=0;
  vector<string> insertStr = splitString(line, "\()");
  vector<string> attr = splitString(insertStr[1], ", ");

  // Check if SELECT keyword is there
  vit = find(cmdStr.begin(), cmdStr.end(), "SELECT");
  if(vit != cmdStr.end()) {
    callFromInsert = true;
    while(vit != cmdStr.end()) {
      selectStmt += " " + *vit;
      ++vit;
    }
    strings = splitString(selectStmt, " ");
    outputTuples = processSelect(selectStmt, strings, schema_manager, mem, callFromInsert);
    // get values from these tuples and insert in the table
    for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT) {
      Tuple tuple = relation_ptr->createTuple(); // create a tuple for each tuple from SELECT
      Schema tuple_schema = tuple.getSchema();
      for(attrIT = attr.begin(); attrIT != attr.end(); ++attrIT) {
        if (tuple_schema.getFieldType(*attrIT)==INT) {
          int val = tupleIT->getField(*attrIT).integer;
          tuple.setField(*attrIT, val);
        }
        else {
          string val = *(tupleIT->getField(*attrIT).str);
          tuple.setField(*attrIT, val);
        }
      }
      memory_block_index = findBlockForTuple(mem);
      appendTupleToRelation(relation_ptr, mem, memory_block_index, tuple);
    }
    //printTupleVector(outputTuples);
    //cout << endl;
  }
  else {
    //vector<string> insertStr = splitString(line, "\()");
    //vector<string> attr = splitString(insertStr[1], ", ");
    Tuple tuple = relation_ptr->createTuple();
    vector<string> val = splitString(insertStr[3], ", ");
    
    for (int i = 0; i < attr.size(); i++) {
      if (tuple.getSchema().getFieldType(attr[i])==INT)
        tuple.setField(attr[i], atoi(val[i].c_str()));
      else
        tuple.setField(attr[i], val[i]);
    }
    memory_block_index = findBlockForTuple(mem);
    appendTupleToRelation(relation_ptr, mem, memory_block_index, tuple);
  }
  
  // create a hash to map relation to the block used for the relation??
  // Use this hash later in select operations to get the blocks for the operations
  // Naive implementation of indexing??
  // appendTupleToRelation takes care of appending the tuple to appropriate memory in Disk
  
  //cout << "End of the function Insert" << endl;
 
  //addTupleToBlock(tuple, mem);
  //cout << "After insertion of tuple now the reation stuff is " << endl;
  // TODO do we need this function, what does it do?
  //printRelation(relation_ptr);
  //printing the relation after the insert
  //cout << "Now the relation is: " << endl;
  //cout << *relation_ptr << endl;
  return;
}

// An example procedure of appending a tuple to the end of a relation
// using memory block "memory_block_index" as output buffer
// To test this function please uncomment
void appendTupleToRelation(Relation* relation_ptr, MainMemory& mem, int memory_block_index, Tuple& tuple) {
                           Block* block_ptr;
  if (relation_ptr->getNumOfBlocks()==0) {
    //cout << "The relation is empty" << endl;
	//cout << "Hashing the relation to the first memory block" << endl;
	// add the memory block to the hash
	//relation_hash[relation_ptr->getRelationName()] = memory_block_index;
	  relation_hash[relation_ptr->getRelationName()].push_back(memory_block_index);
    //cout << "Get the handle to the memory block " << memory_block_index << " and clear it" << endl;
    block_ptr=mem.getBlock(memory_block_index);
    block_ptr->clear(); //clear the block
    block_ptr->appendTuple(tuple); // append the tuple
    //cout << "Write to the first block of the relation" << endl;
    relation_ptr->setBlock(relation_ptr->getNumOfBlocks(),memory_block_index);
  } else {
    //cout << "Read the last block of the relation into memory block: " << memory_block_index << endl;
    relation_ptr->getBlock(relation_ptr->getNumOfBlocks()-1,memory_block_index);
	//cout << "IMPT realtion block, memory block " << relation_ptr->getNumOfBlocks()-1 << " " << memory_block_index << endl;
    block_ptr=mem.getBlock(memory_block_index);
	//cout << "Printing block please" << *block_ptr << endl;
	// add the memory block to the relation_hash
	  relation_hash[relation_ptr->getRelationName()].push_back(memory_block_index);
    if (block_ptr->isFull()) {
	  //cout << "(The block is full: Clear the memory block and append the tuple)" << endl;
      block_ptr->clear(); //clear the block
      block_ptr->appendTuple(tuple); // append the tuple
      //cout << "Write to a new block at the end of the relation" << endl;
      relation_ptr->setBlock(relation_ptr->getNumOfBlocks(),memory_block_index); //write back to the relatio
	  //cout << "This condition should never be true, or big problem" << endl;
    } else {
      //cout << "(The block is not full: Append it directly)" << endl;
      block_ptr->appendTuple(tuple); // append the tuple
      //cout << "Write to the last block of the relation" << endl;
      relation_ptr->setBlock(relation_ptr->getNumOfBlocks()-1,memory_block_index); //write back to the relation
    }
  }
  // written to schema.. clear the block for future use?
  block_ptr->clear();
}
