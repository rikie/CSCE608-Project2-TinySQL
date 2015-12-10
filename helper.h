extern list<int> free_blocks;
extern map<string, list<int> > relation_hash;

void resetFreeBlocks(list<int>&);
void appendTupleToRelation(Relation*, MainMemory&, int, Tuple&);
vector<string> splitString(string, string);
void printSchema(Schema);
void printRelation(Relation*);

void printTuple(Tuple);

void printVector(vector<string> vstr);

// print vector of tuples
void printTupleVector(vector<Tuple>);

void printSchema(Schema schema);
 
void printRelation(Relation* relation_ptr);

vector<string> splitString(string str, string delimiter);

vector<string> removeDot(vector<string> processVec);

int findBlockForTuple(MainMemory &mem);

void printTuple(Tuple tuple);

void printTupleVector(vector<Tuple>);

bool compareDiffSchTuples(Tuple, Tuple, string);

bool checkTwoFieldsTuple(Tuple, string, string);

/**
void printJoinedTupleAllAttr(vector<Tuple>, vector<string>, vector<string>);

void printJoinedTupleSpAttr(vector<Tuple>, vector<string>, vector<string>, Schema);
*/
// Uncomment later
                           
void printJoinedTupleAllAttr(vector<Tuple>, vector<string>, vector<string>, int);

void printJoinedTupleSpAttr(vector<Tuple>, vector<string>, vector<string>, Schema, 
                            vector<string>, int );

bool compareTuplesAllAttr(Tuple, Tuple);

bool compareTuplesSpAttr(Tuple, Tuple, vector<string>);

void checkDupTupleSpAttr(vector<Tuple> &tuples, Tuple tuple, vector<string>);

void checkDupTupleAllAttr(vector<Tuple> &tuples, Tuple tuple);

bool canBeJoined(Tuple, Tuple, string, string);

vector<Tuple> getDistinctTuples(Relation*, Relation*&, vector<string>, MainMemory &);

vector<Tuple> getSortedTuples(Relation*, Relation*&, vector<string>, MainMemory &);

void createSortedSublistRelation(Relation*, Relation* &, vector<string>, MainMemory &, int);
// Class to overload the comparision operator in sort function for sorting 
// the tuple vector
class TupleComparator {
  private:
    vector<string> mAttrVec;
  public:
    TupleComparator(vector<string> attrVec) { mAttrVec = attrVec; }
    bool operator() (Tuple t1, Tuple t2);
};