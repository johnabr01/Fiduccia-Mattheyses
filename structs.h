#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
using namespace std;

//NOTE: When I use "", I am referring to the exact name of the variable I used in the code.

struct node {
    bool partition_num;
    int value;
    bool locked = false;
};

struct Net {
/*A net has a source node and may have multiple destination nodes. 
The destination nodes are stored in vector<node> tv  variable.
The source node (eg:0) can be also be destination node for some other node(eg:7).
The unordered_set stores all the nodes in "tv" along with nodes like "7".
We use the "tv" variable to calculate the cutsize.
We use the "unordered_set" to calculate the gain for each net's source node.
You can think of "tv" as a subset of "connected" where all the nodes in "tv" 
are present in "connected" but not vice versa.
    */
    node source;
    vector<node> tv;    //to_vertices
    unordered_set<int> connected;
};

/*A bucket is a 2d vector of pointers to nodes.
the first index would be the gain values ranging from +pmax to -pmax.
At each gain index, we will have a vector of pointers 
pointing to the nodes that have that gain value.
*/
class Bucket {
public:
    Bucket() {};
    vector<vector<node*>> b;
};

class circuit
{
public:
    circuit() {};

    //resize the length of circuit_nets.
    void resizeNet(int size) {
        circuit_nets.resize(size);
    }

    // Define operator[] to access and modify elements at specific indices of circuit_nets.
    Net& operator[] (size_t index) {
        if (index >= circuit_nets.size()) {
            cout << "ERROR:out of range of circuit_nets" << endl;
			exit(1);
        }
        return circuit_nets[index];
    }

    // Define begin() and end() functions to provide iterator interface
    vector<Net>::iterator begin() {
        return circuit_nets.begin();
    }

    vector<Net>::iterator end() {
        return circuit_nets.end();
    }
private:
    vector<Net> circuit_nets;
};
