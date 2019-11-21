#pragma once
#include <vector>
#include "circuit.h"
#include <unordered_set>
#include "tensor_network.h"
using namespace std;


struct Node{
	Node* input[2];
	Node* output[2];
	int level;
	uint64_t bit1, bit2;
	int cluster;

	// refers to the gate index in original circuit
	uint gateIndex;
	Node();
} ;

// initial implementation groups nodes, instead of bits.
struct Cluster{
	unordered_set<int> bits;
	vector<Node*> nodes;
	int id;
	// endLevel is not included
	u_int32_t startLevel, endLevel;
	Cluster(){endLevel = -1;}
	} ;

struct NodeTable{
	vector<vector<Node*>> level;

	// first and last gates that are entered/exited by qubits stored in  this structure 
	vector<Node*> inputGates, outputGates;
	vector<Cluster*> getClusteredNetwork(u_int32_t availableQubits);
	MultiGraphNetwork getGreedyClusteredNetworkPartition(u_int32_t availableQubits, const Circuit & origCircuit);
	vector<size_t> getGreedyClusteredNetworkVector(u_int32_t availableQubits, const Circuit & origCircuit);
	u_int32_t numQubits;
	NodeTable(int size);
} ;


NodeTable circuitToNodeTable(const Circuit & c);
Cluster* getBestCandidate(int availableQubits, Node* n, vector<Cluster*> bitownership);
void addNodeToCluster(Node* n, Cluster* c, vector<Cluster*> & bitowners);
double getAssociativityToChild(Node* p, Node* c);