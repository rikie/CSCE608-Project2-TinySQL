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
#include "helper.h"
#include "helpInsert.h"
#include "selectJoinUtil.h"
#include "evalWhereConds.h"

using namespace std;

// Processing select statements when we have more than a table involved
// tableVec.size() > 1
void processSelectJoin(int typeSelect, vector<string> attrVec, vector<string> tableVec,
                       vector<string> condVec, vector<string>, SchemaManager schema_manager,
                       MainMemory &mem, bool isDistinct, bool isOrderBy, bool callFromInsert) {
  vector<Tuple> outputTuples;
  vector<string> relationNames;
  vector<string> joinConds;
  vector<Tuple>::iterator tupleIT;
  vector<Tuple> tempOutputTuples;
  vector<string>::iterator sit;
  vector<string>::iterator tempVit;
  vector<string>::iterator vit;
  bool writeToDB = false;
  bool isTable1Big = false;
  bool equiJoin = false;
  bool twoPassDistinctDone = false;
  string tempTableName = "tempTable";
  int i = 2;
  int joinCondCount = 0;
  int numCondtions = 0;
  ostringstream oss;
  // if typeSelect == 10 -> cross joiN TODO get a good numbering scheme
  // count tables? two table good, more than two
  // need to store two table cross join in a new schema
  // push this in disk and then do it
  // 
  // select * from r, s, t and no where condition
  // create a temporaty schema or no depending if we need to do order by, distinct 
  // and the resulting schema after join is bigger than MM
  //cout << "Inside processSelectJoin function" << endl;

  // if we have more than one table we need to save it temporarily
  if(tableVec.size() > 2) {
    writeToDB = true;
    //cout << "More than one table " << endl;
  }
  // pass two tables first and then keep on pushing leftover tables
  //Relation* relation_ptr1 =schema_manager.getRelation(tableVec[0]);
  //Relation* relation_ptr2 =schema_manager.getRelation(tableVec[1]);
  //TODO erase the two entries using erase for vector
  //Bring the smaller relation to the main memory if it fits it
  //int relation  = relation_ptr->getNumOfBlocks();
  // returns a relation with the tuples for the cross join
  
  // We have more than 2 tables in join
  // Write back to db the intermediate results
  // we get the joins from the condVec and then set those conditions to true
  if(typeSelect == 12 || typeSelect == 13) {
    joinConds = getJoinConditions(condVec);
    if(joinConds.size() > 0) {
      equiJoin = true;
      numCondtions = joinConds.size();
    }
    //try getJoinCompConditions
    //getJoinCompConditions(condVec, "test");
  }
  // depending on the order of joins we need to reorder the tableVec
  // SELECT * FROM r, s, t WHERE r.a = t.a AND r.b = s.b AND s.c = t.c
  // reorder to join r and t - then result to s and then result to t
  // FROM r, t - use r.a = t.a then s use - r.b = s.b 
  // extra join condition remain then elimiate those that do not satisfy in the end result
  vector<string> tempTableVec;
  if(equiJoin) {
    for(vit = joinConds.begin(); vit != joinConds.end(); ++vit) {
      string tempTableName;
      vector<string> tempVec = splitString(*vit, "= ");
      tempTableName = splitString(tempVec[0], ". ")[0];
      if(tempTableVec.size() == 0) tempTableVec.push_back(tempTableName);
      else {
        //find if it exists, if not then add to the tempTableVec vector
        sit = find(tempTableVec.begin(), tempTableVec.end(), tempTableName);
        if(sit == tempTableVec.end()) tempTableVec.push_back(tempTableName);
      }
      tempTableName = splitString(tempVec[1], ". ")[0];
      sit = find(tempTableVec.begin(), tempTableVec.end(), tempTableName);
      if(sit == tempTableVec.end()) tempTableVec.push_back(tempTableName);
    }
    tableVec = tempTableVec;
  }
  
  string table1 = tableVec[0];
  string table2 = tableVec[1];
  Schema schema;
  int sizeRel1 = (schema_manager.getRelation(table1))->getNumOfBlocks();
  int sizeRel2 = (schema_manager.getRelation(table2))->getNumOfBlocks();
  Relation* smallRelation;
  Relation* bigRelation;
  if(sizeRel1 < sizeRel2) {
    smallRelation = schema_manager.getRelation(table1);
    bigRelation = schema_manager.getRelation(table2);
    schema = createSchemaAllAttrJoinTables(table2, table1, schema_manager);
  }
  else {
    smallRelation = schema_manager.getRelation(table2);
    bigRelation = schema_manager.getRelation(table1);
    schema = createSchemaAllAttrJoinTables(table1, table2, schema_manager);
  }
  //Schema schema = createSchemaAllAttrJoinTables(bigRelation->getSchema(), smallRelation->getSchema, schema_manager);
  // TODO: get a new name everytime -> use a pool of names?
  //Relation* tempRelation_ptr = schema_manager.createRelation(tempTableName, schema);
  //relationNames.push_back(tempTableName);
  Relation* tempRelation_ptr = schema_manager.createRelation(tempTableName, schema);
  //vector<Tuple> processCrossJoinOnePass(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, SchemaManager schema_manager, MainMemory &mem, bool writeToDB)
  // Main logic -> if more then 2 table we need to reuse 
  // we are sure there is at least one join of two tables
  // My late night addition starts //
  string leftKey, rightKey, key1, key2;
  if((typeSelect == 12 || typeSelect == 13) && equiJoin) {
    // get the two keys for the first join
    // the keys will be ordered along wiht the relation order?
    //joinConds = getJoinConditions(condVec);
    vector<string> joinCondsVec = splitString(joinConds[joinCondCount],"= ");
    leftKey = joinCondsVec[0];
    rightKey = joinCondsVec[1];
    // take care then temp will be appended with relation name
    if(splitString(leftKey, ".")[0].compare(smallRelation->getRelationName()) == 0) {
      key2 = splitString(leftKey, ". ")[1];
      key1 = splitString(rightKey, " .")[1];
    }
    else {
      key1 = splitString(leftKey, ". ")[1];
      key2 = splitString(rightKey, " .")[1];
    }
    joinCondCount++;
  }
  if((typeSelect == 12 || typeSelect == 13) && equiJoin) {  
    //cout << "Size of relation 1 is " <<  sizeRel1 << " and eixe of relation 2 is " << sizeRel2 << endl;
    // constraint satisfies two pass natural join
    if(sizeRel1 > NUM_OF_BLOCKS_IN_MEMORY && sizeRel2 > NUM_OF_BLOCKS_IN_MEMORY &&
      ((sizeRel1 / NUM_OF_BLOCKS_IN_MEMORY) + (sizeRel2 / NUM_OF_BLOCKS_IN_MEMORY)) < 
      NUM_OF_BLOCKS_IN_MEMORY) {
      cout << "calling processNaturalJoinTwoPass" << endl;
      outputTuples = processNaturalJoinTwoPass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                               mem, key1, key2, writeToDB);
    }
    //cout << "calling processNaturalJoinOnePass" << endl;
    else if(sizeRel1 <= NUM_OF_BLOCKS_IN_MEMORY - 1 || sizeRel2 <= NUM_OF_BLOCKS_IN_MEMORY - 1){
      outputTuples = processNaturalJoinOnePass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                               mem, key1, key2, writeToDB);
    }
    else {
      cout << "Both the relations are very big, need to do nested loop NATURAL JOIN" << endl;
      outputTuples = processNaturalJoinGeneric(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                               mem, key1, key2, writeToDB);                    
    }
  }
  else {
    // remove writeToDB from function headder, not used in the called function
    cout << "calling processCrossJoinOnePass" << endl;
    outputTuples = processCrossJoinOnePass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                   mem, writeToDB);
  }
  if(writeToDB) {
    // More than two tables, do something here
    //cout << "Writing to the DB" << endl;
    //printVector(tableVec);
    //cout << endl;
    while(i < tableVec.size()) {
      //cout << "Inside while" << endl;
      // Writing the relation to DB and saving the relation name to delete later
      relationNames.push_back(tempRelation_ptr->getRelationName());
      for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
        appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT);
      }
      string table1 = tempRelation_ptr->getRelationName();
      //cout << "Inside while and the name of my table is " << table1 << endl;
      string table2 = tableVec[i];
      int sizeRel1 = (schema_manager.getRelation(table1))->getNumOfBlocks();
      int sizeRel2 = (schema_manager.getRelation(table2))->getNumOfBlocks();
      if(sizeRel1 < sizeRel2) {
        smallRelation = schema_manager.getRelation(table1);
        bigRelation = schema_manager.getRelation(table2);
        // bigrelation + smallrelation, bigrelation is in outer loop
        schema = createSchemaAllAttrJoinTables(table2, table1, schema_manager);
      }
      else {
        isTable1Big = true;
        smallRelation = schema_manager.getRelation(table2);
        bigRelation = schema_manager.getRelation(table1);
        schema = createSchemaAllAttrJoinTables(table1, table2, schema_manager);
      }
      // need to create a unique schema name everytime, use i, convert to string and append
      oss << i;
      //relationNames.push_back(oss.str() + tempTableName);
      tempRelation_ptr = schema_manager.createRelation(oss.str() + tempTableName, schema);
      
      if((typeSelect == 12 || typeSelect == 13) && equiJoin) {
        // get the two keys for the first join
        // the keys will be ordered along wiht the relation order?
        // in this case table2 will be the fresh table, r.a, or t.c, so c is good for search value
        // table1 is problemetic -> from tempRelation_ptr , we need to do something here -> the extended tuple.
        // table 1 tuples' schema will be like r.a or temptable.r.a or 2temptable.r.a etc.
        // I might be alll wrong on the stuff written above.....sorry.
        // If anyone reads this you have won $10!!!
        vector<string> joinCondsVec = splitString(joinConds[joinCondCount],"= ");
        leftKey = joinCondsVec[0];
        rightKey = joinCondsVec[1];
        // take care then temp will be appended with relation name
        /**
        vector<string> tempRelNameVec = splitString(smallRelation->getRelationName(), ". ");
        string relNameToCmp;
        if(tempRelNameVec.size() > 1) relNameToCmp = tempRelNameVec[tempRelNameVec.size()-1];
        else relNameToCmp = tempRelNameVec[0];
        if(splitString(leftKey, ".")[0].compare(relNameToCmp)== 0) {
        */
        //cout << endl << endl;
        //cout << "leftKey is : " << leftKey << " rightKey is: " << rightKey << endl;
        // tempRelation_ptr->getSchema().printSchema();
        // cout << endl;
        // use the schema of table 1 to find keys
        vector<string> table1FieldNames = schema_manager.getSchema(table1).getFieldNames();
        // printVector(table1FieldNames);
        // big relation key 1: remember
        // tempSchema -> table1 -> key2 = leftey
        for(vit = table1FieldNames.begin(); vit != table1FieldNames.end(); ++vit) {
          vector<string> tempVec = splitString(*vit, ". ");
          string tempSchema = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
          if(leftKey.compare(tempSchema) == 0){
            if(isTable1Big) { key1 = *vit; key2 = splitString(leftKey, ".")[1]; }
            else { key2 = *vit; key1 = splitString(leftKey, ".")[1]; }
            break;
          }
          else if (rightKey.compare(tempSchema) == 0){
            if(isTable1Big) { key1 = *vit; key2 = splitString(leftKey, ".")[1]; }
            else { key2 = *vit; key1 = splitString(leftKey, ".")[1]; }
            break;
          }
        }
        //cout << "Final keys are: " << key1 << " and " << key2 << endl;
        //// changes .....above
        /**
        cout << "Table 1 is " << table1 << endl;
        if(splitString(leftKey, ".")[0].compare(table2) == 0) {
          key2 = splitString(leftKey, ". ")[1];
          key1 = splitString(rightKey, " .")[1];
        }
        else {
          key1 = splitString(leftKey, ". ")[1];
          key2 = splitString(rightKey, " .")[1];
        }
        */
        joinCondCount++;
        //cout << "Multiple tables detected, inside While loop" << endl;
        //cout << "Size of relation 1 is " <<  sizeRel1 << " and eixe of relation 2 is " << sizeRel2 << endl;
        // constraint satisfies two pass natural join
        if(sizeRel1 > NUM_OF_BLOCKS_IN_MEMORY && sizeRel2 > NUM_OF_BLOCKS_IN_MEMORY &&
          ((sizeRel1 / NUM_OF_BLOCKS_IN_MEMORY) + (sizeRel2 / NUM_OF_BLOCKS_IN_MEMORY)) < 
          NUM_OF_BLOCKS_IN_MEMORY) {
          cout << "calling processNaturalJoinTwoPass" << endl;
          outputTuples = processNaturalJoinTwoPass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                                   mem, key1, key2, writeToDB);
        }
        else if(sizeRel1 <= NUM_OF_BLOCKS_IN_MEMORY || sizeRel2 <= NUM_OF_BLOCKS_IN_MEMORY){
          outputTuples = processNaturalJoinOnePass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                                   mem, key1, key2, writeToDB);
        }
        else {
          cout << "Both the relations are very big, need to do nested loop NATURAL JOIN" << endl;
          outputTuples = processNaturalJoinGeneric(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
                                               mem, key1, key2, writeToDB);
        }          
          
        //outputTuples = processNaturalJoinOnePass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, 
        //                                         mem, key1, key2, writeToDB);
      }
      else
        outputTuples = processCrossJoinOnePass(bigRelation, smallRelation, tempRelation_ptr, schema_manager, mem, writeToDB);
      //outputTuples.insert(outputTuples.end(), resTuples.begin(), resTuples.end());
      i++;
    }
  }
  // if equijoin and more conditions remain
  if((typeSelect == 12 || typeSelect == 13) && equiJoin) {
    if(joinCondCount != numCondtions) {
      vector<string> table1FieldNames = tempRelation_ptr->getSchema().getFieldNames();
    // filter outputTuples with these conditions
    // TODO: Do we need to write in a relation and then do the operations below?
      string origLeftKey, origRightKey;
      //tempRelation_ptr->printRelation(); 
      while(joinCondCount != numCondtions) {
        vector<Tuple> tempOutputTuples;
        vector<string> joinCondsVec = splitString(joinConds[joinCondCount],"= ");
        leftKey = joinCondsVec[0];
        rightKey = joinCondsVec[1];
        joinCondCount++;
        for(vit = table1FieldNames.begin(); vit != table1FieldNames.end(); ++vit) {
          vector<string> tempVec = splitString(*vit, ". ");
          string tempSchema = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
          if(leftKey.compare(tempSchema) == 0){
            origLeftKey = *vit;
          }
          if (rightKey.compare(tempSchema) == 0){
            origRightKey = *vit;
          }
        }
        for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT) {
          // check the two values are same or not for each tuple
          if(checkTwoFieldsTuple(*tupleIT, origLeftKey, origRightKey))
            tempOutputTuples.push_back(*tupleIT);
        }
        outputTuples = tempOutputTuples;
        //cout << "Origleft key and orig rightkey are " << origLeftKey << " " << origRightKey << endl;
      }
    }
  }
  if(outputTuples.size() == 0) {
    cout << "Empty Set, no tuples returned" << endl;
    return;
  }
  // Applying Distinct and Order BY
  if(isDistinct) {
    // distinct by attrVec
    // we need to find the attrVec for the particular Schema match
    // tempRelation_ptr is the schema of the original Schema
    //vector<string> table1FieldNames = tempRelation_ptr->getSchema().getFieldNames();
    vector<string> distinctAttrVec;
    vector<string> fieldNames = outputTuples[0].getSchema().getFieldNames();
    //tempRelation_ptr->printRelation();
    for(vit = attrVec.begin(); vit != attrVec.end(); ++vit) {
      for(tempVit = fieldNames.begin(); tempVit != fieldNames.end(); ++tempVit) {
        string realFieldName;
        vector<string> tempVec = splitString(*tempVit, ".");
        if(tempVec.size() > 2)
            realFieldName = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
        else
          realFieldName = *tempVit;
        if((*vit).compare(realFieldName) == 0) {
          // use the *tempVit to find distinct tuples
          distinctAttrVec.push_back(*tempVit);
          //cout << *vit << endl;
          break;
        }
      }
    }
    if(outputTuples.size() <= NUM_OF_BLOCKS_IN_MEMORY - 1) { // do in memory distinct removal
      for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT) {
        if(typeSelect == 10 || typeSelect == 12)
          checkDupTupleAllAttr(outputTuples, *tupleIT);
        else
          checkDupTupleSpAttr(outputTuples, *tupleIT, distinctAttrVec);
      }
    }
    else { // do a two pass distinct algorithm
      // write the result of join to the relation ptr first if needed
      cout << "Doing two pass Distinct in join condtions" << endl;
      string tempTableDistinct = "tempTableDistinct";
      Relation* tempRelation_ptr_Distinct = schema_manager.createRelation(tempTableDistinct, tempRelation_ptr->getSchema());
      // we need to dump the outputTuples to tempRelation_ptr if writeToDB was false
      // else it was already written --> TODO need to check DISTINCT for 3 tables
      // this is because our two pass algorithm expects a relation to sort and a relation to keep the sorted result
      //cout << "outputTuples size is " << outputTuples.size() << endl;
      //cout << "getTuplesPerBlock is " << outputTuples[0].getTuplesPerBlock() << endl;
      if (tempRelation_ptr->getNumOfBlocks() != 0)
        tempRelation_ptr->deleteBlocks(0);
      //if(!writeToDB) {
        for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
          appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT);
        }
      //}
      outputTuples = getDistinctTuples(tempRelation_ptr, tempRelation_ptr_Distinct, distinctAttrVec, mem);
      if(tempRelation_ptr_Distinct->deleteBlocks(0) && schema_manager.deleteRelation(tempTableDistinct)) cout << "Table " << tempTableDistinct << " successfully deleted/dropped" << endl;
      else cout << "Table " << tempTableDistinct << " not found" << endl;
      //if(!writeToDB) {
      //  if(tempRelation_ptr->deleteBlocks(0) && schema_manager.deleteRelation(tempRelation_ptr->getRelationName())) cout << "Table " << tempRelation_ptr->getRelationName() << " successfully deleted/dropped" << endl;
      //  else cout << "Table " << tempRelation_ptr->getRelationName() << " not found" << endl;
     // }
    }
    //cout << "Interesting " << endl;
    //printVector(distinctAttrVec);
  }
  
  if(isOrderBy) {
    vector<string> orderByAttrVec;
    vector<string> fieldNames = outputTuples[0].getSchema().getFieldNames();
    for(vit = orderByAttrVec.begin(); vit != orderByAttrVec.end(); ++vit) {
      for(tempVit = fieldNames.begin(); tempVit != fieldNames.end(); ++tempVit) {
        string realFieldName;
        vector<string> tempVec = splitString(*tempVit, ".");
        if(tempVec.size() > 2)
            realFieldName = tempVec[tempVec.size()-2] + "." + tempVec[tempVec.size()-1];
        else
          realFieldName = *tempVit;
        if((*vit).compare(realFieldName) == 0) {
          // use the *tempVit to find distinct tuples
          orderByAttrVec.push_back(*tempVit);
          //cout << *vit << endl;
          break;
        }
      }
    }
    // cout << "ORDER BY vec is " << orderByAttrVec << endl;
    // do the order by here
    if(outputTuples.size() <= NUM_OF_BLOCKS_IN_MEMORY - 1) { // do in memory sorting
      sort(outputTuples.begin(), outputTuples.end(), TupleComparator(orderByAttrVec)); // Do an in-memory sort(simulated)
    }
    else { // we need to do two pass sorting, tuples cannot fit in MM
      cout << "Doing two pass Order/sorting in join condtions" << endl;
      string tempTableSort = "tempTableSort";
      Relation* tempRelation_ptr_Sort = schema_manager.createRelation(tempTableSort, tempRelation_ptr->getSchema());
      // we need to dump the outputTuples to tempRelation_ptr if writeToDB was false
      // else it was already written --> TODO need to check DISTINCT for 3 tables
      // this is because our two pass algorithm expects a relation to sort and a relation to keep the sorted result
      if (tempRelation_ptr->getNumOfBlocks() != 0)
        tempRelation_ptr->deleteBlocks(0);
      //if(!writeToDB) {
        for(tupleIT = outputTuples.begin(); tupleIT != outputTuples.end(); ++tupleIT){
          appendTupleToRelation(tempRelation_ptr, mem, 0, *tupleIT);
       // }
      }
      outputTuples = getSortedTuples(tempRelation_ptr, tempRelation_ptr_Sort, orderByAttrVec, mem); // calling two pass ORDER BY function getSortedTuples
      if(tempRelation_ptr_Sort->deleteBlocks(0) && schema_manager.deleteRelation(tempTableSort)) cout << "Table " << tempTableSort << " successfully deleted/dropped" << endl;
      else cout << "Table " << tempTableSort << " not found" << endl;    
      //if(!writeToDB) {
      //  if(tempRelation_ptr->deleteBlocks(0) && schema_manager.deleteRelation(tempRelation_ptr->getRelationName())) cout << "Table " << tempRelation_ptr->getRelationName() << " successfully deleted/dropped" << endl;
      //  else cout << "Table " << tempRelation_ptr->getRelationName() << " not found" << endl;
     //}
    }
  }
  
  // Print the outputTuples.
  // Different logic to print if we have * in attribute list or column names in attribute list
  // printing the schema and the tuples of the cross product output
  // The final result is in the schema tempTableName
  // TODO need to find a better name for the final Schema
  // temporary differenciation for printing cross product, in the final version make separate functions to print4
  if(typeSelect == 11 || typeSelect == 13) {
    // print the column names
    for(vit = attrVec.begin(); vit != attrVec.end(); ++vit) {
      cout << *vit << "\t";
    }
    cout << endl;
    // Now print the values of the columns in specified order
    // get the schema of the final result relation
    // TODO flaw what if only 2 tables and no i in tablename
    // use tuple to get schema?
    Schema tuple_schema = schema_manager.getSchema(oss.str() + tempTableName);
    vector<string>fieldNames = tuple_schema.getFieldNames();
    printJoinedTupleSpAttr(outputTuples, fieldNames, attrVec, tuple_schema, condVec, typeSelect);
    if(relationNames.size() > 0) {
      for(vit = relationNames.begin(); vit != relationNames.end(); ++vit) {
        if(schema_manager.getRelation(*vit)->deleteBlocks(0) && schema_manager.deleteRelation(*vit))
          cout << "Table " << *vit << " successfully deleted/dropped DHONG" << endl;
        else
          cout << "Table " << *vit << " not found" << endl;
      }
      cout << endl;
    }
    relationNames.clear();
    return;
  }
  else {
    vector<string>fieldNames = schema_manager.getSchema(oss.str() + tempTableName).getFieldNames();
    //cout << "Calling printJoinedTupleAllAttr " << endl;
    printJoinedTupleAllAttr(outputTuples, fieldNames, condVec, typeSelect);
    //Delete the relations for future use
    if(relationNames.size() > 0) {
      //cout << "Vector Size " << relationNames.size() << endl;
      //printVector(relationNames);
      for(vit = relationNames.begin(); vit != relationNames.end(); ++vit) {
      if(schema_manager.getRelation(*vit)->deleteBlocks(0) && schema_manager.deleteRelation(*vit))
        cout << "Table " << *vit << " successfully deleted/dropped " << endl;
      else
        cout << "Table " << *vit << " not found" << endl;
      }
      //cout << endl;
    }
    relationNames.clear();
    //cout << "last try: " << endl;
    //printVector(condVec);
    return;
  }

  return;
}

//vector<Tuple> processCrossJoinOnePass(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, 
//                                      SchemaManager schema_manager, MainMemory &mem, bool writeToDB) {
// Not using this function for now

// process select * from r, s, t, .......... <no where>
vector<Tuple> processCrossJoinOnePass(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, 
                                      SchemaManager schema_manager, MainMemory &mem, bool writeToDB) {
  //cout << "Inside processCrossJoinOnePass function" << endl;  
  // bring in MM the smaller realtion
  vector<Tuple> smallRelationTuples;
  vector<Tuple> bigRelationTuples;
  vector<Tuple> outputTuples;
  vector<Tuple>::iterator smallTupleIT;
  vector<Tuple>::iterator bigTupleIT;
  int blocksInBigRelation = bigRelation->getNumOfBlocks();
  int i;
  //smallRelation->printRelation(); cout << endl;
  //bigRelation->printRelation(); cout << endl;
  for(i = 0; i < blocksInBigRelation; i++) {
    //cout << " i is " << i << endl;
    //get one block for big relation and bring in MM
    bigRelation->getBlock(i,0);
    bigRelationTuples = mem.getTuples(0,1);
    int remainingBlocksInSmallRelation = smallRelation->getNumOfBlocks();
    int lastBlockSmallRelation = 0;
    int numBlocksInSmallRelation;
    // get the smaller relation blocks in MM - 1
    while(remainingBlocksInSmallRelation > 0) {
      if(remainingBlocksInSmallRelation > NUM_OF_BLOCKS_IN_MEMORY - 1) {
        numBlocksInSmallRelation = NUM_OF_BLOCKS_IN_MEMORY - 1;
      }
      else {
        numBlocksInSmallRelation = remainingBlocksInSmallRelation;
      }
      smallRelation->getBlocks(lastBlockSmallRelation, 1, numBlocksInSmallRelation);
      smallRelationTuples=mem.getTuples(1, numBlocksInSmallRelation);
      for(bigTupleIT = bigRelationTuples.begin(); bigTupleIT != bigRelationTuples.end(); ++bigTupleIT) {
        for(smallTupleIT = smallRelationTuples.begin(); smallTupleIT != smallRelationTuples.end(); ++smallTupleIT) {
          //cout << "Checking "; printTuple(*bigTupleIT); cout << " and "; printTuple(*smallTupleIT); cout << endl;
          Tuple tempTuple = getJoinedTuple(*bigTupleIT, *smallTupleIT, tempRelation_ptr); // decide the params
          outputTuples.push_back(tempTuple);
        }
      }
      remainingBlocksInSmallRelation -= (NUM_OF_BLOCKS_IN_MEMORY - 1);
      lastBlockSmallRelation += (NUM_OF_BLOCKS_IN_MEMORY - 1); 
    }
  }
  return outputTuples;
}

// select * from r, s, t 
Schema createSchemaAllAttrJoinTables(string table1, string table2, SchemaManager schema_manager) {
  //cout << "Inside createSchemaAllAttrJoinTables function" << endl;
  vector<string> field_names;
  vector<enum FIELD_TYPE> field_types;
  vector<string>::iterator vit;
  vector<enum FIELD_TYPE>::iterator fieldIT;
  
  // Get the schema and then get the table 1 and 2 field names and types respectively
  Schema schemaTable1 = schema_manager.getSchema(table1);
  Schema schemaTable2 = schema_manager.getSchema(table2);
  vector<string> fieldNamesTable1 = schemaTable1.getFieldNames();
  vector<string> fieldNamesTable2 = schemaTable2.getFieldNames();
  vector<enum FIELD_TYPE> fieldTypesTable1 = schemaTable1.getFieldTypes();
  vector<enum FIELD_TYPE> fieldTypesTable2 = schemaTable2.getFieldTypes();
  //schemaTable1.printSchema();
  // need to use . because duplicate names not allowed in schema
  // TODO: if already . present in original statement what to do? like
  // select select r.a, s.b .....
  for(vit = fieldNamesTable1.begin(); vit != fieldNamesTable1.end(); ++vit) {
    field_names.push_back(table1 + "." + *vit);
  }
  for(vit = fieldNamesTable2.begin(); vit != fieldNamesTable2.end(); ++vit) {
    field_names.push_back(table2 + "." + *vit);
  }
  for(fieldIT = fieldTypesTable1.begin(); fieldIT != fieldTypesTable1.end(); ++fieldIT) {
    field_types.push_back(*fieldIT);
  }
  for(fieldIT = fieldTypesTable2.begin(); fieldIT != fieldTypesTable2.end(); ++fieldIT) {
    field_types.push_back(*fieldIT);
  }
  // TODO do we need to append the table name in front of the attr name?
  // We hope that number of columns is < 8 , or big problem
  // check?
  if(field_names.size() > 8)
    cout << "Error: columns exceeed more than 8 per schema" << endl;
  //create new schema
  Schema schema = Schema(field_names,field_types);
  //cout << "The new schema is: " << endl;
  //schema.printSchema();
  return schema;
}

Schema createSchemaSpAttrJoinTables(string table1, string table2, vector<string> attrVec, SchemaManager schema_manager) {
  //cout << "Inside createSchemaAllAttrJoinTables function" << endl;
  vector<string> field_names;
  vector<enum FIELD_TYPE> field_types;
  vector<string>::iterator vit;
  vector<enum FIELD_TYPE>::iterator fieldIT;
  string tempTableName;
  string tempFieldName;
  vector<string> tempVector;
  // Get the schema and then get the table 1 and 2 field names and types respectively
  Schema schemaTable1 = schema_manager.getSchema(table1);
  Schema schemaTable2 = schema_manager.getSchema(table2);
  // Assumption, in joins we assume, the attr names are like r.a, s.b, i.e. there is a .
  //get table and attribute names
  for (int i = 0; i < attrVec.size(); i++){
     tempVector = splitString(attrVec[i], ".");
     tempTableName = tempVector[tempVector.size()-2];
     tempFieldName = tempVector[tempVector.size()-1];
     Schema tempSchema = schema_manager.getSchema(tempTableName);
     // get the field type for this table.field and push to the field_types
     field_types.push_back(tempSchema.getFieldType(tempFieldName));
     field_names.push_back(attrVec[i]);
     tempVector.clear();
  }
  // TODO do we need to append the table name in front of the attr name? -> DONE
  // We hope that number of columns is < 8 , or big problem
  // check?
  if(field_names.size() > 8)
    cout << "Error: columns exceeed more than 8 per schema" << endl;
  //create new schema
  Schema schema = Schema(field_names,field_types);
  //cout << "The new schema is: " << endl;
  //schema.printSchema();
  return schema;
}

Tuple getJoinedTuple(Tuple t1, Tuple t2, Relation* &newRelation_ptr) {
  //cout << "Inside getJoinedTuple function" << endl;
  vector<string>::iterator vit;
  Schema schemaT1 = t1.getSchema();
  Schema schemaT2 = t2.getSchema();
  //cout << "Schema of T1 is: " << endl;
  //schemaT1.printSchema();
  //cout << "Schema of T2 is: " << endl;
  //schemaT2.printSchema();
  Tuple tuple = newRelation_ptr->createTuple();
  //cout << "Schema of the new relation is: " << endl;
  //newRelation_ptr->getSchema().printSchema();
  int numFields = tuple.getNumOfFields();
  //cout << "Number of fields in the newRelation_ptr is: " << numFields << endl;
  int attrPos = 0; // offset for new Tuple
  int tempOffset = 0; //offset for t1 and t2
  int i;
  // banking on the fact that the order of scheman of new tuple = t1 + t2
  // just fetch serially and add to new tuple, fields should match
  // Process T1, We can also use vitg as field name to retrieve values other than offset
  // night coding just realized the above stuff!!
  //for(vit = schemaT1.getFieldNames().begin(); vit != schemaT1.getFieldNames().end(); ++vit) {
  // for the above loop, there was a bug for the statement, inifite loop:
  // SELECT * FROM threeTuple, twoTuple
  for(i = 0; i < t1.getSchema().getNumOfFields();  i++) {
    //cout << "Processing T1" << endl;
    //cout << "attrPos is : " << attrPos << " and tempOffset is: " << tempOffset << endl;
    if (schemaT1.getFieldType(tempOffset)==INT)
      tuple.setField(attrPos, t1.getField(tempOffset).integer);
	  else
	    tuple.setField(attrPos, *(t1.getField(tempOffset).str));
    attrPos++;
    tempOffset++;
  }
  tempOffset = 0;
  // Process T2
  //for(vit = schemaT2.getFieldNames().begin(); vit != schemaT2.getFieldNames().end(); ++vit) {
    //cout << "Processing T2" << endl;
    //cout << "attrPos is : " << attrPos << " and tempOffset is: " << tempOffset << endl;
  for(i = 0; i < t2.getSchema().getNumOfFields();  i++) {
    if (schemaT2.getFieldType(tempOffset)==INT)
      tuple.setField(attrPos, t2.getField(tempOffset).integer);
	  else
	    tuple.setField(attrPos, *(t2.getField(tempOffset).str));
    attrPos++;
    tempOffset++;
  }
  //cout << "The tuple created is: " << endl;
  //tuple.printTuple();
  return tuple;
}

//void processNaturalJoins()

// return tuples that satify r.a = s.a, takes in realtion r, s and keys key1, key2 respectively
vector<Tuple> processNaturalJoinOnePass(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, 
                                        SchemaManager schema_manager, MainMemory &mem, string key1, string key2,
                                        bool writeToDB) {
  //cout << "Inside processNaturalJoinOnePass function" << endl;  
  // bring in MM the smaller realtion
  vector<Tuple> smallRelationTuples;
  vector<Tuple> bigRelationTuples;
  vector<Tuple> outputTuples;
  vector<Tuple>::iterator smallTupleIT;
  vector<Tuple>::iterator bigTupleIT;
  vector<Tuple>::iterator tupleIT;
  //int numBlocksInSmallRelation = 0;
 // int remainingBlocksInSmallRelation = smallRelation->getNumOfBlocks();
  //int lastBlockSmallRelation = 0;
  int i;
  int blocksInBigRelation = bigRelation->getNumOfBlocks();
  int blocksInSmallRelation = smallRelation->getNumOfBlocks();
  // Its a one pass Natural join, so the small realtion must fit in Main Memory?
  // Bring Small relation in MM-1, the size check do in the calee of processNaturalJoinOnePass
  smallRelation->getBlocks(0, 1, blocksInSmallRelation);
  smallRelationTuples=mem.getTuples(1, blocksInSmallRelation);
  //Bigger block, bring in MM block 0 location
  for(i = 0; i < blocksInBigRelation; i++) {
      //get one block for big relation and bring in MM
      bigRelation->getBlock(i,0);
      bigRelationTuples = mem.getTuples(0,1);
      // join tuples in this vector to all the tuples in the small relation
      for(bigTupleIT = bigRelationTuples.begin(); bigTupleIT != bigRelationTuples.end(); ++bigTupleIT) {
        for(smallTupleIT = smallRelationTuples.begin(); smallTupleIT != smallRelationTuples.end(); ++smallTupleIT) {
          // compare the two tuples if the value of keys are same
          // TODO check is key logic is correct for joining, i.e. mapping of key -> relation( 1, 2 vs small, big)
          // at this point check the tuples *bigTupleIT and *smallTupleIT if they satisfy any condition put in the condVec
          // TODO difficult logic, need to think
          if(canBeJoined(*bigTupleIT, *smallTupleIT, key1, key2)) {
            //cout << "Joining tuple" << endl;
            Tuple tempTuple = getJoinedTuple(*bigTupleIT, *smallTupleIT, tempRelation_ptr); // decide the params
            outputTuples.push_back(tempTuple);
          }
          // join does not satisfy, move over to next tuple in small realtion
          else
            continue;
        }
      }
  }
  // outputTuples is a vector of the tuples satisfying the natural join
  return outputTuples;
}

// Two pass algorithm for Natural Join -> B(R) + B(S) <= M * M 
vector<Tuple> processNaturalJoinTwoPass(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, 
                                        SchemaManager schema_manager, MainMemory &mem, string key1, string key2,
                                        bool writeToDB) {
  //cout << "Inside processNaturalJoinTwoPass function" << endl;
  vector<Tuple> smallRelationTuples;
  vector<Tuple> bigRelationTuples;
  vector<Tuple> outputTuples;
  vector<Tuple>::iterator smallTupleIT;
  vector<Tuple>::iterator bigTupleIT;
  vector<Tuple>::iterator tupleIT;
  // remember to delete these tables after resetting all the blocks
  string tableNameSmall = "smallTableSorted";
  string tableNameBig = "bigTableSorted";
  vector<Tuple> tempTuples;
  tempTuples.clear();
  Block* block_ptr;
  Block* block_ptr_Small;
  Block* block_ptr_Big;
  // hashmap for block in Sublist and block number to read
  map<int, int> sublistSmallBlockNumHash;
  map<int, bool> sublistSmallIsEmptyHash;
  map<int, int> sublistBigBlockNumHash;
  map<int, bool> sublistBigIsEmptyHash;
  int realBlockIndex, i, j, k;
  vector<string> key1Vec;
  vector<string> key2Vec;
  key1Vec.push_back(key1);
  key2Vec.push_back(key2);
  
  int blocksInBigRelation = bigRelation->getNumOfBlocks();
  int blocksInSmallRelation = smallRelation->getNumOfBlocks();
  Relation* smallTempRelation_ptr = schema_manager.createRelation(tableNameSmall, smallRelation->getSchema());
  Relation* bigTempRelation_ptr = schema_manager.createRelation(tableNameBig, bigRelation->getSchema());
  //cout << "calling createSortedSublistRelation for relation with key: " ;
  //cout << smallRelation->getRelationName() << " and " << key2 << endl;
  //cout << "calling createSortedSublistRelation for relation with key: " ;
  //cout << bigRelation->getRelationName() << " and " << key1 << endl;
  createSortedSublistRelation(smallRelation, smallTempRelation_ptr, key2Vec, mem, NUM_OF_BLOCKS_IN_MEMORY);
  createSortedSublistRelation(bigRelation, bigTempRelation_ptr, key1Vec , mem, NUM_OF_BLOCKS_IN_MEMORY);
  int numBlocksInSmallRelation = smallTempRelation_ptr->getNumOfBlocks();  
  int numBlocksInBigRelation = bigTempRelation_ptr->getNumOfBlocks();
  //cout << "Name of small and big relation are " <<  smallTempRelation_ptr->getRelationName() << " and " << bigTempRelation_ptr->getRelationName() << endl;
  //cout << "Num blocks in numBlocksInSmallRelation & numBlocksInBigRelation are " << numBlocksInSmallRelation << " and " << numBlocksInBigRelation << endl;
  // Now do the second pass on the tempRelation_ptr relation
  //cout << "Original contents are " << endl;
  //cout << "Small" << endl; smallRelation->printRelation(); cout << endl; cout << endl;
  //cout << "Big" << endl; bigRelation->printRelation(); cout << endl; cout << endl;
  //cout << "Printing the rlation Big" << endl;
  //bigTempRelation_ptr->printRelation(); cout << endl; cout << endl;
  //cout << "Printing the small realtion " << endl;
  //smallTempRelation_ptr->printRelation(); cout << endl; cout << endl;
  
  
  int numSmallSortedSublist, numBigSortedSublist;
  int numSmallBlocksLastSublist, numBigBlocksLastSublist;
  if(numBlocksInSmallRelation % NUM_OF_BLOCKS_IN_MEMORY == 0) {
    numSmallSortedSublist = numBlocksInSmallRelation / NUM_OF_BLOCKS_IN_MEMORY;
    numSmallBlocksLastSublist = NUM_OF_BLOCKS_IN_MEMORY;
  }
  else {
    numSmallSortedSublist = (numBlocksInSmallRelation / NUM_OF_BLOCKS_IN_MEMORY) + 1;
    numSmallBlocksLastSublist = numBlocksInSmallRelation % NUM_OF_BLOCKS_IN_MEMORY;
  }
  if(numBlocksInBigRelation % NUM_OF_BLOCKS_IN_MEMORY == 0) {
    numBigSortedSublist = numBlocksInBigRelation / NUM_OF_BLOCKS_IN_MEMORY;
    numBigBlocksLastSublist = NUM_OF_BLOCKS_IN_MEMORY;
  }
  else {
    numBigSortedSublist = (numBlocksInBigRelation / NUM_OF_BLOCKS_IN_MEMORY) + 1;
    numBigBlocksLastSublist = numBlocksInBigRelation % NUM_OF_BLOCKS_IN_MEMORY;
  }
  int remainingSmallSublist = numSmallSortedSublist;
  int remainingBigSublist = numBigSortedSublist;
  //cout << "remainingBigSublist and remainingSmallSublist are " << remainingBigSublist << " And " << remainingSmallSublist << endl;
  bool deleteBig = false;
  // bring one block from big and small
  int memBlockIndex = 0;
  for(i = 0; i < numBigSortedSublist; i++) {
    sublistBigBlockNumHash[i] = 0;
    sublistBigIsEmptyHash[i] = false;
    //bigTempRelation_ptr->getBlock(sublistBigBlockNumHash[i], memBlockIndex);
    bigTempRelation_ptr->getBlock(sublistBigBlockNumHash[i] + (i * NUM_OF_BLOCKS_IN_MEMORY), memBlockIndex);
    memBlockIndex++;
  }
  for(i = 0; i < numSmallSortedSublist; i++) {
    sublistSmallBlockNumHash[i] = 0;
    sublistSmallIsEmptyHash[i] = false;
    //smallTempRelation_ptr->getBlock(sublistSmallBlockNumHash[i], memBlockIndex);
    smallTempRelation_ptr->getBlock(sublistSmallBlockNumHash[i] + (i * NUM_OF_BLOCKS_IN_MEMORY), memBlockIndex);
    memBlockIndex++;
  }
  
  //cout << " memBlockIndex is " << memBlockIndex << endl;
  memBlockIndex = 0; // TODO revisit this...
  // I am bringing big block sublist and then small block sublist
  // so big block sublist ranges from 0 - B(big)/M = numBigSortedSublist-1
  // small block sublits starts from numBigBlocksLastSublist - numSmallBlocksLastSublist -1
  Tuple minTupleBig = smallTempRelation_ptr->createTuple();
  Tuple minTupleSmall = bigTempRelation_ptr->createTuple();
  minTupleBig.null(); minTupleSmall.null();
  int maxTuplesBigRel = bigTempRelation_ptr->getSchema().getTuplesPerBlock();
  int maxTuplesSmallRel = smallTempRelation_ptr->getSchema().getTuplesPerBlock();
  
  // New Data structures
  bool deleteBigTuple = false;
  bool deleteSmallTuple = false;
  bool doJoin = false;
  bool breakLoop = false;
  bool foundBigMin = false;
  bool foundSmallMin = false;
  vector<Tuple> tempTupleVecBig;
  vector<Tuple> tempTupleVecSmall;
  // get all the tuples in Big in vector and find min, do same for small
  do {
    for(i = 0; i < numBigSortedSublist; i++) { // go through each block in MM
      if(!sublistBigIsEmptyHash[i]) { // process block only if that particular corresponding sublist has something
        block_ptr = mem.getBlock(i); // get the block_ptr to the memory block
        vector<Tuple> blockTupVec = block_ptr->getTuples(); // get vector of tuples
        for(tupleIT = blockTupVec.begin(); tupleIT != blockTupVec.end(); ++tupleIT) {
          if(!tupleIT->isNull()) 
            tempTuples.push_back(*tupleIT);
        }
      }
    }
    //cout << "tempTuples is " << endl;
    //printTupleVector(tempTuples);
    //cout << endl;
    minTupleBig = (*(min_element(tempTuples.begin(), tempTuples.end(), TupleComparator(key1Vec))));
    //cout << "The minTupleBig is " << endl;
    //printTuple(minTupleBig);
    //cout << endl;
    tempTuples.clear();
    for(i = 0, j = numBigSortedSublist; j < (numBigSortedSublist + numSmallSortedSublist); i++, j++) {
      if(!sublistSmallIsEmptyHash[i]) { // Dicy Step
        block_ptr = mem.getBlock(j); // get the block_ptr
        vector<Tuple> blockTupVec = block_ptr->getTuples(); // get vector of tuples
        for(tupleIT = blockTupVec.begin(); tupleIT != blockTupVec.end(); ++tupleIT) {
          if(!tupleIT->isNull()) 
            tempTuples.push_back(*tupleIT);
        }
      }
    }
    //cout << "for Small tempTuples is " << endl;
    //printTupleVector(tempTuples);
    //cout << endl;
    minTupleSmall = (*(min_element(tempTuples.begin(), tempTuples.end(), TupleComparator(key2Vec))));
    //cout << "The minTupleSmall is " << endl;
    //printTuple(minTupleSmall);
    //cout << endl;
    tempTuples.clear();
    //cout << "Will be interesting " << endl;
    //printTupleVector(tempTuples);
    //cout << endl;
    if(canBeJoined(minTupleBig, minTupleSmall, key1, key2)) {
      deleteBigTuple = true; // delete all tuples that equal to minTupleBig
      deleteSmallTuple = true; // delete all tuples that equal to minTupleSmall
      doJoin = true;
      //cout << "The two mintupes can be joined" << endl;
       // collect all r min and s min tuples and send to outputTuples
      // big all tuples equal to minBig from the MM sublist of big relation
      for(i = 0; i < numBigSortedSublist; i++) {
        if(sublistBigIsEmptyHash[i]) continue; // if that particulat sublist is exhausted skip that MM block
        // we ensure that at least there is a single tuple in the MM
        block_ptr_Big = mem.getBlock(i); // block ptr for mem -> Big rel
       // The following condition should not be needed
       // if(block_ptr_Big->getTuples().size() == 0) continue; // mem block has no tuples
        for(j = 0; j < maxTuplesBigRel; j++) { // go through the tuples in that block -> BIG rel
          //if(block_ptr_Big->getNumTuples() == 0) continue;
          //cout << "691" << endl;
          Tuple tuple = block_ptr_Big->getTuple(j); // get tuple at offset j
          //cout << "694" << endl;
          if(tuple.isNull()) continue; // this is a hole in MM block, check next tuple
          if(compareTuplesAllAttr(minTupleBig, tuple)) {
            //tempTupleVecBig.push_back(tuple);
          }
        }
      }

      // Now for the smaller block in MM get the memory blocks
      for(i = numBigSortedSublist, j=0; i < (numBigSortedSublist + numSmallSortedSublist); i++, j++) {
        //cout << "Value of i, numBigSortedSublist, numSmallSortedSublist, j is " << i << " " << numBigSortedSublist <<  " " << numSmallSortedSublist << " " << j << endl;
        if(sublistSmallIsEmptyHash[j]) continue; // corresponding sublist empty
        // ensure that at least there is a single tuple in the MM
        block_ptr_Small = mem.getBlock(i); // block ptr for mem -> Small rel
        for(k = 0; k < maxTuplesSmallRel; k++) {
          Tuple tuple = block_ptr_Small->getTuple(k);
          if(tuple.isNull()) continue; // this is a hole in MM block
          if(compareTuplesAllAttr(minTupleSmall, tuple)) {
            //tempTupleVecSmall.push_back(tuple);
          }
        }
      }

      // join the two vector tuples tempTupleVecBig and tempTupleVecSmall
      //cout << "Joining tuples" << endl;
      // DO the join here

      //cout << endl << endl << endl;
      //tempTupleVecBig.clear();
      //tempTupleVecSmall.clear();
    }
    // the min are not same, delete the smaller tuple
    else {
      if(compareDiffSchTuples(minTupleBig, minTupleSmall, key1)) {
        //Tuple minTuple = minTupleBig;
        deleteBigTuple = true;
        deleteSmallTuple = false;
      }
      else {
        //Tuple minTuple = minTupleSmall;
        //cout << endl;        
        deleteBigTuple = false;
        deleteSmallTuple = true;
      }
    }
    do {
      foundBigMin = false;
      foundSmallMin = false;
    // Now the delete operations begin
    if(deleteBigTuple) {
      //cout << "Inside deleteBigTuple" << endl;
      for(i = 0; i < numBigSortedSublist; i++) {
        if(sublistBigIsEmptyHash[i]) continue; // no need to operate in this MM block
        block_ptr_Big = mem.getBlock(i); // at least one tuple is in i MM block
        //if(block_ptr_Big->getNumTuples() == 0) continue;
        for(j = 0; j < maxTuplesBigRel; j++) { // loop through all the tuples
          Tuple tuple = block_ptr_Big->getTuple(j);
          if(tuple.isNull()) continue; // this is a hole in MM block
          if(compareTuplesAllAttr(minTupleBig, tuple)) { // need to delete the tuple
            foundBigMin = true;
            if(doJoin) tempTupleVecBig.push_back(tuple);
            //cout << "Need to delete " << endl; printTuple(tuple); cout << endl;
            block_ptr_Big->nullTuple(j); // make the tuple invalid
            if(block_ptr_Big->getNumTuples() == 0) { // if this was the last tuple in block, delete and break this loop
              sublistBigBlockNumHash[i]++; // count to track if it is > MM
              int compare;
              // special case for last sublist
              if(i == numBigSortedSublist - 1) compare = numBigBlocksLastSublist;
              else compare = NUM_OF_BLOCKS_IN_MEMORY;
              if(sublistBigBlockNumHash[i] < compare) { // still blocks in sublist, bring to MM i
                realBlockIndex = sublistBigBlockNumHash[i] + (i * NUM_OF_BLOCKS_IN_MEMORY);
                //cout << "Block " << i << " empty " << endl;
                //cout << "Bringing new sublist to MM from index " << realBlockIndex << endl;
                bigTempRelation_ptr->getBlock(realBlockIndex, i);
              }
              else { // no more blocks in sublist, invalidate this sublist
                if(!sublistBigIsEmptyHash[i]) {// first time its empty
                  remainingBigSublist--;
                  if(remainingBigSublist == 0) breakLoop = true;
                }
                //cout << "This should not be printed " << endl;  
                sublistBigIsEmptyHash[i] = true;
              }
              break;
            }
          }
        }
      }
    }
    
    //if(breakLoop) break;
    
    if(deleteSmallTuple) {
      //cout << "Inside deleteSmallTuple" << endl;
      for(i = numBigSortedSublist, j=0; i < (numBigSortedSublist + numSmallSortedSublist); i++, j++) {
        if(sublistSmallIsEmptyHash[j]) continue; // no need to operate in this MM block
        block_ptr_Small = mem.getBlock(i); // at least one tuple is in i MM block
        //if(block_ptr_Small->getNumTuples() == 0) continue;
        for(k = 0; k < maxTuplesSmallRel; k++) { // loop through all the tuples
          //cout << "Inside tricky" << endl;
          Tuple tuple = block_ptr_Small->getTuple(k);
          if(tuple.isNull()) continue; // this is a hole in MM block
          if(compareTuplesAllAttr(minTupleSmall, tuple)) { // need to delete the tuple
            foundSmallMin = true;
            if(doJoin) tempTupleVecSmall.push_back(tuple);
            //cout << "found need to delete "; printTuple(tuple); cout << endl;
            block_ptr_Small->nullTuple(k); // make the tuple invalid
            if(block_ptr_Small->getNumTuples() == 0) { // if this was the last tuple in block, delete and break this loop
              sublistSmallBlockNumHash[j]++; // count to track if it is > MM
              int compare;
              // special case for last sublist
              if(i == (numBigSortedSublist + numSmallSortedSublist) - 1) compare = numSmallBlocksLastSublist;
              else compare = NUM_OF_BLOCKS_IN_MEMORY;
              
              if(sublistSmallBlockNumHash[j] < compare) { // still blocks in sublist, bring to MM i
                realBlockIndex = sublistSmallBlockNumHash[j] + (j * NUM_OF_BLOCKS_IN_MEMORY);
                smallTempRelation_ptr->getBlock(realBlockIndex, i); // Bring to MM block index i
              }
              else { // no more blocks in sublist, invalidate this sublist
                if(!sublistSmallIsEmptyHash[j]) // first time its empty
                  remainingSmallSublist--;
                sublistSmallIsEmptyHash[j] = true;
              }
              break;
            }
          }
        }
      }      
    }
    }while(foundBigMin || foundSmallMin);
    
      // cout << "The value of tempTupleVecBig is " << endl;
      //printTupleVector(tempTupleVecBig);
      //cout << endl;
      //cout << "The value of tempTupleVecSmall is " << endl;
      //printTupleVector(tempTupleVecSmall);
      //cout << endl;
      
      
    if(doJoin) {
      for(bigTupleIT = tempTupleVecBig.begin(); bigTupleIT != tempTupleVecBig.end(); ++bigTupleIT) {
        for(smallTupleIT = tempTupleVecSmall.begin(); smallTupleIT != tempTupleVecSmall.end(); ++smallTupleIT) {
          Tuple tempTuple = getJoinedTuple(*bigTupleIT, *smallTupleIT, tempRelation_ptr);
          //cout << "The joined tuple is: " << tempTuple << endl;
          outputTuples.push_back(tempTuple);
          //cout << " Outtuples is " << endl; printTupleVector(outputTuples); cout << endl;
        }
      }
    }
    deleteBigTuple = false; // delete all tuples that equal to minTupleBig
    deleteSmallTuple = false;
    doJoin = false;
    tempTupleVecBig.clear();
    tempTupleVecSmall.clear();
    //cout << "remainingBigSublist and remainingSmallSublist is " << remainingBigSublist << " " << remainingSmallSublist << endl;
  } while(remainingSmallSublist > 0 && remainingBigSublist > 0);
  
  // Do the clean up before returning, any temprary schema needs to be deleted here
  schema_manager.getRelation(tableNameSmall)->deleteBlocks(0);
  schema_manager.deleteRelation(tableNameSmall);
  schema_manager.getRelation(tableNameBig)->deleteBlocks(0);
  schema_manager.deleteRelation(tableNameBig);
  
  // Its a one pass Natural join, so the small realtion must fit in Main Memory?
  // Bring Small relation in MM-1, the size check do in the calee of processNaturalJoinOnePass
  //smallRelation->getBlocks(0, 1, blocksInSmallRelation);
  //smallRelationTuples=mem.getTuples(1, blocksInSmallRelation); 
  //Tuple getNatualJoinedTuple()
  return outputTuples;
}


// generic natural join - when both the relation are very big
// Bring MM - 1 block for the small relation
// and one block of large relation and then do the join
// return tuples that satify r.a = s.a, takes in realtion r, s and keys key1, key2 respectively
vector<Tuple> processNaturalJoinGeneric(Relation* bigRelation, Relation *smallRelation, Relation* &tempRelation_ptr, 
                                        SchemaManager schema_manager, MainMemory &mem, string key1, string key2,
                                        bool writeToDB) {
  //cout << "Inside processNaturalJoinGeneric function" << endl;  
  // bring in MM the smaller realtion
  vector<Tuple> smallRelationTuples;
  vector<Tuple> bigRelationTuples;
  vector<Tuple> outputTuples;
  vector<Tuple>::iterator smallTupleIT;
  vector<Tuple>::iterator bigTupleIT;
  vector<Tuple>::iterator tupleIT;
  int i;
  int blocksInBigRelation = bigRelation->getNumOfBlocks();
  int count = 0;
  //smallRelation->printRelation(); cout << endl;
  //bigRelation->printRelation(); cout << endl;
  for(i = 0; i < blocksInBigRelation; i++) {
    //cout << " i is " << i << endl;
    //get one block for big relation and bring in MM
    bigRelation->getBlock(i,0);
    bigRelationTuples = mem.getTuples(0,1);
    int remainingBlocksInSmallRelation = smallRelation->getNumOfBlocks();
    int lastBlockSmallRelation = 0;
    int numBlocksInSmallRelation;
    // get the smaller relation blocks in MM - 1
    while(remainingBlocksInSmallRelation > 0) {
      if(remainingBlocksInSmallRelation > NUM_OF_BLOCKS_IN_MEMORY - 1) {
        numBlocksInSmallRelation = NUM_OF_BLOCKS_IN_MEMORY - 1;
      }
      else {
        numBlocksInSmallRelation = remainingBlocksInSmallRelation;
      }
      smallRelation->getBlocks(lastBlockSmallRelation, 1, numBlocksInSmallRelation);
      smallRelationTuples=mem.getTuples(1, numBlocksInSmallRelation);
      for(bigTupleIT = bigRelationTuples.begin(); bigTupleIT != bigRelationTuples.end(); ++bigTupleIT) {
        for(smallTupleIT = smallRelationTuples.begin(); smallTupleIT != smallRelationTuples.end(); ++smallTupleIT) {
          //cout << "Checking "; printTuple(*bigTupleIT); cout << " and "; printTuple(*smallTupleIT); cout << endl;
          if(canBeJoined(*bigTupleIT, *smallTupleIT, key1, key2)) {
            Tuple tempTuple = getJoinedTuple(*bigTupleIT, *smallTupleIT, tempRelation_ptr); // decide the params
            outputTuples.push_back(tempTuple);
          }
        }
      }
      remainingBlocksInSmallRelation -= (NUM_OF_BLOCKS_IN_MEMORY - 1);
      lastBlockSmallRelation += (NUM_OF_BLOCKS_IN_MEMORY - 1); 
    }
    count++;
  }
  return outputTuples;
}
  
