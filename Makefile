
all: Quest_/libQuEST.a
	g++ -std=c++17 -o bin/quest_test -I QuEST_/include/ src/quest_test.cpp src/simulate.cpp src/parser.cpp -L QuEST_ -l QuEST

Quest_/libQuEST.a:
	$(MAKE) -C QuEST_
