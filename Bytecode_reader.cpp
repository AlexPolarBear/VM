#include <iostream>
#include <fstream>
#include <string>
#iclude "../PLTools/Lama/blob/1.10/byterun/byterun.c"

using namespace std;

int main() {
	unsigned int x;
	unsigned char b;
	ifstream myFile(bytefile, ios::out | ios::binary);
	myFile.seekg(0, ios::beg);
	myFile.read(reinterpret_cast<char*>(&b), sizeof(b));
	x = (unsigned int)b;
	return b;
}
