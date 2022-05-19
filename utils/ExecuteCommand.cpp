#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>

#include <string>
#include <vector>
#include "stdstrutils.h"
#include "ExecuteCommand.h"

int executeCommand(std::string sCommand, std::vector<std::string> & vLines) {
    int iResult = 0;
    std::vector<std::string> vParts;
    uint iNum = splitString(sCommand, vParts, " \t\n");

    char **args = new char*[iNum+1];
    for (unsigned int i = 0; i < vParts.size(); i++) {
        args[i] = strdup(vParts[i].c_str());
    }
    args[vParts.size()] = NULL;

    iResult = executeCommand(args, vLines);
    
    for (unsigned int i = 0; i < vParts.size(); i++) {
        free(args[i]);
    }
    delete[] args;
    return iResult;
}
    
    

int executeCommand(char **pCommand, std::vector<std::string> & vLines) {
    int iResult = 0;

    int pipefd[2];
    iResult = pipe(pipefd);
    if (iResult == 0) {
        pid_t pid = fork();

        if (pid == 0) {
            // this is the child
            close(pipefd[0]);    // close reading end in the child
        
            dup2(pipefd[1], 1);  // send stdout to the pipe
            dup2(pipefd[1], 2);  // send stderr to the pipe
        
            close(pipefd[1]);    // this descriptor is no longer needed
        
            int iRes = execvp(pCommand[0], pCommand);
            // exec only returns if it fails
            exit(iRes);

        } else if (pid > 0) {
            // this is the parent
            char buffer[1024];
            char prevbuf[1024];
            memset(buffer, 0,  1024);

            std::string sPrev;
            close(pipefd[1]);  // close the write end of the pipe in the parent

            // now split the buffer contents into strings and put them into the vector
            while (read(pipefd[0], buffer, sizeof(buffer)-1) != 0) {
            
                memset(prevbuf, 0,  1024);
                buffer[sizeof(buffer)-1] = '\0';
                char *pPrev = buffer;
                char *pCur = NULL;
                for (pCur = buffer; *pCur != '\0'; pCur++) {
                    if (*pCur == '\n') {
                        *pCur = '\0';
                        vLines.push_back(sPrev+pPrev);
                        sPrev.clear();
                        pPrev = pCur+1;
                    }
                }
                sPrev = pPrev;
                //            strcpy(prevbuf, pPrev);
                memset(buffer, 0,  1024);
        
            }
            if (sPrev.size() > 0) {
                vLines.push_back(sPrev);
            }

        } else {
            iResult = -1;
        }
    
        // wait ofr the child to finish and gtet the return code
        int waitstatus;
        wait(&waitstatus);
        //int i = WEXITSTATUS(waitstatus);    
        //printf("Return2: %d (%d)\n", i, getpid());
    } else {
        fprintf(stderr, "couldn't create pipes\n");
    }
    return iResult;
}
