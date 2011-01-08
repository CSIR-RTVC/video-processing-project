#pragma once

#include <map>
#include <string>
#include <vector>

typedef std::vector<std::string> StringList_t;
typedef std::map<std::string, std::string> StringMap_t;

#define SAFE_DELETE(a) if(a){delete a;a=NULL;}

