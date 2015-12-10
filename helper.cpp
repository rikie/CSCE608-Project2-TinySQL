// Special includes
#include<iostream>
#include<iterator>
#include<cstdlib>
#include<ctime>
#include<string>
#include<list>
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
#include "helper.h"
#include "deleteUtil.h"

using namespace std;

// print a string vector
void printVector(vector<string> vstr) {
  vector<string>::iterator vit;
  cout << "Printing the vector string" << endl;
  for(vit = vstr.begin(); vit != vstr.end(); ++vit) {
    cout << *vit << " " << endl;
  }
  cout << endl;
}

void printSchema(Schema schema) {
  vector<string> field_names;
  vector<enum FIELD_TYPE> field_types;
    // Print the information about the schema
  cout << schema << endl;
  
  cout << "The schema has " << schema.getNumOfFields() << " fields" << endl;
  cout << "The schema allows " << schema.getTuplesPerBlock() << " tuples per block" << endl;
  cout << "The schema has field names: " << endl;
  field_names=schema.getFieldNames();
  copy(field_names.begin(),field_names.end(),ostream_iterator<string,char>(cout," "));
  cout << endl;
  cout << "The schema has field types: " << endl;
  field_types=schema.getFieldTypes();
  for (int i=0;i<schema.getNumOfFields();i++) {
    cout << (field_types[i]==0?"INT":"STR20") << "\t";
  }
  cout << endl;  
  cout << "The first field is of name " << schema.getFieldName(0) << endl;
  cout << "The second field is of type " << (schema.getFieldType(1)==0?"INT":"STR20") << endl;
  cout << "The field 3 is of type " << (schema.getFieldType("project")==0?"INT":"STR20") << endl;
  cout << "The field 4 is at offset " << schema.getFieldOffset("exam") << endl << endl;

  return;
 }
 
void printRelation(Relation* relation_ptr) {
  // Print the information about the Relation
  cout << "The table has name " << relation_ptr->getRelationName() << endl;
  cout << "The table has schema:" << endl;
  cout << relation_ptr->getSchema() << endl;
  cout << "The table currently have " << relation_ptr->getNumOfBlocks() << " blocks" << endl;
  cout << "The table currently have " << relation_ptr->getNumOfTuples() << " tuples" << endl << endl; 

  return;
}

vector<string> splitString(string str, string delimiter) {
  //cout << "Inside splitString function" << endl;
  //cout << str << endl;
  //cout << "Delimiter is " << delimiter << endl;
  vector<string> wordVector;
  wordVector.clear();
  std::size_t prev = 0, pos;
  while ((pos = str.find_first_of(delimiter, prev)) != std::string::npos) {
        if (pos > prev) {
            wordVector.push_back(str.substr(prev, pos-prev));
            //cout << "Inside while print: " << str.substr(prev, pos-prev) << endl;
        }
        prev = pos+1;
  }
  if (prev < str.length()) {
    wordVector.push_back(str.substr(prev, std::string::npos));
    //cout << "Outside while printing: " << str.substr(prev, std::string::npos) << endl;
  }
  //printVector(wordVector);
  return wordVector;
}

// Takes in a vector and returns a vector with dots removed and table name pruned
vector<string> removeDot(vector<string> processVec) {
  vector<string>::iterator vit;
  vector<string> tempVec;
  for(vit=processVec.begin(); vit!=processVec.end(); ++vit) {
    tempVec = splitString(*vit, ".");
    if(tempVec.size() > 1) *vit = tempVec[1];
    else *vit = tempVec[0];
  }
  return processVec;
}

// Create a free block list of the main memory
void resetFreeBlocks(list<int>& free_list){
	//empty free_list if not already clear
	free_list.clear();
	//refill block indices
	for (int i = 0; i < NUM_OF_BLOCKS_IN_MEMORY; i++){
		free_list.push_back(i);
	}
}

// Returns the free main memory block, no all full then return 0
int findBlockForTuple(MainMemory &mem){
  Block* block_ptr = mem.getBlock(free_blocks.front());
  while (block_ptr->isFull()){
    if (free_blocks.empty()){
	  //cout << "No free blocks left. Everything is terrible." << endl;
	  // cout << "No free blocks, using block 0 as default" << endl;
      return 0;
    }
    free_blocks.pop_front();
    block_ptr = mem.getBlock(free_blocks.front());
  }
	//we have a block that is not full
  //free(block_ptr);
  //cout << endl << endl;
  //cout << " BIG BIG BIG " << free_blocks.front() << endl << endl;
  return free_blocks.front();
}

// Print the inforamtion about the tuple - schema and the content
void printTuple(Tuple tuple){
  // Print the information about the tuple
  //cout << "Created a tuple " << tuple << " through the relation" << endl;
  //cout << "The tuple is invalid? " << (tuple.isNull()?"TRUE":"FALSE") << endl;
  Schema tuple_schema = tuple.getSchema();
  //cout << "The tuple has schema" << endl;
  //cout << tuple_schema << endl;
  //cout << "A block can allow at most " << tuple.getTuplesPerBlock() << " such tuples" << endl;
  
  //cout << "The tuple has fields: " << endl;
  for (int i=0; i<tuple.getNumOfFields(); i++) {
    if (tuple_schema.getFieldType(i)==INT)
      cout << tuple.getField(i).integer << "\t";
    else
      cout << *(tuple.getField(i).str) << "\t";
  }
  //cout << endl << endl;
}

// print vector of tuples
void printTupleVector(vector<Tuple> tuples){
  vector<Tuple>::iterator tupleIT;
  Schema tuple_schema = tuples[0].getSchema();
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    for (int i=0; i< tupleIT->getNumOfFields(); i++) {
      if (tuple_schema.getFieldType(i)==INT)
        cout << tupleIT->getField(i).integer << "\t";
      else
        cout << *(tupleIT->getField(i).str) << "\t";
    }
  }
  //cout << endl << endl;
}

// print vector of joined Tuples all atrributes
void printJoinedTupleAllAttr(vector<Tuple> outputTuples, vector<string> fieldNames,
                             vector<string> condVec, int typeSelect) {
  //cout << "Inside printJoinedTupleAllAttr function" << endl;
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;  
  for(vit = fieldNames.begin(); vit!= fieldNames.end(); ++vit) {
    //print the last two elements after splitting on . to eliminate the junk temp tablenames
    vector<string>temp = splitString(*vit, ".");
    int tempSize = temp.size();
    string tempStr = temp[tempSize-2] + "." + temp[tempSize-1];
    cout << tempStr << "\t";
    temp.clear();
  }
  cout << endl;
  Schema tuple_schema = outputTuples[0].getSchema();
  for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
    if(typeSelect == 12 ) {
        if(checkTupleStatisfiesWhereJoin(*tupleIT, condVec, tuple_schema)) {
        tupleIT->printTuple();
      }
    }
    else { // typeSelect = 10 - no where condition
      tupleIT->printTuple();
    }
    //cout << endl;
  }
 }
// print vector joined tuple Specific attributes
void printJoinedTupleSpAttr(vector<Tuple> outputTuples, vector<string> fieldNames, 
                            vector<string> attrVec, Schema tuple_schema,
                            vector<string> condVec, int typeSelect) {
  //cout << "Inside printJoinedTupleSpAttr function" << endl;
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;
  vector<string>::iterator tempVit;
  for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
    //process the tuple only if it satisfies the where condition {
    if(typeSelect == 13) {
      if(checkTupleStatisfiesWhereJoin(*tupleIT, condVec, tuple_schema)) {
        for(vit = attrVec.begin(); vit != attrVec.end(); ++vit) {
        //cout << "Printing " << *vit << endl;
          for(tempVit = fieldNames.begin(); tempVit != fieldNames.end(); ++tempVit) {
            //cout << "Field name processing is " << *tempVit << endl;
            string realFieldName;
            vector<string> tempVec = splitString(*tempVit, ".");
            if(tempVec.size() > 2)
              realFieldName = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
            else
              realFieldName = *tempVit;
            //cout << "Realfiel Name is " << realFieldName << endl;
            if((*vit).compare(realFieldName) == 0) {
              //cout << "matched " << realFieldName << endl;
              if(tuple_schema.getFieldType(*tempVit) == INT)
                cout << tupleIT->getField(*tempVit).integer << "\t";
              else
                cout << *(tupleIT->getField(*tempVit).str) << "\t";
              //cout << endl;
              break;
            }
          }
        }
      }
    }
    else { // typeSelect == 11 -> no where conditoion, specific attributes
      for(vit = attrVec.begin(); vit != attrVec.end(); ++vit) {
      //cout << "Printing " << *vit << endl;
        for(tempVit = fieldNames.begin(); tempVit != fieldNames.end(); ++tempVit) {
          //cout << "Field name processing is " << *tempVit << endl;
          string realFieldName;
          vector<string> tempVec = splitString(*tempVit, ".");
          if(tempVec.size() > 2)
            realFieldName = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
          else
            realFieldName = *tempVit;
          //cout << "Realfiel Name is " << realFieldName << endl;
          if((*vit).compare(realFieldName) == 0) {
            //cout << "matched " << realFieldName << endl;
            if(tuple_schema.getFieldType(*tempVit) == INT)
              cout << tupleIT->getField(*tempVit).integer << "\t";
            else
              cout << *(tupleIT->getField(*tempVit).str) << "\t";
            break;
          }
        }
      }  
    }
    cout << endl;
  }
}

// to process DISTINCT keywords
// compare two tuples for select distinct * from table [where]
bool compareTuplesAllAttr(Tuple t1, Tuple t2) {
  //cout << "Inside compareTuplesAllAttr function" << endl;
  vector<string>::iterator vit;
  if(t1.isNull() || t2.isNull()) return false;
  Schema tuple_schema1 = t1.getSchema();
  Schema tuple_schema2 = t2.getSchema();
  if(tuple_schema1 != tuple_schema2) return false;
  vector<string> attrVec = tuple_schema1.getFieldNames();
  for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
    if(tuple_schema1.getFieldType(*vit) == INT) {
      if(t1.getField(*vit).integer != t2.getField(*vit).integer)
        return false;
    }
    else {
      if((*(t1.getField(*vit).str)).compare(*(t2.getField(*vit).str)) != 0)
        return false;
    }
  }
  return true;
}

// compare two tuples when the attributes are specified
// To process the DISTINCT statement in Select a, b, c ...... statements
bool compareTuplesSpAttr(Tuple t1, Tuple t2, vector<string> attrVec) {
  vector<string>::iterator vit;
  // TODO: we should not process null tuples in the first palce
  if(t1.isNull() || t2.isNull()) return false;
  Schema tuple_schema1 = t1.getSchema();
  Schema tuple_schema2 = t2.getSchema();
  if(tuple_schema1 != tuple_schema2) return false;
  for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
    if(tuple_schema1.getFieldType(*vit) == INT) {
      //cout << t1.getField(*vit).integer << " " << t2.getField(*vit).integer << endl;
      if(t1.getField(*vit).integer != t2.getField(*vit).integer){
        //cout << "Values do not match: " << t1.getField(*vit).integer << " != " << t2.getField(*vit).integer << endl;
        return false;
      }
    }
    else {
      if((*(t1.getField(*vit).str)).compare(*(t2.getField(*vit).str)) != 0)
        return false;
    }
  }
  return true;
}

// TODO: remove dot from OrderbyVec
bool TupleComparator::operator()(Tuple t1, Tuple t2) {
  // if either first is null and second is not return false
  // if first is no null and second is then return true
  vector<string>::iterator vit;
  if(t1.isNull() && t2.isNull()) return false;
  if(t1.isNull() && !t2.isNull()) return false;
  if(!t1.isNull() && t2.isNull()) return true;
  Schema tuple_schema1 = t1.getSchema();
  //cout << "mAttrVec" << endl;
  //printVector(mAttrVec);
  // go through all the list of attrVec and return
  // needed for two pass DISTINCT -> attrVec
  // compare the two integer values of the column and return
  for(vit=mAttrVec.begin(); vit != mAttrVec.end(); ++vit) {
    //cout << "Comparing " << *vit << endl;
    if(tuple_schema1.getFieldType(*vit) == INT) {
      if(t1.getField(*vit).integer != t2.getField(*vit).integer)
        return t1.getField(*vit).integer < t2.getField(*vit).integer;
      else {
        //cout << t1.getField(*vit).integer << " is same for t1 and t2" << endl;
        continue;
      }
    }
    else {
      if((*(t1.getField(*vit).str)).compare(*(t2.getField(*vit).str)) < 0)
        return true;
      else if((*(t1.getField(*vit).str)).compare(*(t2.getField(*vit).str)) > 0)
        return false;
      else
        continue;
    }
  }
  return false;
}

// modfies the tuple vector to remove the duplciates
void checkDupTupleSpAttr(vector<Tuple> &tuples, Tuple tuple, vector<string> attrVec) {
  int count = 0;
  vector<Tuple>::iterator tupleIT;
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    //count = 0;
    if(compareTuplesSpAttr(*tupleIT, tuple, attrVec)){
      //return true;
      //cout << "Tuple is same" << endl;
      count++;
      //cout << "count = " << count << endl;
      if(count > 1) {
        //cout << "Erasing tuple, count is " << count << endl;
        tuples.erase(tupleIT);
        advance(tupleIT, -1);
      }
    }
  }
  //return false;
  return;
}

void checkDupTupleAllAttr(vector<Tuple> &tuples, Tuple tuple) {
  int count = 0;
  vector<Tuple>::iterator tupleIT;
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    if(compareTuplesAllAttr(*tupleIT, tuple)) {
      //cout << "Tuple is same" << endl;
      count++;
      if(count > 1) {
        //cout << "Erasing tuple, count is " << count << endl;
        tuples.erase(tupleIT);
        advance(tupleIT, -1);
      }
    }
  }
  return;
}

Schema createSchemaFromJoin(vector<string> attrVec, vector<string> tableVec, SchemaManager schema_manager, MainMemory &mem){
  
  vector<string> field_names;
  vector<string> table_names;
  vector<string> temp;
  vector<enum FIELD_TYPE> field_types;
  
  //get table and attribute names
  for (int i = 0; i < attrVec.size(); i++){
     temp = splitString(attrVec[i], ".");
     table_names.push_back(temp[0]);
     field_names.push_back(temp[1]);
     temp.clear();
  }
  
  for (int j = 0; j < field_names.size(); j++){
    //get field_types from table
    Schema tuple_schema = schema_manager.getSchema(table_names[j]);
    field_types.push_back(tuple_schema.getFieldType(field_names[j]));
  }
  
  //create new schema
  Schema schema = Schema(field_names,field_types);
  return schema;
}
  /**
  int count = 0;
  for(;it!=cmdStr.end();) {
    if (count == 0){
	  cout << (*it).substr(1) << endl;
	  field_names.push_back((*it).substr(1));
	  count++;
	}
	else{
	  cout << *it << endl;
	  field_names.push_back(*it);
	}
	++it;
	cout << (*it).substr(0,(*it).size()-1) << endl;
	if((*it).substr(0,(*it).size()-1).compare("INT") == 0)
	  field_types.push_back(INT);
	else
	  field_types.push_back(STR20);
	++it;
  }
  Schema *schema = new Schema(field_names,field_types);
  return schema;
  */

bool canBeJoined(Tuple t1, Tuple t2, string key1, string key2) {
  // if either first is null and second is not return false
  // if first is no null and second is then return true
  if(t1.isNull() || t2.isNull()) return false;
  Schema tuple_schema1 = t1.getSchema();
  Schema tuple_schema2 = t2.getSchema();
  //cout << "key 1 and key 2 are : " << key1 << " and " << key2 << endl;
  //cout << "Schema of t1 is " << endl;
  //tuple_schema1.printSchema();
  //cout << "Schema of t2 is " << endl;
  //tuple_schema2.printSchema(); 
  //cout << endl << endl;
  //compare the two integer values of the column and return  cvcv
  //cout << "Interesting " << key2.length() << endl;
  //cout << "So " << key2.at(0) << " and " << key2.at(1) << endl;
  if(tuple_schema1.getFieldType(key1) == INT) {
    //cout << "This is at leasr true" << endl;
    //cout << t1.getField(key1).integer << " and " << t2.getField("b").integer << endl;
    return t1.getField(key1).integer == t2.getField(key2).integer;
  }
  else {
    if((*(t1.getField(key1).str)).compare(*(t2.getField(key2).str)) == 0)
      return true;
    else
      return false;
  }
  //return true;
}  

bool checkTwoFieldsTuple(Tuple t, string key1, string key2) {
  Schema tuple_schema = t.getSchema(); 
  if(tuple_schema.getFieldType(key1) == INT) {
    return t.getField(key1).integer == t.getField(key2).integer;
  }
  else
    if((*(t.getField(key1).str)).compare(*(t.getField(key2).str)) == 0)
      return true;
    else
      return false;
}
// Two Pass Algorithms, make sorted Sublist in a tempRelation
// based on a attribute sortByAttr
// pass the tempRelation_ptr by reference
/**
    if(isDistinct) {
      // call sortRelationTwoPass with appropriate paramaters
      // create a temporary relation_ptr with the same schema as relation_ptr
      Relation* tempRelation_ptr = schema_manager.createRelation("twoPassTable",relation_ptr->getSchema());
      createSortedSublistRealtion(relation_ptr, tempRelation_ptr, );
    }
    //
*/
void createSortedSublistRelation(Relation* relation_ptr, Relation* &tempRelation_ptr, vector<string>attrVec,
                               MainMemory &mem, int memorySize) {
  int remBlocksRelation = relation_ptr->getNumOfBlocks();
  int lastBlockRelation = 0;
  int numBlocksRelation;
  vector<Tuple> tuples;
  vector<Tuple>::iterator tupleIT;
  //cout << "Inside createSortedSublistRelation function" << endl;
  //since this function was called we already know that numBlocksInRelation > MM
  // bring everything in MM 
  // reset the block in tempRelation_ptr before the sort is called and it is used?
  //tempRelation_ptr->deleteBlocks(0);
  //cout << "createSoremBlocksRelation, remBlocksRelation is " << remBlocksRelation << endl;
  while(remBlocksRelation > 0) {
    if(remBlocksRelation > memorySize) {
      numBlocksRelation = memorySize;
    }
    else {
      numBlocksRelation = remBlocksRelation;
    }
    // get the vector of tuples from MM 
    relation_ptr->getBlocks(lastBlockRelation, 0, numBlocksRelation);
    tuples = mem.getTuples(0, numBlocksRelation);
    // sort these tuples -> on which attribute?
    sort(tuples.begin(), tuples.end(), TupleComparator(attrVec));
    // tuples now are sorted, write them back to the MM & then DB -> in tempRelation
    int sizeTuples = tuples.size(); // to make sure we do not count holes
    //mem.setTuples(0, tuples);
    for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT){
      appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT);
    }
    remBlocksRelation -= memorySize;
    lastBlockRelation += memorySize; 
  }
}

vector<Tuple> getDistinctTuples(Relation* relation_ptr, Relation*& tempRelation_ptr, vector<string> attrVec, 
                                MainMemory &mem) {
  //cout << "In getDistinctTuples function " << endl;
  vector<Tuple> tempTuples;
  tempTuples.clear();
  Block* block_ptr;
  Block* block_ptr2;
  vector<Tuple>::iterator tupleIT;
  // hasmap for block in Sublist and block number to read
  map<int, int> sublistBlockNumHash;
  map<int, bool> sublistIsEmptyHash;
  //tempRelation_ptr = schema_manager.createRelation("twoPassTableDistinct",relation_ptr->getSchema());
  //string sortByAttr;  
  if(attrVec.size() == 1 && attrVec[0].compare("*") == 0)
    attrVec = relation_ptr->getSchema().getFieldNames();
  //else
    //sortByAttr = attrVec[0];
  createSortedSublistRelation(relation_ptr, tempRelation_ptr, attrVec, mem, NUM_OF_BLOCKS_IN_MEMORY);
  int numBlocksInRelation = tempRelation_ptr->getNumOfBlocks();
  
  //cout << "numBlocksInRelation  is " << numBlocksInRelation << endl;
  
  // Now do the second pass on the tempRelation_ptr relation
  //cout << "Doing second pass of DISTINCT operation" << endl;
  int numSortedSublist, numBlocksLastSublist;
  if(numBlocksInRelation%NUM_OF_BLOCKS_IN_MEMORY == 0) {
    numSortedSublist = numBlocksInRelation / NUM_OF_BLOCKS_IN_MEMORY;
    numBlocksLastSublist = NUM_OF_BLOCKS_IN_MEMORY;
  }
  else {
    numSortedSublist = numBlocksInRelation / NUM_OF_BLOCKS_IN_MEMORY + 1;
    numBlocksLastSublist = numBlocksInRelation % NUM_OF_BLOCKS_IN_MEMORY;
  }
  
  //cout << "numSortedSublist is " << numSortedSublist << endl;
  
  // bring one block from each sublist and comapare the minimum
  int blockCountRel = 0;
  int realBlockIndex, i, j, k;
  int totalBLocksToBring = NUM_OF_BLOCKS_IN_MEMORY;
  int remainingSublist = numSortedSublist;
  vector<Tuple> outputTuples;
  outputTuples.clear();
  Tuple minTuple = tempRelation_ptr->createTuple();
  minTuple.null();
  int maxNumTuples = tempRelation_ptr->getSchema().getTuplesPerBlock();
  bool foundMin = false;
  // create the hashmap, initialize with block 0 of disk and bring that to MM corresponding block
  for(i = 0; i < numSortedSublist; i++) {
    sublistBlockNumHash[i] = 0;
    sublistIsEmptyHash[i] = false;
    //realBlockIndex = sublistBlockNumHash[i] + (i * NUM_OF_BLOCKS_IN_MEMORY);
    tempRelation_ptr->getBlock(sublistBlockNumHash[i], i);
  }
  
  while(remainingSublist > 0) {
    for(i = 0; i < numSortedSublist; i++) {
      if(!sublistIsEmptyHash[i]) {
        block_ptr = mem.getBlock(i); // get the block_ptr
        //cout << "before " << endl;
        vector<Tuple> blockTupVec = block_ptr->getTuples(); // get vector of tuples
        //cout << "After " << endl;
        for(tupleIT = blockTupVec.begin(); tupleIT != blockTupVec.end(); ++tupleIT) {
          if(!tupleIT->isNull()) 
            tempTuples.push_back(*tupleIT);
        }
      }
    }
    minTuple = (*(min_element(tempTuples.begin(), tempTuples.end(), TupleComparator(attrVec))));
    //cout << "isDistinct: The minimum tuple is output it " << endl;
    //printTuple(minTuple);
    //cout << endl;
    tempTuples.clear();
    outputTuples.push_back(minTuple);
    //tempTuples.clear();
    
    do {
      foundMin = false;
      for(j = 0; j < numSortedSublist; j++) {
        if(sublistIsEmptyHash[j]) continue;
        //cout <<  "Maybe Here " << endl;
        block_ptr2 = mem.getBlock(j);
        //cout << "Now number of tuples in block " << j << " is: " << block_ptr2->getNumTuples() << endl;
        //int numTuples = block_ptr2->getNumTuples();
        //int maxNumTuples = FIELDS_PER_BLOCK / tempRelation_ptr->getSchema().getNumOfFields();
        // for all the tuples in that block search
        for(k = 0; k < maxNumTuples; k++) {
          if(block_ptr2->getNumTuples() == 0) continue;
          //cout <<  "Maybe THere " << endl;
          Tuple tuple = block_ptr2->getTuple(k);
          if(tuple.isNull()) continue; // this is a hole in MM block
          if(compareTuplesSpAttr(minTuple, tuple, attrVec)) {
            foundMin = true;
            //cout << "Similar Tuples with attrVec " << attrVec[0] << " is" << endl;
            //printTuple(tuple);
            //cout << endl;
            //delete it
            block_ptr2->nullTuple(k);
            //cout << "deleting tuple at positon " << k << endl;
            if(block_ptr2->getNumTuples() == 0) { // block is empty, no valid tuples -> all holes
              // clear the block
              // TODO this might create problems, block->ptr->clear();
              // bring new block from Disk, we have to change the hash value first
              //cout << "Block " << j << " is empty" << endl;
              sublistBlockNumHash[j]++;
              int compare;
              // last sublist special case
              if(j == numSortedSublist - 1) compare = numBlocksLastSublist;
              else compare = NUM_OF_BLOCKS_IN_MEMORY;
              if(sublistBlockNumHash[j] < compare) {
                //cout << "Fetching new block for " << j << endl;
                realBlockIndex = sublistBlockNumHash[j] + (j * NUM_OF_BLOCKS_IN_MEMORY);
                //cout << "The realBlockIndex is " << realBlockIndex << endl;
                tempRelation_ptr->getBlock(realBlockIndex, j);
              }
              else {
                //cout << "Else executed " << endl;
                if(!sublistIsEmptyHash[j])
                  remainingSublist--;
                sublistIsEmptyHash[j] = true;
                //cout << "value of remainingSublist is " << remainingSublist << endl;
              }
            }
          }
        }
      }
    } while(foundMin && remainingSublist > 0);
  }
  return outputTuples;
}


// Two Pass Sorting Algorithms
vector<Tuple> getSortedTuples(Relation* relation_ptr, Relation*& tempRelation_ptr, vector<string> attrVec, 
                              MainMemory &mem) {
  //cout << "In getSortedTuples function " << endl;
  vector<Tuple> tempTuples;
  tempTuples.clear();
  Block* block_ptr;
  Block* block_ptr2;
  vector<Tuple>::iterator tupleIT;
  // hashmap for block in Sublist and block number to read
  map<int, int> sublistBlockNumHash;
  map<int, bool> sublistIsEmptyHash;
  //tempRelation_ptr = schema_manager.createRelation("twoPassTableDistinct",relation_ptr->getSchema());
  //string sortByAttr;  
  if(attrVec.size() == 1 && attrVec[0].compare("*") == 0)
    attrVec = relation_ptr->getSchema().getFieldNames();
  //else
    //sortByAttr = attrVec[0];
  createSortedSublistRelation(relation_ptr, tempRelation_ptr, attrVec, mem, NUM_OF_BLOCKS_IN_MEMORY);
  int numBlocksInRelation = tempRelation_ptr->getNumOfBlocks();
  // Now do the second pass on the tempRelation_ptr relation
  int numSortedSublist, numBlocksLastSublist;
  if(numBlocksInRelation%NUM_OF_BLOCKS_IN_MEMORY == 0) {
    numSortedSublist = numBlocksInRelation / NUM_OF_BLOCKS_IN_MEMORY;
    numBlocksLastSublist = NUM_OF_BLOCKS_IN_MEMORY;
  }
  else {
    numSortedSublist = numBlocksInRelation / NUM_OF_BLOCKS_IN_MEMORY + 1;
    numBlocksLastSublist = numBlocksInRelation%NUM_OF_BLOCKS_IN_MEMORY;
  }
  // bring one block from each sublist and get the minimum
  int blockCountRel = 0;
  int realBlockIndex, i, j, k;
  int totalBLocksToBring = NUM_OF_BLOCKS_IN_MEMORY;
  int remainingSublist = numSortedSublist;
  vector<Tuple> outputTuples;
  outputTuples.clear();
  Tuple minTuple = tempRelation_ptr->createTuple();
  minTuple.null();
  int maxNumTuples = tempRelation_ptr->getSchema().getTuplesPerBlock();
  bool foundMin = false;
  // create the hashmap, initialize with block 0 of disk and bring that to MM corresponding block
  for(i = 0; i < numSortedSublist; i++) {
    sublistBlockNumHash[i] = 0;
    sublistIsEmptyHash[i] = false;
    //realBlockIndex = sublistBlockNumHash[i] + (i * NUM_OF_BLOCKS_IN_MEMORY);
    // TODO: Initialization is wrong, check
    tempRelation_ptr->getBlock(sublistBlockNumHash[i], i);
  }
  while(remainingSublist > 0) {
    for(i = 0; i < numSortedSublist; i++) {
      if(!sublistIsEmptyHash[i]) {
        block_ptr = mem.getBlock(i); // get the block_ptr
        vector<Tuple> blockTupVec = block_ptr->getTuples(); // get vector of tuples
        for(tupleIT = blockTupVec.begin(); tupleIT != blockTupVec.end(); ++tupleIT) {
          if(!tupleIT->isNull()) 
            tempTuples.push_back(*tupleIT);
        }
      }
    }
    minTuple = (*(min_element(tempTuples.begin(), tempTuples.end(), TupleComparator(attrVec))));
    tempTuples.clear();
    outputTuples.push_back(minTuple);
    do {
      foundMin = false;
      for(j = 0; j < numSortedSublist; j++) {
        if(sublistIsEmptyHash[j]) continue;
        //cout <<  "Maybe Here " << endl;
        block_ptr2 = mem.getBlock(j);
        //cout << "Now number of tuples in block " << j << " is: " << block_ptr2->getNumTuples() << endl;
        //int numTuples = block_ptr2->getNumTuples();
        //int maxNumTuples = FIELDS_PER_BLOCK / tempRelation_ptr->getSchema().getNumOfFields();
        // for all the tuples in that block search
        for(k = 0; k < maxNumTuples; k++) {
          if(block_ptr2->getNumTuples() == 0) continue;
          //cout <<  "Maybe THere " << endl;
          Tuple tuple = block_ptr2->getTuple(k);
          if(tuple.isNull()) continue; // this is a hole in MM block
          if(compareTuplesSpAttr(minTuple, tuple, attrVec)) {
            foundMin = true;
            outputTuples.push_back(minTuple);
            //cout << "Similar Tuples with attrVec " << attrVec[0] << " is" << endl;
            //printTuple(tuple);
            //cout << endl;
            //delete it
            block_ptr2->nullTuple(k);
            //cout << "deleting tuple at positon " << k << endl;
            if(block_ptr2->getNumTuples() == 0) { // block is empty, no valid tuples -> all holes
              // clear the block
              // TODO this might create problems, block->ptr->clear();
              // bring new block from Disk, we have to change the hash value first
              //cout << "Block " << j << " is empty" << endl;
              sublistBlockNumHash[j]++;
              int compare;
              // last sublist special case
              if(j == numSortedSublist - 1) compare = numBlocksLastSublist;
              else compare = NUM_OF_BLOCKS_IN_MEMORY;
              if(sublistBlockNumHash[j] < compare) {
                //cout << "Fetching new block for " << j << endl;
                realBlockIndex = sublistBlockNumHash[j] + (j * NUM_OF_BLOCKS_IN_MEMORY);
                //cout << "The realBlockIndex is " << realBlockIndex << endl;
                tempRelation_ptr->getBlock(realBlockIndex, j);
              }
              else {
                //cout << "Else executed " << endl;
                if(!sublistIsEmptyHash[j])
                  remainingSublist--;
                sublistIsEmptyHash[j] = true;
                //cout << "value of remainingSublist is " << remainingSublist << endl;
              }
            }
          }
        }
      }
    } while(foundMin && remainingSublist > 0);
  }
  return outputTuples;
}

bool compareDiffSchTuples(Tuple t1, Tuple t2, string key) {
  if(t1.isNull() && t2.isNull())
		return false;
	if((!t1.isNull()) && t2.isNull())
		return true;
	if(t1.isNull() && (!t2.isNull()))
		return false;
  
	Schema tuple_schema1 = t1.getSchema();
	Schema tuple_schema2 = t2.getSchema();
  if(tuple_schema1.getFieldType(key) == INT) {
    return t1.getField(key).integer < t2.getField(key).integer;
  }
  else {
    if((*(t1.getField(key).str)).compare(*(t2.getField(key).str)) < 0)
      return true;
    else
      return false;
  }
  return false;
}

/**
void removeTuplesInBlock(Tuple minTuple, int maxNumTuples, vector<string> attrVec, MainMemory &mem) {
  int k;
  for(k = 0; k < maxNumTuples; k++) {
  Tuple tuple = block_ptr2->getTuple(k);
  if(tuple.isNull()) continue; // this is a hole in MM block
  // delete tuple from other blocks which have same attrVec -> DISTINCT applied
  // compare this tuple to all other tuples on this attrVec
  if(compareTuplesSpAttr(minTuple, tuple, attrVec)) {
    cout << "Similar Tuples with attrVec " << attrVec[0] << " is" << endl;
    printTuple(tuple);
    cout << endl;
    //delete it
    block_ptr2->nullTuple(k);
    cout << "deleting tuple at positon " << k << endl;
    // if block all null tuples make it empty and bring a new block from disk from corresponding sorted sublist
    if(block_ptr2->getNumTuples() == 0) { // block is empty, no valid tuples -> all holes
      cout << "Block " << j << " is empty" << endl;
      sublistBlockNumHash[j]++;
      int compare;
      // last sublist special case
      if(j == numSortedSublist - 1) compare = numBlocksLastSublist;
      else compare - NUM_OF_BLOCKS_IN_MEMORY;
      if(sublistBlockNumHash[j] < compare) {
        cout << "Fetching new block for " << j << endl;
        realBlockIndex = sublistBlockNumHash[j] + (j * NUM_OF_BLOCKS_IN_MEMORY);
        cout << "The realBlockIndex is " << realBlockIndex << endl;
        tempRelation_ptr->getBlock(realBlockIndex, j);
        // remove if the current block has duplicate to mintupple remove here only
      }
      else {
        cout << "Else executed " << endl;
        if(!sublistIsEmptyHash[j])
          remainingSublist--;
        sublistIsEmptyHash[j] = true;
        cout << "No other value of sublistIsEmptyHash " << j << " is " << sublistIsEmptyHash[j] << endl;
        cout << "value of remainingSublist is " << remainingSublist << endl;
      }
    }
  }
  else {
    stillDups = false;
  }
  } 
  return;
}
*/
// Unused functions
/**
vector<string> splitString(string str) {
  cout << str <<endl;
  vector<string> strings;
  size_t pos = 0;
  size_t newpos;
  while(pos != string::npos) {
    newpos = str.find_first_of(" ", pos);
    strings.push_back(str.substr(pos, newpos-pos));
    if(pos != string::npos)
        pos++;
  } 
  cout << "Working" << endl;
  return strings;
}
*/