#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>

#include <string>
#include <vector>
#include "ExecuteCommand.h"


int executeCommandold(char **pCommand, std::vector<std::string> & vLines) {
    int iResult = 0;

    int pipefd[2];
    pipe(pipefd);
    
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
    int i = WEXITSTATUS(waitstatus);    
    printf("Return2: %d (%d)\n", i, getpid());

    return iResult;
}


int main(int iArgC, char *apArgV[]) {
    int iResult = 0;

    const char* prog2[] = { "las", "-l", 0};
    const char* prog1[] = {"/usr/bin/python", "/home/jody/progs/multi_spc_QHG3/useful_stuff/gridpreparation/alt_interpolator.py", 
                           "/home/jody/progs/multi_spc_QHG3/useful_stuff/core_data/ETOPO1_Bed_g_gmt4.grd", "22", "fatalt.qdf", 0};
    const char* prog3 = "/usr/bin/python /home/jody/progs/multi_spc_QHG3/useful_stuff/gridpreparation/alt_interpolator.py /home/jody/progs/multi_spc_QHG3/useful_stuff/core_data/ETOPO1_Bed_g_gmt4.grd 22 fatalt.qdf";
    char sprog[2048];
    strcpy(sprog, prog3);

    char **argv = new char*[sizeof(prog1)/sizeof(char *)];
    for (uint i = 0; i < sizeof(prog1)/sizeof(char *); i++) {
        argv[i] = (char *) (prog1[i]);
    }

    std::vector<std::string> vLines;

    iResult =  executeCommand(sprog, vLines);
    //    iResult =  executeCommand(argv, vLines);

    for (unsigned int i = 0; i < vLines.size(); i++) {
        printf("%s\n", vLines[i].c_str());
    }
    printf("ok, byebye!\n");
    return iResult;
}

/*    char buffer[8192];
    //freopen("/dev/null", "a", stdout);
    //setbuf(stdout, buffer);
    //freopen("/dev/null", "a", stdout);
    setvbuf(stdout, buffer, _IOLBF, 1024);
  
    const char* prog1[] = { "ls", "-l",  0};
    
    char **argv = new char*[sizeof(prog1)/sizeof(char *)];
    for (uint i = 0; i < sizeof(prog1)/sizeof(char *); i++) {
        argv[i] = (char *) (prog1[i]);
    }
    execvp(argv[0],argv);
    perror("execvp of ls failed");
    
    

    freopen ("/dev/tty", "a", stdout);
    printf("buf [%s]\n", buffer);


    return iResult;
}
*/
