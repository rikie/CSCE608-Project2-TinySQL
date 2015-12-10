extern list<int> free_blocks;
extern map<string, list<int> > relation_hash;

// Process the SELECT Statements
vector<Tuple> processSelect(string, vector<string>, SchemaManager, MainMemory &, bool);
     
// TODO: Unused: To print schema heading for select statement like select sid, grade from table......
void printSpecificSchema(vector<string>attrVec, Tuple tuple);

// Process a tuple if the relation_ptr matches with the one from the select statement
void printSpecificTupleAttr(vector<string>attrVec, Tuple tuple);

// To print tuple which satisfies select * from a where c > 10 
void printAllTupleAttr(Tuple tuple);

// for statement like select a,b from table - no where condition
void printSelectNoWhere(vector<string> attrVec, vector<string> tableVec, SchemaManager schema_manager, MainMemory &mem);

// Handle Select statements with where clause but no join involved, single table after from keyword.
void printComplexSelect(vector<string> attrVec, vector<string> tableVec, vector<string> condVec, SchemaManager schema_manager, MainMemory &mem);

// handle select * from table
void printSelectStar(vector<string> attrVec, vector<string> tableVec, SchemaManager schema_manager);

// print select * using one pass
void printSelectStarOnePass(vector<string>, vector<string>, SchemaManager, MainMemory &);

// print select statement with attr list as * - all attributes
void printTupleVecAllAttr(vector<Tuple>);

// print select statements with specific attributes
void printTupleVecSpecificAttr(vector<Tuple>, vector<string>);

vector<Tuple> printSelectNoJoinOnePass(int, vector<string>, vector<string>, vector<string>,
                                       vector<string>, SchemaManager, MainMemory &, bool, 
                                       bool, bool);

// Not used
/**
void printTupleVecWhereCondSpAttr(vector<Tuple>, vector<string>, vector<string>, bool &);

void printTupleVecWhereCondAllAttr(vector<Tuple>, vector<string>, bool &);
*/