// Helper file for select join

extern list<int> free_blocks;
extern map<string, list<int> > relation_hash;
//perform and print results of a join without where conditions (cross join)
void printSelectJoinNoWhere(vector<string>, vector<string>, SchemaManager, MainMemory&);
//perform and print results of a join on all attributes with a where clause
void printSelectJoinStar(vector<string>, vector<string>, SchemaManager, MainMemory&);
//perform and print results of a join on all attributes without a where clause
void printSelectJoinStarNoWhere(vector<string>, vector<string>, SchemaManager, MainMemory&);

// New functions
//perform and print results of a one pass join
void printSelectJoinOnePass(int, vector<string>, vector<string>,vector<string>, SchemaManager, MainMemory &);