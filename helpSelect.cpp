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
#include "helpSelect.h"
#include "selectJoinUtil.h"
#include "helper.h"
#include "evalWhereConds.h"
#include "deleteUtil.h"


// Process all the select statements
vector<Tuple> processSelect(string line, vector<string> cmdStr, SchemaManager schema_manager, 
     MainMemory &mem, bool callFromInsert) {
       
  vector<string> selectVec;
  vector<string> attrVec;
  vector<string> tableVec;
  vector<string> condVec;
  vector<string> orderByVec;
  vector<Tuple> outputTuples;
  bool isDistinct = false;
  bool foundOrderBy = false;
  bool foundFrom = false;
  bool foundWhere = false;
  string orderByAttr = "";
  
  //cout << "Processing Select " << endl;  
  selectVec = splitString(line, " ,");
  vector<string>::iterator it;
  vector<string>::iterator nt;
  it = selectVec.begin();
  advance(it, 1);
  //cout << "After advancing, the it is: " << *it << endl;
  if ((*it).compare("DISTINCT") == 0) {
    //cout << "Did not find DISTINCT" << endl;
    //cout << "Found DISTINCT" << endl;
    isDistinct = true;
    advance(it, 1);
  }
  // create the data structure for tables, conditions etc.
  for(; it != selectVec.end(); ++it) {
    if((*it).compare("FROM") == 0) {
      foundFrom = true;
      continue;
    }
    if((*it).compare("WHERE") == 0) {
      foundWhere = true;
      continue;
    }
    if((*it).compare("ORDER") == 0) {
      foundOrderBy = true;
      continue;
    }
    if((*it).compare("BY") == 0 && foundOrderBy) {
      continue;
    }
	  // Till FROM keyword is not scanned, push attribute list
    if(!foundFrom) {
	    attrVec.push_back(*it);
	  }
	/*
	// We scanned FROM but not WHERE, these are table names1
	if(foundFrom && !foundWhere) {
	  tableVec.push_back(*it);
	}
	// FROM and WHERE have been scanned ORDER BY remains, push that statement.
	if(foundFrom && foundWhere && !foundOrderBy) {
	  condVec.push_back(*it);
	}
	// ORDER BY is there in the statement, is there GROUP BY?
	if(foundOrderBy && !foundGroupBy) {
	  orderByVec.push_back(*it);
	}
	*/
	// We have found a FROM keyword
	  else if (foundFrom){
		//we have found a where clause, but no order by clause
		  if (foundWhere && !foundOrderBy){
		    condVec.push_back(*it);
      }
      /*
      if(foundWhere) {
        if(!foundOrderBy) condVec.push_back(*it);
        else orderByVec.push_back(*it);
      }
      */
      
      // we have found an order by clause, but no where clause
      // we have found an order by clause along with where clase
		  else if ((foundOrderBy && !foundWhere) || (foundOrderBy && foundWhere)){
		    orderByVec.push_back(*it);
		  }
		//we have found both (order by takes priority)
		
		//we have found neither clause (default case)
		  else{
		    tableVec.push_back(*it);
		  }
	  }
  }
  //cout << "Printing the orderByVec vector" << endl;
  //printVector(orderByVec);
  //cout << "Printing the tableVec vector" << endl;
  //printVector(tableVec);
  //cout << "Printing the condVec vector" << endl;
  //printVector(condVec);
  //cout << "Printing the attrVec vector" << endl;
  //printVector(attrVec);
  //cout << endl << endl;
  /**
  cout << "Printing the conditions in the Select clause" << endl;
  for(it=condVec.begin(); it!=condVec.end(); ++it) {
    cout << *it << endl;
  }
  */
  //TODO add a condition to check only single table after from, no joins
  // 24 Nov: I found that separate the logic for join and no join
  // select a,b,c from table where c = 90
  // select s.a, s.b, t.c from s, t where s.a = t.c
  // process select statement with no joins - single table
  int typeSelect = 0;
  // No join, single table
  if (tableVec.size() == 1) {
    attrVec = removeDot(attrVec);
    //tableVec = removeDot(tableVec);
    condVec = removeDot(condVec);
    orderByVec = removeDot(orderByVec);
    if(foundWhere) {
      if(attrVec.size() == 1 && attrVec[0].compare("*") == 0)
        typeSelect = 2; // select * from table where <condition>
      else
        typeSelect = 3; // select a, b, c from table where <condition>
    }
    //printing the schema for Select * from table
    else if(attrVec.size() == 1 && attrVec[0].compare("*") == 0 && !foundWhere){
      typeSelect = 0;
    }
    else { // there is no select * but something like select a,b,c from xyz [no where clause]
      typeSelect = 1;
    }
    //cout << "Calling the No Join Select Print function" << endl;
    if(orderByVec.size() == 1)
      orderByAttr = orderByVec[0];

    outputTuples = printSelectNoJoinOnePass(typeSelect, attrVec, tableVec, condVec, orderByVec, 
                             schema_manager, mem, isDistinct, foundOrderBy, 
                             callFromInsert);
    if(callFromInsert)
      return outputTuples;
                          
    if(typeSelect == 0 || typeSelect == 2) {
      printTupleVecAllAttr(outputTuples);
    }
    else if(typeSelect == 1 || typeSelect == 3) {
      printTupleVecSpecificAttr(outputTuples, attrVec);
    }
    outputTuples.clear();
  }
  // There is more than one table involved, new logic - joins
  else {
    //cout << "Joins detected" << endl;
    if(foundWhere) {
      //cout << "WHERE condition found " << endl;
      if(attrVec.size() == 1 && attrVec[0].compare("*") == 0) { // select * from r, s, t where <condition> - natural/cross
        typeSelect = 12;
        //cout <<  "Here " << endl;
      }
      else if(attrVec.size() > 1) { // select r.a, t.b etc. from r, t, where <condition> - natural/cross
        typeSelect = 13;
      }
      //processSelectJoin(typeSelect, attrVec, tableVec, condVec, orderByAttr, 
      //                    schema_manager, mem, isDistinct, foundOrderBy);
      //printSelectJoinNoWhere(attrVec, tableVec, schema_manager, mem);
      
      //vector<string> joinConds = getJoinConditions(condVec);
      //cout << "found join conditions:" << endl;
      //for (int i = 0; i < joinConds.size(); i++){
        //cout << joinConds[i] << endl;
      //}
      //cout << endl;
      
    }
    // printing statment of the form like select s.a, t.b, r.c from s, t, r
    // printing statment like select * from s, t, r
    else if(attrVec.size() == 1 && attrVec[0].compare("*") == 0 && !foundWhere){
      typeSelect = 10;
     // processSelectJoin(typeSelect, attrVec, tableVec, condVec, orderByAttr, 
      //                  schema_manager, mem, isDistinct, foundOrderBy);
    }
    else if(attrVec.size() > 1 && !foundWhere){
      typeSelect = 11;
      //processSelectJoin(typeSelect, attrVec, tableVec, condVec, orderByAttr, 
      //                  schema_manager, mem, isDistinct, foundOrderBy);
    }
    processSelectJoin(typeSelect, attrVec, tableVec, condVec, orderByVec, 
                      schema_manager, mem, isDistinct, foundOrderBy, 
                      callFromInsert);
  }
  //handle from
  //right now, i'm just going to pull the first name from tableVec; this will need to be redone
  //Relation* relation_ptr = schema_manager.getRelation(tableVec[0]);
  //handle where (if present)
  
  //handle group by (if present)
  
  //create projection based on attrVec
  /*
  if(attrVec.size() == 1 && attrVec[0].compare("*") == 0){
	//asterisk found; return all attributes
	//for now, we will just print
	cout << "Found relation " << tableVec[0] << endl;
	cout << *relation_ptr << endl << endl;
  }
  else{
	//we have one or more attributes to work with
	//return projection of results, using attributes in attrVec
  }
  */
  
  /*
  string tableName = selectVec[3];
  Relation* relation_ptr = schema_manager.getRelation(tableName);
  cout << "Now the relation contains: " << endl;
  cout << *relation_ptr << endl << endl;
  return;
  */
  outputTuples.clear();
  return outputTuples;
}

vector<Tuple> printSelectNoJoinOnePass(int selectType, vector<string> attrVec, vector<string> tableVec,
                              vector<string> condVec, vector<string> orderByVec, SchemaManager schema_manager,
                              MainMemory &mem, bool isDistinct, bool isOrderBy, bool callFromInsert) {
  vector<Tuple> tuples;
  vector<Tuple>::iterator tupleIT;
  vector<Tuple> outputTuples;
  vector<Tuple> distinctOutputTuples;
  vector<Tuple> sortedOutputTuples;
  bool atLeastOneTuple = false;
  bool doMultiPassSelect = true;
  bool useTempRel = false;
  Relation* relation_ptr = schema_manager.getRelation(tableVec[0]);
  int numBlocksInRelation = relation_ptr->getNumOfBlocks();
  int remainingBlocksInRelation = 0;
  int lastBlockRelation = 0;
  Relation* tempRelation_ptr;
  string tempTableName = "twoPassTableDistinct"; // for DISTINCT operations
  string tempTableName2 = "twoPassTableOrderBy"; // for ORDER BY operations
  
  outputTuples.clear();
  //cout << "Inside printSelectNoJoinOnePass function" << endl;
  // If blank table, which is possible, print "empty table"
  if(numBlocksInRelation == 0) {
    cout << "No tuples in that relation, empty table" << endl;
    return outputTuples;
  }
  // Bring all the blocks in the main memory and print or store and then process them(sort, distinct)
  // Call the appropriate SELECT statement accrodingly
  // I am saving one block in memory for processing duplicates
  if(numBlocksInRelation <= NUM_OF_BLOCKS_IN_MEMORY) {
    //cout << "The number of blocks fits in MM" << endl;
    doMultiPassSelect = false;
    relation_ptr->getBlocks(0, 0, numBlocksInRelation);
    tuples=mem.getTuples(0, numBlocksInRelation);
    if(selectType == 0 || selectType == 1) { // select * from table <no where>
      if(isDistinct) { // remove the duplicates from the memory, then print
        // push tuples to outputTuples after eleminating duplicates
        //cout << "Eleminating duplicates" << endl;
        for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
          if (selectType == 0){
            checkDupTupleAllAttr(tuples, *tupleIT);
          }
          else{
            checkDupTupleSpAttr(tuples, *tupleIT, attrVec);
          } 
        }
      }
      if(isOrderBy) { // sort the tuples on orderby column(orderByAttr)
        // sort the tuples and assign to outputTuples
        // cout << "Sorting the tuple" << endl;
        sort(tuples.begin(), tuples.end(), TupleComparator(orderByVec));
      }
      outputTuples.insert(outputTuples.end(), tuples.begin(), tuples.end());
      //cout <<  "Here" << endl;
    }
    else if(selectType == 2 || selectType == 3) { // select with where condition
      for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
        if(checkTupleStatisfiesWhere(*tupleIT, condVec, (*tupleIT).getSchema())) {
          outputTuples.push_back(*tupleIT);
        }
      }
      //check for distinct keyword, should be push it before everything? - no performance hit as all tuples fit MM
      if(isDistinct){
        // cout << "Eliminating duplicates" << endl;
        for (tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
          if (selectType == 2){ //select * with where
            checkDupTupleAllAttr(outputTuples, *tupleIT);
          }
          else{
            checkDupTupleSpAttr(outputTuples,*tupleIT, attrVec);
          }
        }
      }
      //function to use for duplicate checking is checkDupTupleSpAttr
      if(isOrderBy) { // sort the tuples on orderby column(orderByAttr)
        // sort the tuples and assign to outputTuples
        //cout << "Sorting the tuple" << endl;
        sort(outputTuples.begin(), outputTuples.end(), TupleComparator(orderByVec));
      }
      //cout << "There" << endl;
    }
  }
  else { // this point we need to do a tow/multi pass 
    // Do a two pass duplicate eliminate algorithm
    // Do the first pass, sort the sublits
    //cout << "Doing mutipass" << endl;
    if(isDistinct) {
      // call sortRelationTwoPass with appropriate paramaters
      // create a temporary relation_ptr with the same schema as relation_ptr
      tempRelation_ptr = schema_manager.createRelation(tempTableName, relation_ptr->getSchema());
      // call two pass getDistinctTuples function using a temporary relation pointer tempRelation_ptr
      //cout << "Calling getDistinctTuples function" << endl;
      distinctOutputTuples = getDistinctTuples(relation_ptr, tempRelation_ptr, attrVec, mem);
     
      tempRelation_ptr->deleteBlocks(0);
      schema_manager.deleteRelation(tempTableName);
      // check if distinctOutputTuples.size() <= MM if Yes then directly apply the filter
      // else we will need to write to temporary schema
      if(distinctOutputTuples.size() <= NUM_OF_BLOCKS_IN_MEMORY) {
        doMultiPassSelect = false; // No multipass required, after DISTINCT operation all the tuples can fit MM, so we can load the whole tuples in MM and do our stuff
        // Directly Apply the select operation here
        if(selectType == 2 || selectType == 3) { // where conditions applied
          for(tupleIT = distinctOutputTuples.begin(); tupleIT != distinctOutputTuples.end(); ++tupleIT) {
            if(checkTupleStatisfiesWhere(*tupleIT, condVec, tempRelation_ptr->getSchema())) {
              outputTuples.push_back(*tupleIT);
            }
          }
        }
        else { // No where condition
          outputTuples = distinctOutputTuples; // fits in MM so below do a in place sort
        }
        if(isOrderBy) { // check if order by is there, if yes then we will need to apply it in the end
          //cout << "Sorting the tuple" << endl;
          sort(outputTuples.begin(), outputTuples.end(), TupleComparator(orderByVec)); // Do an in-memory sort(simulated)
        }
      }
      // we will need to write to schema and do two pass - for select as well as order by
      else {
        // two pass is made true, multipass is true here, do not do ORDER BY, do below else stuff
        // use the same table name as used in DISTINCT processing and writing to DB. Also set flag to two pass
        useTempRel = true;
        tempRelation_ptr = schema_manager.createRelation(tempTableName, relation_ptr->getSchema());
        for(tupleIT = distinctOutputTuples.begin(); tupleIT != distinctOutputTuples.end(); ++tupleIT){
          appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT); // Wrote the duplicate free relation to Disk/DB
        }
      }  
      // remember to clear the distinctOutputTuples here in the end  
      distinctOutputTuples.clear();
    }
    
    else if(doMultiPassSelect) { // Need to to multipass SELECT. at this point two pass distinct already applied if it was needed
      // need to do select and order by if needed
      // Not enough main memory, bring as much as possible, i.e.  NUM_OF_BLOCKS_IN_MEMORY
      //cout << "In else, numBlocksInRelation is " << numBlocksInRelation << endl;
      Schema tuple_schema;
      if(useTempRel) { // we did Multipass DISTINCT, use this temp relation Instead
        numBlocksInRelation = tempRelation_ptr->getNumOfBlocks();
        tuple_schema = tempRelation_ptr->getSchema();
      }
      else tuple_schema = relation_ptr->getSchema(); // does not matter actually, both schema are same
      
      remainingBlocksInRelation = numBlocksInRelation;
      //cout << "Doing multipass SELECT operation, numBlocksInRelation are " << numBlocksInRelation << endl;
      while(remainingBlocksInRelation > 0) {
        if (remainingBlocksInRelation >= NUM_OF_BLOCKS_IN_MEMORY) { // bring everyting in MM
          if(useTempRel) tempRelation_ptr->getBlocks(lastBlockRelation, 0, NUM_OF_BLOCKS_IN_MEMORY);
          else relation_ptr->getBlocks(lastBlockRelation, 0, NUM_OF_BLOCKS_IN_MEMORY);
          tuples=mem.getTuples(0, NUM_OF_BLOCKS_IN_MEMORY);
          //cout << "Size of the tuples is " << tuples.size() << endl;
          if(selectType == 0 || selectType == 1) { // select with no where condition
            outputTuples.insert(outputTuples.end(), tuples.begin(), tuples.end());
          }
          else if(selectType == 2 || selectType == 3) { // select with where condition
            for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
              if(checkTupleStatisfiesWhere(*tupleIT, condVec, tuple_schema)) {
                outputTuples.push_back(*tupleIT);
              }
            }
          } 
          //printTupleVec(tuples, atLeastOneTuple);
          lastBlockRelation += NUM_OF_BLOCKS_IN_MEMORY;
          remainingBlocksInRelation -= NUM_OF_BLOCKS_IN_MEMORY;
        }
        else { // Special case -> last pass, num blocks might be < NUM_OF_BLOCKS_IN_MEMORY
          //cout << "Inside else" << endl;
          if(useTempRel) tempRelation_ptr->getBlocks(lastBlockRelation, 0, remainingBlocksInRelation);
          else relation_ptr->getBlocks(lastBlockRelation, 0, remainingBlocksInRelation);
          tuples=mem.getTuples(0, remainingBlocksInRelation);
          if(selectType == 0 || selectType == 1) { // select with no where condition
            outputTuples.insert(outputTuples.end(), tuples.begin(), tuples.end());
          }
          else if(selectType == 2 || selectType == 3) { // select with where condition
            for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
              if(checkTupleStatisfiesWhere(*tupleIT, condVec, tuple_schema)) {
                outputTuples.push_back(*tupleIT);
              }
            }
          }
          break; // break the loop here, because this is the last block set of the relation
          //printTupleVec(tuples, atLeastOneTuple);
        }
      }
      //cout << "Outside While loop" << endl;
      //cout << "Printing outputTuples " << endl;
      //printTupleVector(outputTuples);
      // tempTableName delted, we are done using it
      if(useTempRel) {
        tempRelation_ptr->deleteBlocks(0);
        schema_manager.deleteRelation(tempTableName);
      }
      // Now do the order By if needed - this is either one pass if outputTuples <= MM or two pass otherwise
      if(isOrderBy) { // DISTINCT was already handled by this point of code if it was presetnt
        if(outputTuples.size() <= NUM_OF_BLOCKS_IN_MEMORY) { // one pass, do in-place memory sort
          sort(outputTuples.begin(), outputTuples.end(), TupleComparator(orderByVec));
        }
        else {  // outputTuples size is bigger than MM, so we need to first write it back to Disk and call two pass ORDER BY, cant do in place Mmemory Sort
          // first write back outputTuples to a temporary schema, then apply two pass order by and then project it back
          tempRelation_ptr = schema_manager.createRelation(tempTableName, relation_ptr->getSchema());
          for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
            appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT); // Wrote the relation to Disk/DB
          }
          Relation* tempRelation_ptr2 = schema_manager.createRelation(tempTableName2, tuple_schema);
          sortedOutputTuples = getSortedTuples(tempRelation_ptr, tempRelation_ptr2, attrVec, mem); // calling two pass ORDER BY function getSortedTuples
          tempRelation_ptr->deleteBlocks(0);
          schema_manager.deleteRelation(tempTableName);
          tempRelation_ptr2->deleteBlocks(0);
          schema_manager.deleteRelation(tempTableName2);
          outputTuples = sortedOutputTuples; // if works, combine with the statement above
        }
      }
    }
    //cout << "Printing outputTuples " << endl;
    //printTupleVector(outputTuples);
  }
  // print the tuples now from the outtuple, call the * for specific attr print.
  //cout << "trying to print something" << endl;
  /**
  if(selectType == 0 || selectType == 2) {
    printTupleVecAllAttr(outputTuples);
  }
  else if(selectType == 1 || selectType == 3) {
    printTupleVecSpecificAttr(outputTuples, attrVec);
  }
  */
  //outputTuples.clear();
  //cout << "Printing outputTuples of size " << outputTuples.size() << endl;
  //printTupleVector(outputTuples);
  //cout << endl;
  return outputTuples;
}

/**
// Sort a relation and store it back in Disk - First Pass for two pass Algo
void sortRelationTwoPass(string table, vector<string> attrVec, vector<string> tableVec,
                         vector<string> condVec, string orderByAttr, SchemaManager schema_manager,
                         MainMemory &mem, bool isDistinct, bool isOrderBy) {
  vector<Tuple> outputTuples;
  int numBlocksInRelation = 0;
  int remainingBlocksInRelation = relation_ptr->getNumOfBlocks();
  int lastBlockRelation = 0;
  int i, j, numTuples;
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
    // get all the  blocks in main memory and get a vector and sort it
    // how to take care of holes in tuples in the relation
    relation_ptr->getBlocks(lastBlockRelation, 0, numBlocksInRelation);
    outputTuples = mem.getTuples(0, numBlocksInRelation);
    //sort this tuples
    
    
    for(i = 0; i < numBlocksInRelation; i++) {
      Block* block_ptr=mem.getBlock(i);
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
        //cout << "Many tuple per block which is: " << numTuples<< endl;
        for(j = 0; j < numTuples; j++) {
          Tuple tuple = block_ptr->getTuple(j);
          if(tuple.isNull()) continue; // this is a hole in MM block
          // delete tuple if it satisfies where condition
          if(checkTupleStatisfiesWhere(tuple, condVec, tuple_schema)) {
            block_ptr->nullTuple(j);
          }
        }
      }
    }
    relation_ptr->setBlocks(lastBlockRelation, 0, numBlocksInRelation);
    remainingBlocksInRelation -= NUM_OF_BLOCKS_IN_MEMORY;
    lastBlockRelation += NUM_OF_BLOCKS_IN_MEMORY;
  }
}
*/

// print select statements with specific attributes
void printTupleVecSpecificAttr(vector<Tuple> tuples, vector<string> attrVec) {
  //cout << "Inside printTupleVecSpecificAttr function " << endl;
  //cout << "Size of tuple is " << tuples.size() << endl;
  //cout << "AttrVec is " << endl;
  //printVector(attrVec);
  if(tuples.size() == 0) {
    cout << "Empty Set, no tuples returned" << endl;
    return;
  }
  bool atLeastOneTuple = false;
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;
  Schema tuple_schema = tuples[0].getSchema();
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    // print the headder of the select statement
    // maintain the order mentioned in the query, use attrVec
    if(!atLeastOneTuple) {
      for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
        cout << *vit << "\t";
      }
      cout << endl;
      atLeastOneTuple = true;
    }
    for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
      if(tuple_schema.getFieldType(*vit) == INT)
        cout << tupleIT->getField(*vit).integer << "\t";
      else
        cout << *(tupleIT->getField(*vit).str) << "\t";
    }
    cout << endl;
  }
}

// print tuples for where condition
// print tuples that satisfies where condition
// TODO - Not used, but good - refer it
/**
void printTupleVecWhereCondSpAttr(vector<Tuple> tuples, vector<string> attrVec,
                                  vector<string> condVec, bool &atLeastOneTuple) {
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;
  Schema tuple_schema = tuples[0].getSchema();
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    // print the headder of the select statement
    // maintain the order mentioned in the query, use attrVec
    if(!atLeastOneTuple) {
      for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
        cout << *vit << "\t";
      }
      cout << endl;
      atLeastOneTuple = true;
    }

    if(checkTupleStatisfiesWhere(*tupleIT, condVec, tuple_schema)) {
      //cout << "Tuple satisfies the WHERE condition, printing:" << endl;
      for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
        if(tuple_schema.getFieldType(*vit) == INT)
          cout << tupleIT->getField(*vit).integer << "\t";
        else
          cout << *(tupleIT->getField(*vit).str) << "\t";
      }
      cout << endl;
    }
  }
}
*/
// print tuples that satisfies where condition with all the attr
// select * from table where <condition>
// TODO - Not used, but good - refer it
/**
void printTupleVecWhereCondAllAttr(vector<Tuple> tuples, vector<string> condVec,
                                   bool &atLeastOneTuple) {
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;
  Schema tuple_schema = tuples[0].getSchema();
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
    // print the headder of the select statement
    // maintain the order mentioned in the query, use attrVec
    if(!atLeastOneTuple) {
      Schema tuple_schema = tupleIT->getSchema();
      vector<string> fieldVec = tuple_schema.getFieldNames();
      for(vit = fieldVec.begin(); vit != fieldVec.end(); ++vit) {
        cout << *vit << "\t";
      }
      cout << endl;
      atLeastOneTuple = true;
    }

    if(checkTupleStatisfiesWhere(*tupleIT, condVec, tuple_schema)) {
      //cout << "Tuple satisfies the WHERE condition, printing:" << endl;
      printTuple(*tupleIT);
      cout << endl;
    }
  }
}
*/

// for statement like select * from table
// We need to first read each tuple to the main memory and then print it?
// check if number of blocks in the relation is less than or equal to the number of block in MM
// projection implementation - One Pass -> Single pass projection operation

// print select statements with * in the attribute list
void printTupleVecAllAttr(vector<Tuple> tuples) {
  if(tuples.size() == 0) {
    cout << "Empty Set, no tuples returned" << endl;
    return;
  }
  bool atLeastOneTuple = false;
  //cout << "Inside printTupleVec" << endl;
  vector<Tuple>::iterator tupleIT;
  vector<string>::iterator vit;
  for(tupleIT = tuples.begin(); tupleIT != tuples.end(); ++tupleIT) {
      // print the schema for the relation, header
    if(!atLeastOneTuple) {
      Schema tuple_schema = tupleIT->getSchema();
      vector<string> fieldVec = tuple_schema.getFieldNames();
      for(vit = fieldVec.begin(); vit != fieldVec.end(); ++vit) {
        cout << *vit << "\t";
      }
      cout << endl;
      atLeastOneTuple = true;
    }
    printTuple(*tupleIT);
    cout << endl;
  }
  return;
}

// To print tuple which satisfies select * from a where c > 10 
void printAllTupleAttr(Tuple tuple) {
  vector<string>::iterator vit;
  Schema tuple_schema = tuple.getSchema();
  // since select * get names of all the attributes and print them in order
  vector<string> attrVec = tuple_schema.getFieldNames();
  for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
    if(tuple_schema.getFieldType(*vit) == INT)
      cout << tuple.getField(*vit).integer << "\t\t";
    else
      cout << *(tuple.getField(*vit).str) << "\t\t";
  }
  cout << endl;
}

// TODO: Unused: To print schema heading for select statement like select sid, grade from table......
void printSpecificSchema(vector<string>attrVec, Tuple tuple) {
  Schema tuple_schema = tuple.getSchema();
  vector<string>::iterator vit;
  for(vit=attrVec.begin(); vit!=attrVec.end(); ++vit) {
    cout << "Do Something " << endl;
  }
}

