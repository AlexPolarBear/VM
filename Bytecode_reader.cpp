#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
	unsigned int x;
	unsigned char b;
	ifstream myFile("example.txt", ios::out | ios::binary);
	myFile.seekg(0, ios::beg);
	myFile.read(reinterpret_cast<char*>(&b), sizeof(b));
	x = (unsigned int)b;
	return b;
}
