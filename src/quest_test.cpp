#include "simulate.h"
#include "parser.h"
#include <sstream>
#include <iostream>
#include <fstream>

#define tassert(cond,name) {if(!(cond)){std::cout << "TEST FAILED: " name ": " #cond "\n";}}

void test_parsing(){
    std::ifstream file("Samples/4regRand30Node1-p1.qasm");
    std::string str((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
     std::stringstream reader(str);
     std::stringstream writer;
    Circuit c = parseGates(reader);
    printGates(c,writer);
    std::string out_str = writer.str();
    //std::cout << str << "\n\n\n\n\n";
    //std::cout << out_str << "\n\n\n\n\n";
    tassert(str == out_str,"parser_test");
}
int main (int narg, char** varg) {
    test_parsing();
    simulate("Samples/4regRand20Node5-p1.qasm");
    simulate("Samples/rand-nq6-cn2-d10_rxz.qasm");
}
