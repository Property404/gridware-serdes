#ifndef __INC_UTIL_H
#define __INC_UTIL_H
#include <string>
/* Some utilities to make network simulation easier */

// Get file contents as binary
// Throw exception if it DNE
std::string getFileContents(const std::string& path);

#endif
