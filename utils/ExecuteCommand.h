#ifndef __EXECUTECOMMAND_H__
#define __EXECUTECOMMAND_H__

#include <string>
#include <vector>

int executeCommand(char **pCommand, std::vector<std::string> & vLines);
int executeCommand(std::string sCommand, std::vector<std::string> & vLines);

#endif
