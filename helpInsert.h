// An example procedure of appending a tuple to the end of a relation
// using memory block "memory_block_index" as output buffer
extern list<int> free_blocks;
extern map<string, list<int> > relation_hash;
void processInsert(string, vector<string>, SchemaManager, MainMemory &);
void appendTupleToRelation(Relation* relation_ptr, MainMemory& mem, int memory_block_index, Tuple& tuple);
