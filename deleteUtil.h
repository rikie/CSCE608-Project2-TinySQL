extern list<int> free_blocks;

// process statements realted to Delete
void processDelete(string, vector<string>, SchemaManager, MainMemory &);

// delete tuples for statements having where clause
// delete from table where <condition>
void deleteTupleWhereClause(string, vector<string>, SchemaManager, MainMemory &);

// verify where a tuple satisfies a WHERE condition
bool checkTupleStatisfiesWhere(Tuple, vector<string>, Schema);

bool checkTupleStatisfiesWhereJoin(Tuple, vector<string>, Schema);