#include "structs.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream> // for stringstream
#include <cstdlib> // for rand() and srand()
#include <ctime>   // for time()
#include <algorithm>
#include <unordered_set>
#include <random>
#include <chrono>
using namespace std;



// Create a random number engine and seed it
std::random_device rd;
std::mt19937 gen(rd()); 
// Create a uniform distribution for floating-point numbers between 0 and 1 for randomizing the partition.
std::uniform_real_distribution<double> dis(0.0, 1.0);


//NOTE: When I use "", I am referring to the exact name of the variable I used in the code.


class Graph {
private:
	int numVertices;    //number of vertices
    int cutsize;        
    int pmax=0;         //maximum gain possible.
    int offset = 0; //an offset is required to store both +ve and -ve gains.
    vector<node> nodes;

    vector<node> partition1;
    vector<node> partition2;    
    circuit circuit_nets;   //each index of circuit_nets holds a net.
	Bucket gainBucket;      //the Bucket structure.    


public:
    int calc_Cutsize_and_gain() {
        /*we need to reset the gainBucket so that the previous gain values
        for nodes are removed.
        we use a temp Bucket to replace the current Bucket.
        THe outer loop iterates through every net.
        In each net, the first inner loop iterates through all the nodes
        that are connected to the source node.
		the second inner loop iterates through all the nodes that are 
        connected to the current source node and checks whether they are 
        in the same partition or not.
        */

        Bucket temp;
        temp.b.resize(2 * pmax + 1);    

		int cutsize = 0;
        for (auto& net : circuit_nets) {
            int out_edge = 0;
            int in_edge = 0;
            bool partition = net.source.partition_num;
            for (auto& x : net.connected) {
                if (partition != circuit_nets[x].source.partition_num) {
                    out_edge++;
                }
                else {
                    in_edge++;
                }
            }
            for (auto& x : net.tv) {
                if (partition != circuit_nets[x.value].source.partition_num) {
                    cutsize++;
                }
            }
            cout << "node:" << net.source.value<<" gain:"<< out_edge - in_edge << " locked: " << net.source.locked <<" partition:" << net.source.partition_num << endl;
	//You can read the next instruction as: 
    // Calculate the gain, add an offset to it for the index of the Bucket,
	// take that index of the bucket,
    // push back a pointer to the source node of the current net.
    // We sent pointer because the bucket holds pointers to the nodes.
            temp.b[(out_edge - in_edge) + offset].push_back(&(net.source)); //the offset is necessary for -ve gains.
            
        }
        gainBucket = temp;
        cout << "cutsize: " << cutsize << endl;
		return cutsize;
    }

    // Constructor to read the number of vertices from a file and initialize everything.
    Graph(const string& filename) {
        ifstream inFile(filename);
        if (!inFile.is_open()) {
            cerr << "Error opening file " << filename << endl;
            exit(1);
        }

        // Read number of vertices
        inFile >> numVertices;

        // Initialize nodes vector
        nodes.resize(numVertices);

        int numNets;
        inFile >> numNets;
        circuit_nets.resizeNet(numVertices);

        for (int i = 0; i < numVertices; ++i) {
            nodes[i].value = i;
            nodes[i].partition_num = false; //assign all nodes to partition 1 initially.
            circuit_nets[i].source.value = i;
        }


        

        // Read nets
        string line;
        int source;
        node source_node;
        bool p = false;  //assign all nodes to partition 1 initially.

        getline(inFile, line);
        for (int i = 0; i < numNets; ++i) {
            getline(inFile, line);
            stringstream ss(line);

            ss >> source;
            source_node.value = source;
            source_node.partition_num = p;

            unordered_set<int> temp;
            vector<node> tv;
            node n;
            int node_value;

            
            
            while (ss >> node_value) {
                n.value = node_value; //assigns node value
                n.partition_num = p;  //assigns to partition 1.
				tv.push_back(n);  //add to vector of destination nodes
				temp.insert(node_value); //all the nodes in "tv" must be in "connected".
            }
            Net N1;
            N1.source = source_node;
            N1.tv = tv;
            N1.connected = temp;

            if (tv.size() > pmax) {
				pmax = tv.size();   //for bucket.
            }
            circuit_nets[source] = N1;  

        }

		inFile.close();

        //create random partitions.
        for (auto& x : circuit_nets) {
            double r = dis(gen);
			cout << "r:" << r << endl; 
            if (r < 0.5) {
                p = false;
            }
            else { p = true; }

			x.source.partition_num = p;
            if (x.source.partition_num == false) {
                partition1.push_back(x.source);
            }
            else {
				partition2.push_back(x.source);
            }

        /*This is where we initialize the unordered_set "connected" for 
        calculating the gain later. In each net of circuit_nets, we 
        iterate through the destination nodes(eg: y) of the net and add 
        the current source node(i.e., x.source.value) to the unordered_set 
        of the current destination node, i.e., y. 
        We then check for the largest connected node to set as pmax.
        */
            for (auto& y : x.tv) {
                circuit_nets[y.value].connected.insert(x.source.value);
                if (circuit_nets[y.value].connected.size() > pmax) {
					pmax = circuit_nets[y.value].connected.size();
                }
            }
        }

        /*the offset is equal to pmax because we want the gainBucket to hold
        values from +pmax to -pmax.*/
		offset = pmax;  
        cout << "pmax: " << pmax << endl;
        gainBucket.b.resize(2 * pmax + 1);


/*This is where we actually assign the nodes to partitions
* The indices of circuit_nets correspond to the respective source nodes
* For example, circuit_nets[1] holds the net N whose source node is 1.
*/
        cout << "partition1:" << endl;
        for (auto& x : partition1) {
            circuit_nets[x.value].source.partition_num = x.partition_num;
			cout << circuit_nets[x.value].source.value << circuit_nets[x.value].source.partition_num << endl;
        } 
        cout << "partition2:" << endl;
        for (auto& x : partition2) {
            circuit_nets[x.value].source.partition_num = x.partition_num;
            cout << circuit_nets[x.value].source.value << circuit_nets[x.value].source.partition_num << endl;
        }
        cout << "in the circuit:" << endl;
        for (auto& x : circuit_nets) {
			cout << x.source.value << x.source.partition_num << endl;
        }

      
        // Perform Fiduccia-Mattheyses algorithm
        fiduccia_mattheyses();

        //cutsize = calc_Cutsize_and_gain();
        
        // Output final cutsize
        cout << "Final cutsize after Fiduccia-Mattheyses: " << cutsize << endl;
    }


    /*Function to perform the Fiduccia - Mattheyses algorithm.
    * First, it selects the highest gain non empty node.
	* then it locks the node and moves it to the opposite partition.
    * then it recalculates the new gains and cutsize for all the nodes.
	* If the new cutsize is less than the current cutsize then
	* it saves the new cutsize and performs another node move.
    */ 
    void fiduccia_mattheyses() {

        // Calculate initial cutsize and gain
        cutsize=calc_Cutsize_and_gain();

        bool improvement = true;

        while(improvement){
                cout << "-------------------------------------------------" << endl;
                cout << "moving node..." << endl;
                
                node& k = findHighestNonEmptyIndex();
                if (&k == nullptr) {
                    cout << "all nodes locked." << endl;
					return;
                }
				cout << "node moved:" << k.value << endl; 
                k.locked = true;
                k.partition_num = !k.partition_num; //move to other partition.
                //recalculate new gains for all nodes
                int new_cutsize = calc_Cutsize_and_gain();  
                if (new_cutsize < cutsize) {
                    if (new_cutsize == 1) {
                        cutsize = new_cutsize;
                        cout << "minimum partitioning found." << endl;
                        return;
                    }
					if (new_cutsize > 1) { //if cutsize is 0, dont modify the current cutsize because there might free nodes still present.
                        cutsize = new_cutsize; 
                    }
                }
                else {
                    cout << "no more improvements." << endl;
                    improvement = false; 
                }
        }
    }
   
    /*This function is used to check if all the nodes
    * in a given index of our gainBucket are locked or not.
    * It returns a pair of values that are used by the
    * findHighestNonEmptyIndex() function.
    * The first return value in the pair returns true if 
    * all the nodes are locked and false otherwise.
    * The second value returns the node that is free or 
    * returns nullptr if no free node is found.
    */
    pair<bool, node*> check_all_nodes_locked(int i) {
        for (auto& x_ptr : gainBucket.b[i]) {
            node* x = x_ptr;
            if (!x->locked) { // Check if node is not locked
                return make_pair(false, x); // Return false and node pointer if found
            }
        }
        return make_pair(true, nullptr); // Return true and nullptr if all nodes are locked
    }

	/*This function is used to find the highest index where
    * a non-empty node can be found.
    */
    node& findHighestNonEmptyIndex() {
        int highestIndex = 0;
        node* k=nullptr;
        bool found = false;
        int i = 0;
        while(i < gainBucket.b.size()){ //iterate through every gain index.
            if (!gainBucket.b[i].empty()) {      // if the bucket index is not empty
                pair<bool, node*> result = check_all_nodes_locked(i);      //check for free nodes within that gain index.
				if (!result.first){         //if free node found
                    highestIndex = i;       //highest non empty index  
                    k = result.second;
                }                    
            }
            ++i;
        }
		return *k;
    }

    int return_cutsize() {
        return cutsize;
    }

};

//Number of iterations to run Fidducia-Matheyses.
const int NUM_OF_ITERATIONS = 100;
int main() {
    //Start timer.
    auto start = std::chrono::high_resolution_clock::now();

    string file = "input_net2.txt";
	int final_cutsize = INT_MAX;
    for (int i = 1; i <= NUM_OF_ITERATIONS; i++) {
        cout << endl;
        cout << "**********************ITERATION " << i << "*******************************" << endl;
        Graph g(file);
        if (g.return_cutsize() < final_cutsize) {
			final_cutsize = g.return_cutsize();
        }
        if (g.return_cutsize() == 1) {
            cout << "Minimum cutsize found." << endl;
            break;
        }
    }

    // Stop timer
    auto end = std::chrono::high_resolution_clock::now();
    // Calculate the duration
    std::chrono::duration<double> duration = end - start;
    // Convert duration to milliseconds
    double milliseconds = duration.count() * 1000.0;
    // Output the time taken
    cout << endl;
    std::cout << "Time taken: " << milliseconds << " milliseconds" << std::endl;
}
