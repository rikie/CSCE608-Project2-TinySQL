all: evalWhereConds.o parserLib.o helper.o helpSelect.o helpInsert.o selectJoinUtil.o deleteUtil.o dropUtil.o StorageManager.o project2

project2: StorageManager.o 
	g++ -o project2 StorageManager.o helper.o helpInsert.o helpSelect.o selectJoinUtil.o deleteUtil.o dropUtil.o evalWhereConds.o parserLib.o project2.cpp

StorageManager.o: Block.h Disk.h Field.h MainMemory.h Relation.h Schema.h SchemaManager.h Tuple.h Config.h
	g++ -c StorageManager.cpp

evalWhereConds.o: evalWhereConds.h
	g++ -c evalWhereConds.cpp

parserLib.o:
	g++ -c parserLib.cpp

helper.o: helper.h
	g++ -c helper.cpp

helpInsert.o: helpInsert.h
	g++ -c helpInsert.cpp

helpSelect.o: helpSelect.h
	g++ -c helpSelect.cpp

selectJoinUtil.o: selectJoinUtil.h
	g++ -c selectJoinUtil.cpp

deleteUtil.o: deleteUtil.h
	g++ -c deleteUtil.cpp

dropUtil.o: dropUtil.h
	g++ -c dropUtil.cpp

clean:
	rm *.o project2
