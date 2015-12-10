//overall function that handles joins
void processSelectJoin(int, vector<string>, vector<string>,vector<string>, vector<string>, 
                       SchemaManager, MainMemory &, bool, bool, bool);
                       
//vector<Tuple> processCrossJoinOnePass(string, string, SchemaManager, MainMemory &, string, bool, Relation*&);

//handle one pass cross joins, return vector of joined tuples
vector<Tuple> processCrossJoinOnePass(Relation*, Relation *, Relation* &, SchemaManager,
                                      MainMemory &, bool );

//join two tuples and return the joined result
Tuple getJoinedTuple(Tuple, Tuple, Relation*&);

//create a schema from all attributes of two other relation schemas
Schema createSchemaAllAttrJoinTables(string, string, SchemaManager);

//process one pass cross join on specific attributes
vector<Tuple> processCrossJoinOnePassSpAttr(string, string, vector<string>, 
                                            SchemaManager, MainMemory &, string, bool);
//create a schema on specific attributes
Schema createSchemaSpAttrJoinTables(string, string, vector<string>, SchemaManager);

//get tuples joined on specific attributes
Tuple getJoinedTupleSpAttr(Tuple, Tuple, Relation *, Relation*, Relation* &, vector<string>);

//one pass natural join
vector<Tuple> processNaturalJoinOnePass(Relation*, Relation *, Relation* &, SchemaManager , 
                                        MainMemory &, string , string , bool );
//two-pass sort based natural join                                      
vector<Tuple> processNaturalJoinTwoPass(Relation*, Relation *, Relation* &, SchemaManager , 
                                        MainMemory &, string , string , bool );
//nested loop natural join                                        
vector<Tuple> processNaturalJoinGeneric(Relation*, Relation *, Relation* &, SchemaManager , 
                                        MainMemory &, string , string , bool );