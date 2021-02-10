#include "util.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string getFileContents(const std::string& filename){
	std::ifstream fp(filename, std::ios::binary|std::ios::in);
	std::string contents;
	std::stringstream buffer;
	if(fp.fail()){
		throw std::runtime_error("Could not open file "+filename);
	}
	buffer << fp.rdbuf();
	contents = buffer.str();
	fp.close();
	return contents;
}
