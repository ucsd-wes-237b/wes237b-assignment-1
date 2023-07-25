#include "huffman.h"
#include <queue>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

// Node struct for our Huffman Tree
struct Node {
    unsigned char symbol;
    unsigned int freq;
    Node* left;
    Node* right;

    Node(unsigned char symbol, unsigned int freq) : symbol(symbol), freq(freq), left(nullptr), right(nullptr) {}
};

// Functor to allow us to compare node frequency values: is a's frequency > b's frequency
struct CompareNodes {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};
// global queue to store
priority_queue<Node*, vector<Node*>, CompareNodes> pq;
// Function to build Huffman tree
Node* buildHuffmanTree(const unordered_map<unsigned char, unsigned int>& freqMap) {

    // Insert all nodes into the queue
    for (const auto& newNode : freqMap) {
        pq.push(new Node(newNode.first, newNode.second));
    }

    // Repeat combining the two lowest frequencies to build tree
    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();

        Node* right = pq.top();
        pq.pop();

        Node* parentNode = new Node(0, left->freq + right->freq);
        parentNode->left = left;
        parentNode->right = right;
        pq.push(parentNode);
    }
    // return our final root node
    return pq.top();
}

// Recursive function to generate Huffman code for each symbol in our tree
void generateHuffmanCodes(Node* root, string code, unordered_map<unsigned char, string>& huffmanCodes) {
    // base case, if node exists
    if (!root)
        return;
    // If the node has a symbol, ie is not an intermediary node, store the code
    if (root->symbol) {
        huffmanCodes[root->symbol] = code;
    }
    // generate Huffman Codes for child nodes
    generateHuffmanCodes(root->left, code + "0", huffmanCodes);
    generateHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// Helper function to encode original data with our generated Huffman codes
string encodeData(const unsigned char* bufin, unsigned int bufinlen, const unordered_map<unsigned char, string>& huffmanCodes) {
    string encodedData;
    for (unsigned int i=0; i < bufinlen; ++i) {
        encodedData += huffmanCodes.at(bufin[i]);
    }
    return encodedData;
}

// Helper function to decode encded data using Huffman tree
string decodeData(const string& encodedData, Node* root) {
    string decodedData;
    Node* current = root;

    for (char bit : encodedData) {
        if (bit == '0') {
            current = current->left;
        } else if (bit == '1') {
            current = current->right;
        }

        if (current->symbol) {
            decodedData += current->symbol;
            current = root;
        }
    }
    return decodedData;
}

// Function to release Huffman tree from memory
void deleteHuffmanTree(Node* root) {
    if (!root)
        return;

    deleteHuffmanTree(root->left);
    deleteHuffmanTree(root->right);
    delete root;
}

// Main function for huffman encoding
int huffman_encode(const unsigned char *bufin, unsigned int bufinlen, unsigned char **pbufout, unsigned int *pbufoutlen) {
        // Check if our input buffer does not exist or is of length 0
    if (!bufin || bufinlen == 0)
        return -1;
    // Count and store frequencies of characters in the input buffer
    unordered_map<unsigned char, unsigned int> freqMap;
    for (unsigned int i=0; i < bufinlen; ++i) {
        freqMap[bufin[i]]++;
    }
    // Build our Huffman tree
    Node* root = buildHuffmanTree(freqMap);
    // Generate Huffman codes from tree
    unordered_map<unsigned char, string> huffmanCodes;
    generateHuffmanCodes(root, "", huffmanCodes);
    // Encode our input buffer with our generated codes
    string encodedData = encodeData(bufin, bufinlen, huffmanCodes);
    // Convert our encoded data string into binary
    unsigned int encodedDataSize = (encodedData.size() + 7)/ 8;
    *pbufoutlen = encodedDataSize;
    *pbufout = new unsigned char[encodedDataSize];

    unsigned int byteIndex = 0;
    for (unsigned int i=0; i < encodedData.size(); i+=8) {
        unsigned char byte = 0;
        for (int j=0; j < 8 && i+j < encodedData.size(); ++j) {
            if (encodedData[i+j] == '1') {
                byte |= (1 << (7-j));
            }
        }
        (*pbufout)[byteIndex] = byte;
        byteIndex++;
    }
    // Release tree from memory
    //deleteHuffmanTree(root);

    return 0;
}


// Main function for Huffman decoding
int huffman_decode(const unsigned char *bufin, unsigned int bufinlen, unsigned char **pbufout, unsigned int *pbufoutlen) {
        // Check if our input buffer does not exist or is of length 0
    if (!bufin || bufinlen == 0)
        return -1;
    // Reconstruct encoded string from binary
    string encodedData;
    for (unsigned int i=0; i < bufinlen; ++i) {
        for (int j=7; j >= 0; --j) {
            encodedData += ((bufin[i] & (1 << j)) ? '1' : '0');
        }
    }
    // Get our Huffman tree
    Node* root = pq.top();
    // Decode data using Huffman tree
    string decodedData = decodeData(encodedData, root);
    // Set the output buffer
    *pbufoutlen = decodedData.size();
    *pbufout = new unsigned char[*pbufoutlen];

    for (unsigned int i=0; i < *pbufoutlen; ++i) {
        (*pbufout)[i] = decodedData[i];
    }
    // Release tree from memory
    deleteHuffmanTree(root);
    return 0;
}
