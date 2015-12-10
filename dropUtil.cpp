#include<iostream>
#include<iterator>
#include<cstdlib>
#include<ctime>
#include<string>
#include<list>
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
#include "dropUtil.h"

// process statements realted to DROP
void processDrop(string line, string table, SchemaManager schema_manager, MainMemory &mem) {
  cout << "Inside processDrop function" << endl;
  //TO_DO - reset memory blocks of the table in question
  if(schema_manager.getRelation(table)->deleteBlocks(0) && schema_manager.deleteRelation(table))
    cout << "Table " << table << " successfully deleted/dropped" << endl;
  else
    cout << "Table " << table << " not found" << endl;
  return;
}