#include "clustering.h"
#include <vector>
#include <unordered_map>
#include <list>
#include "circuit.h"
#include <algorithm>
#include <iostream>
#include "tensor_network.h"
#include "gates.h"
using namespace std;

NodeTable circuitToNodeTable(const Circuit & c){
	NodeTable nt(c.num_qubits);
	for(int i = 0; i < c.gates.size(); i++){
		GateInfo gate = c.gates[i];
		// we only care about gates with 2 qubits
		if(gate.bit2 != NULL_BIT){
			Node * n = new Node();
			n->gateIndex = i;
			n->bit1 = gate.bit1;
			n->bit2 = gate.bit2;
			
			// adds itself as the output gate of the parent gates
			if(nt.outputGates[n->bit1] != nullptr){
				if(nt.outputGates[n->bit1]->bit1 == n->bit1)
					nt.outputGates[n->bit1]->output[0] = n;
				else
					nt.outputGates[n->bit1]->output[1] = n;
			}
			if(nt.outputGates[n->bit2] != nullptr){
				if(nt.outputGates[n->bit2]->bit1 == n->bit2)
					nt.outputGates[n->bit2]->output[0] = n;
				else
					nt.outputGates[n->bit2]->output[1] = n;
			}
			
			// add input gates to itself
			n->input[0] = nt.outputGates[gate.bit1];
			n->input[1] = nt.outputGates[gate.bit2];
			
			// if an input gate does not exist, register itself as the input
			if(n->input[0] == nullptr)
				nt.inputGates[n->bit1] = n;
			if(n->input[1] == nullptr)
				nt.inputGates[n->bit2] = n;
				
			// register itself as the output gate
			nt.outputGates[n->bit1] = n;
			nt.outputGates[n->bit2] = n;
			
			// get the maximum level from the previous gates, add 1 to find its level
			// initial level is 0
			n->level = 1+max(n->input[0] == nullptr ? -1 : n->input[0]->level, 
							n->input[1] == nullptr ? -1 : n->input[1]->level);
			
			// add itself to the level part in table
			if(nt.level.size() == n->level)
				nt.level.push_back(vector<Node*>());
			nt.level[n->level].push_back(n);
		}
	}
	return nt;
}

NodeTable::NodeTable(int size):
	inputGates(vector<Node*> (size, nullptr)), outputGates(vector<Node*> (size,nullptr)), numQubits(size)
{}

vector<Cluster*> NodeTable::getClusteredNetwork(u_int32_t availableQubits){
	
		vector<Cluster*> clusters;
		vector<Cluster*> bitowners(numQubits);

		// initialization step
		for (size_t i = 0; i < level[0].size(); i++)
		{
			Node* n = level[0][i];
			n->cluster = i;
			Cluster* c = new Cluster();
			c->startLevel = 0;
			c->id = i;
			addNodeToCluster(n, c, bitowners);
			clusters.push_back(c);
		}
		
		for (size_t i = 1; i < level.size(); i++)
		{
			vector<Cluster*> candidateClusters;
			for (size_t j = 0; j < clusters.size(); j++)
			{
				if (clusters[j]->endLevel==-1)
				{
					candidateClusters.push_back(clusters[j]);
				}
			}
			// check if a cluster received a qubit this turn
			vector<bool> bitmapForCandidates(candidateClusters.size(),false);
			
			
			for (size_t j = 0; j < level[i].size(); j++)
			{
				Node *n = level[i][j];
				Cluster* c = getBestCandidate(availableQubits, n, bitowners);
				if (c == nullptr)
				{
					c = new Cluster();
					c->startLevel = n->level;
					c->id = clusters.size();
					clusters.push_back(c);
				}
				addNodeToCluster(n, c, bitowners);
			}
			
		}
	

		return clusters;
	
	}
MultiGraphNetwork NodeTable::getGreedyClusteredNetworkPartition(u_int32_t availableQubits, const Circuit & origCircuit){
	TensorNetwork tn = from_circuit(origCircuit, vector<uint8_t>());
	return create_multi_graph_network(tn, getGreedyClusteredNetworkVector(availableQubits, origCircuit));
}

vector<size_t> NodeTable::getGreedyClusteredNetworkVector(u_int32_t availableQubits, const Circuit & origCircuit){
	
		

		// initial cluster count is equal to the number of gates
		// as gates are combined we get less but larger clusters
		uint numGates = 0;
		for (size_t i = 0; i < level.size(); i++)
		{
			numGates += level[i].size();
		}
		vector<vector<uint>> numberOfQubitsClusterLevel(numGates);
		for (size_t i = 0; i < numGates; i++)
		{
			numberOfQubitsClusterLevel[i] = vector<uint>(level.size()+1,0);
		}
		
		vector<vector<double>> associativityToChildren(2); 
		vector<vector<double>> constAssociativityToChildren(2); 
		for (size_t i = 0; i < 2; i++)
		{
			associativityToChildren[i] = vector<double>(numGates);
		}

		vector<Node*> nodes(numGates);
		// calculate initial assocativity scores with children
		// initialize number of qubits matrix, initialize cluster numbers
		uint gateNumber = 0;
		for (size_t i = 0; i < level.size(); i++)
		{
			for (size_t j = 0; j < level[i].size(); j++, gateNumber++)
			{
				Node * n = level[i][j];
				n->cluster = gateNumber;
				uint levelL = n->input[0] ? n->input[0]->level +1 : 0;
				uint levelR = n->input[1] ? n->input[1]->level +1 : 0;
				for (size_t k = min(levelL, levelR); k < max(levelL, levelR); k++)
				{
					numberOfQubitsClusterLevel[gateNumber][k] = 1;
				}

				for (size_t k = max(levelL, levelR); k <= n->level; k++)
				{
					numberOfQubitsClusterLevel[gateNumber][k] = 2;
				}

				if (n->output[0] == nullptr || n->output[1] == nullptr)
				{
					int qubitc = (n->output[0] == nullptr) + (n->output[1] == nullptr);
					for (size_t k = n->level+1; k <= level.size(); k++)
					{
						numberOfQubitsClusterLevel[gateNumber][k] = qubitc;
					}
				}

				nodes[gateNumber] = n;
				
				for (size_t k = 0; k < 2; k++)
				{
					associativityToChildren[k][gateNumber] = getAssociativityToChild(n, n->output[k]);
				}
			}
			
		}
		for (size_t k = 0; k < 2; k++)
		{
			constAssociativityToChildren[k] = associativityToChildren[k];
		}
		
		// do the iterations to form the cluster
		bool continueFlag = true;
		while (continueFlag)
		{
			uint maxLindex= max_element(associativityToChildren[0].begin(), associativityToChildren[0].end())- associativityToChildren[0].begin();
			uint maxRindex = max_element(associativityToChildren[1].begin(), associativityToChildren[1].end()) - associativityToChildren[1].begin();
			Node* c, *n;
			double value;
			bool chosenLeft;
			// merge maxLindex'th node with its left child
			if(associativityToChildren[0][maxLindex] > associativityToChildren[1][maxRindex]){
				value = associativityToChildren[0][maxLindex];
				n = nodes[maxLindex];
				c = n->output[0];
				chosenLeft = true;
			}
			else
			{
				value = associativityToChildren[1][maxRindex];
				n = nodes[maxRindex];
				c = n->output[1];
				chosenLeft = false;
			}
			
			// meaningful connections are exhausted
			if (value <= 0)
			{
				continueFlag = false;
			}
			else{
				continueFlag = true;
				bool mergableFlag = n->cluster != c->cluster;
				vector<uint> mergeSizecheck(level.size()+1);
				for (size_t i = 0; i < mergeSizecheck.size() && mergableFlag; i++)
				{
					uint t = numberOfQubitsClusterLevel[c->cluster][i] + numberOfQubitsClusterLevel[n->cluster][i];
					mergeSizecheck[i] = t;
					mergableFlag = t <= availableQubits;
				}
				
				// clustera ait butun elemanlar yeni nodea olan associativityleri kadar coefficient kazanir
				// yeni node da bunlarin toplami kadar kazanir. 0 lanmis coefficientler ayni kalir bunu simdilik gectim
				
				// merge the two clusters
				if(mergableFlag)
				{
					uint previousCluster = c->cluster;
					uint newCluster = n->cluster;
					for (size_t i = 0; i < numGates; i++)
					{
						if (nodes[i]->cluster == previousCluster)
							nodes[i]->cluster = newCluster;
					}
					numberOfQubitsClusterLevel[newCluster] = mergeSizecheck;
				}

				// do nothing if not merging clusters
				// by default always make the new association 0
				if(chosenLeft){
					associativityToChildren[0][maxLindex] = 0;
				}
				else
				{
					associativityToChildren[1][maxRindex] = 0;
				}
			}
		}
		
		TensorNetwork tn = from_circuit(origCircuit, vector<uint8_t>());
		vector<int> partitionIdxs(tn.size(),-1);
		unordered_map<uint,uint> clusterMapping;
		uint clusterCnt = 0;
		// this loop is badly written and not the most efficient
		for (size_t i = 0; i < level.size(); i++)
		{
			for (size_t j = 0; j < level[i].size(); j++)
			{
				Node *n = level[i][j];
				uint reducedClusterIndex = clusterMapping[n->cluster];
				
				// this cluster id is encountered for the first time
				if(clusterMapping.size()>clusterCnt){
					clusterMapping[n->cluster] = clusterCnt++;
					reducedClusterIndex = clusterMapping[n->cluster];
				}
				uint tensorGateIndex = n->gateIndex+numQubits;
				partitionIdxs[tensorGateIndex] = reducedClusterIndex;

				uint gateIndexIterator = tn.backward_edges[tensorGateIndex][0];
				// while the parent tensor is not another cnot/2qubit gate
				while (tn.tensors[gateIndexIterator].op.gate != GateTy::CNOT)
				{
					partitionIdxs[gateIndexIterator] = reducedClusterIndex;
					if(tn.backward_edges[gateIndexIterator].size() == 1)
						gateIndexIterator = tn.backward_edges[gateIndexIterator][0];
					else
						break;
					
				}
				gateIndexIterator = tn.backward_edges[tensorGateIndex][1];
				// while the parent tensor is not another cnot/2qubit gate
				while (tn.tensors[gateIndexIterator].op.gate != GateTy::CNOT)
				{
					partitionIdxs[gateIndexIterator] = reducedClusterIndex;
					if(tn.backward_edges[gateIndexIterator].size() == 1)
						gateIndexIterator = tn.backward_edges[gateIndexIterator][0];
					else
						break;
					
				}

				// if forward gates are not owned by anyone change their group
				gateIndexIterator = tn.forward_edges[tensorGateIndex][0];
				while (tn.tensors[gateIndexIterator].op.gate != GateTy::CNOT && partitionIdxs[gateIndexIterator] == -1)
				{
					partitionIdxs[gateIndexIterator] = reducedClusterIndex;
					if(tn.forward_edges[gateIndexIterator].size() == 1)
						gateIndexIterator = tn.forward_edges[gateIndexIterator][0];
					else
						break;
					
				}

				gateIndexIterator = tn.forward_edges[tensorGateIndex][1];
				while (tn.tensors[gateIndexIterator].op.gate != GateTy::CNOT && partitionIdxs[gateIndexIterator] == -1)
				{
					partitionIdxs[gateIndexIterator] = reducedClusterIndex;
					if(tn.forward_edges[gateIndexIterator].size() == 1)
						gateIndexIterator = tn.forward_edges[gateIndexIterator][0];
					else
						break;
					
				}
			}
		}
		vector<size_t> unsignedPartitionIdx(partitionIdxs.size());
		for (size_t i = 0; i < partitionIdxs.size(); i++)
		{
			assert(partitionIdxs[i]!=-1);
			unsignedPartitionIdx[i] = partitionIdxs[i];
		}
		
		return unsignedPartitionIdx;
	}

// traverse from child node, count the common qubits used with the parent node
// look at most 'arbitraryDepth' depth in the tree
double getAssociativityToChild(Node* p, Node* c){
	
	uint arbitraryDepth = 8;

	if(p == nullptr || c == nullptr)
		return 0;
	double score = 0;

	list<Node*> queue;
	queue.push_back(c); 
	uint curLevel = p->level;
	while(!queue.empty()){
		Node* cn = queue.front();
		queue.pop_front();
		if (cn == nullptr)
			continue;
		uint effect = arbitraryDepth - (cn->level - curLevel);
		if( effect >=0 ){
			queue.push_back(cn->output[0]);
			queue.push_back(cn->output[1]);

			uint commonQubitCnt = (cn->bit1 == p->bit1) + (cn->bit1 == p->bit2)
				 + (cn->bit2 == p->bit1) + (cn->bit2 == p->bit2);
			score += commonQubitCnt << effect;
		}
	}
	// maximum attainable score between two gates. for example, (2^9 - 1)*2 for 2 ^ 8
	return score / ((2 << (arbitraryDepth + 1)) - 2);
}


Cluster* getBestCandidate(int availableQubits, Node* n, vector<Cluster*> bitownership){

	unordered_map<Cluster*, uint64_t> candidates;
	list<Node*> queue;
	queue.push_back(n);
	int arbitraryDepth = 5; 
	int curLevel = n->level;
	while(!queue.empty()){
		Node* cn = queue.front();
		queue.pop_front();
		int effect = arbitraryDepth - (cn->level - curLevel);
		if( effect >=0 ){
			queue.push_back(cn->output[0]);
			queue.push_back(cn->output[1]);

			candidates[bitownership[cn->bit1]] += 1 << effect;
			candidates[bitownership[cn->bit2]] += 1 << effect;
		}
	}

	Cluster* bestCluster;
	uint64_t maxval = 0;
	for (auto iter = candidates.begin(); iter != candidates.end(); iter++)
	{
		if(iter->second > maxval && iter->first->bits.size() < availableQubits){
			maxval = iter->second;
			bestCluster = iter->first;
		}
	}
	return bestCluster;
}


Node::Node(){
		input[0] = input[1] = output[0] = output[1] = nullptr;
	}

void addNodeToCluster(Node* n, Cluster* c, vector<Cluster*> & bitowners){
	
			Cluster* preOwner1 = bitowners[n->bit1];
			Cluster* preOwner2 = bitowners[n->bit2];

			c->nodes.push_back(n);
			bitowners[n->bit1] = c;
			bitowners[n->bit2] = c;
			c->bits.insert(n->bit1);
			c->bits.insert(n->bit2);

	// if a cluster has no members left we should try to merge it to nearby clusters. might not be always possible.
			preOwner1->bits.erase(n->bit1);
			preOwner2->bits.erase(n->bit2);
}
