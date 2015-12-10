// Global List to keep track of free blocks
list<int> free_blocks;
map<string, list<int> > relation_hash;

Schema* processCreate(vector<string>);
// Helper functions
//clear list of free blocks
void resetFreeBlocks(list<int>&);
//add tuple to a relation
void appendTupleToRelation(Relation*, MainMemory&, int, Tuple&);
//split a string
vector<string> splitString(string, string);
//print functions
void printSchema(Schema);
void printRelation(Relation*);
void printTuple(Tuple);
