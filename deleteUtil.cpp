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
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "deleteUtil.h"
#include "helper.h"
#include "evalWhereConds.h"

// process delete from talble where <condition>
void deleteTupleWhereClause(string table, vector<string> condVec, SchemaManager schema_manager, MainMemory &mem) {
  Relation* relation_ptr = schema_manager.getRelation(table);
  vector<string>::iterator vit;
  vector<string>evalCondVec;
  vector<Tuple> tuples;
  int numBlocksInRelation = 0;
  int remainingBlocksInRelation = relation_ptr->getNumOfBlocks();
  int maxTuplesInBlock = relation_ptr->getSchema().getTuplesPerBlock();
  //cout << "maxTuplesInBlock = " << maxTuplesInBlock << endl;
  int lastBlockRelation = 0;
  int i, j, numTuples;
  bool whereCond = false;
  ostringstream convertIntToStr;
  // If blank table, which is possible, print "empty table"
  if(remainingBlocksInRelation == 0) {
    cout << "No tuples in that relation, empty table" << endl;
    return;
  }
  // get the schema of the table
  Schema tuple_schema = schema_manager.getSchema(table);
  while(remainingBlocksInRelation > 0) {
    // Bring all the bloks in the main memory and print them
    if(remainingBlocksInRelation > NUM_OF_BLOCKS_IN_MEMORY) {
      numBlocksInRelation = NUM_OF_BLOCKS_IN_MEMORY;
    }
    else {
      numBlocksInRelation = remainingBlocksInRelation;
    }
    relation_ptr->getBlocks(lastBlockRelation, 0, numBlocksInRelation);
    for(i = 0; i < numBlocksInRelation; i++) {
      Block* block_ptr = mem.getBlock(i);
      numTuples = block_ptr->getNumTuples();
      // only one tuple per block and block full, so easy, no tuple offset involved
      if(numTuples == 1 && block_ptr->isFull()) {
        //cout << "One tuple per block" << endl;
        // this is the only tuple in the block
        Tuple tuple = block_ptr->getTuple(0);
        // if the where condition satisfies, then delete the tuple
        if(checkTupleStatisfiesWhere(tuple, condVec, tuple_schema)) {
          block_ptr->nullTuples();
        }
      }
      // else we will have to go through each of the tuples, using the tuple offset
      else {
        // TODO: Find how much a block can hold and then loop
        //cout << "Many tuple per block which is: " << numTuples<< endl;
        // Call int maxNumTuples = tempRelation_ptr->getSchema().getTuplesPerBlock();
        for(j = 0; j < maxTuplesInBlock; j++) {
          Tuple tuple = block_ptr->getTuple(j);
          //cout << "current offset = " << j << endl;
          if(tuple.isNull()){
            //cout << "We found a hole in our block! Do something about it!" << endl;
            continue; // this is a hole in MM block
           }
          // delete tuple if it satisfies where condition
          if(checkTupleStatisfiesWhere(tuple, condVec, tuple_schema)) {
            //cout << "We found a tuple we should delete! Delete it!" << endl;
            block_ptr->nullTuple(j);
          }
          else{
            //cout << "Tuple should not be deleted. All good here." << endl;
          }
        }
      }
    }
    relation_ptr->setBlocks(lastBlockRelation, 0, numBlocksInRelation);
    remainingBlocksInRelation -= NUM_OF_BLOCKS_IN_MEMORY;
    lastBlockRelation += NUM_OF_BLOCKS_IN_MEMORY;
  }
  return;
}

bool checkTupleStatisfiesWhere(Tuple tuple, vector<string> condVec, Schema tuple_schema) {
  vector<string>::iterator vit;
  ostringstream convertIntToStr;
  vector<string>evalCondVec;
  bool whereCond = false;
  for(vit=condVec.begin(); vit!=condVec.end(); ++vit) {
    if(isalpha((*vit).at(0)) && (*vit).compare("OR") !=0 && (*vit).compare("AND") !=0
		   && (*vit).compare("NOT") !=0) {
      if(tuple_schema.getFieldType(*vit)==INT) {
        convertIntToStr << tuple.getField(*vit).integer;
        evalCondVec.push_back(convertIntToStr.str());
        convertIntToStr.str("");
        convertIntToStr.clear();
      }
      else {
        evalCondVec.push_back(*(tuple.getField(*vit).str));
      }
    }
    else {
      evalCondVec.push_back(*vit);
    }
  }
  whereCond = doesTupleSatisfyWhere(evalCondVec);
  evalCondVec.clear();
  return whereCond;
}

bool checkTupleStatisfiesWhereJoin(Tuple tuple, vector<string> condVec, Schema tuple_schema) {
  vector<string>::iterator vit;
  vector<string>::iterator tempVit;
  ostringstream convertIntToStr;
  vector<string>evalCondVec;
  bool whereCond = false;
  //cout << "In checkTupleStatisfiesWhereJoin function " << endl;
  for(vit=condVec.begin(); vit!=condVec.end(); ++vit) {
    if(isalpha((*vit).at(0)) && (*vit).compare("OR") !=0 && (*vit).compare("AND") !=0
		  && (*vit).compare("NOT") !=0 && (*vit).compare("true") !=0 
      && (*vit).compare("false") !=0) {
      // That means that the variable is a field, fetch with correct fieldName
      // tricky to get the correct fieldname
      // the fielname changed compared to the condVec stuff
      // such as r.a might have become tempTable.r.a or 2tempTable.tempTable.r.a
      vector<string> fieldNames = tuple_schema.getFieldNames();
      //cout << "checkTupleStatisfiesWhereJoin Fieldnames are:" << endl;
      //printVector(fieldNames);
      for(tempVit = fieldNames.begin(); tempVit != fieldNames.end(); ++tempVit) {
        vector<string> tempVec = splitString(*tempVit, ". ");
        string tempSchema = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
        //cout << "tempSchema and *vit are " << tempSchema << " " << tempSchema.length << " " << (*vit) << " " << (*vit).length << endl;
        if((*vit).compare(tempSchema) == 0){
          //cout << "Comparing is true" << endl;
          if(tuple_schema.getFieldType(*tempVit)==INT) {
            convertIntToStr << tuple.getField(*tempVit).integer;
            evalCondVec.push_back(convertIntToStr.str());
            convertIntToStr.str("");
            convertIntToStr.clear();
            //cout << "Found INT, Now breaking" << endl;
            break;
          }
        //} old
          else {
            //cout << "Found STR20, Now breaking" << endl;
            evalCondVec.push_back(*(tuple.getField(*tempVit).str));
            break;
          }
        } // new
      }
      
    }
    else {
      evalCondVec.push_back(*vit);
    }
  }
  //cout << "Inside checkTupleStatisfiesWhereJoin function" << endl;
//cout << "The evalCondVec is: " << endl;
  //printVector(evalCondVec);
  whereCond = doesTupleSatisfyWhere(evalCondVec);
  //cout << "Where cond is " << whereCond << endl;
  evalCondVec.clear();
  return whereCond;
}
// Process all the delete statements
void processDelete(string line, vector<string> cmdStr, SchemaManager schema_manager, 
     MainMemory &mem) {
  //cout << "Inside processDelete" << endl;
  vector<string> deleteVec;
  vector<string> tableVec;
  vector<string> condVec;
  bool foundFrom = false;
  bool foundWhere = false;
  deleteVec = splitString(line, " ,");
  vector<string>::iterator vit;
  vector<string>::iterator nt;
  vit = deleteVec.begin();
  advance(vit, 1);
  for(; vit != deleteVec.end(); ++vit) {
    if((*vit).compare("FROM") == 0) {
      foundFrom = true;
      continue;
	  }
    if((*vit).compare("WHERE") == 0) {
      foundWhere = true;
      continue;
    }
    // We have found FROM keyword and WHERE, so get the condition vector if any
	  if (foundFrom && foundWhere){
        condVec.push_back(*vit);
    }
		// We have found only FROM so get the table list
		else{
		  tableVec.push_back(*vit);
		}
	}
  // if where is found then get the satisfying tuple in MM from disk, remove it and 
  // write back to the disk, check for holes.
  if(foundWhere) {
    //cout << "Need to implement this" << endl;
    // bring as much blocks from disk to main memory, apply the where condition,
    // then write the blocks back to disk
    condVec = removeDot(condVec);
    deleteTupleWhereClause(tableVec[0], condVec, schema_manager, mem);
  }
  // No where condition, then delete all the records of that relation/table 
  else {
    // simple case? just call deleteBlocks() on relation_ptr at block 0
    cout << "Deleting all the records in " << tableVec[0] << endl;
    Relation* relation_ptr = schema_manager.getRelation(tableVec[0]);
    relation_ptr->deleteBlocks(0);
  }
}