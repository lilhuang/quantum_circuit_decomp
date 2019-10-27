
all: Quest_/libQuEST.a metis-5.1.0/build/Linux-x86_64/libmetis/libmetis.a
	g++ -Og -g  -std=c++17 -o bin/quest_test -I QuEST_/include/ -I metis-5.1.0/include src/quest_test.cpp src/simulate.cpp src/parser.cpp src/graph_partition.cpp src/tensor_network.cpp -L QuEST_ -L metis-5.1.0/build/Linux-x86_64/libmetis/ -l QuEST -l metis

Quest_/libQuEST.a:
	cmake QuEST_
	$(MAKE) -C QuEST_


metis-5.1.0/build/Linux-x86_64/libmetis/libmetis.a:
	#cmake metis-5.1.0/
	-$(MAKE) config -C metis-5.1.0
	-$(MAKE) -j4 -C metis-5.1.0
