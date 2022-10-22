#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <vector>
#include <fcntl.h>
#include <fstream>
#include <pwd.h>
using namespace std;

struct myNumberPipe
{
    int number;            // Next number time to pipe the output
    int IndexOfGlobalPipe; // Index of global pipe
};

struct myCommandLine
{
    vector<string> inputCommand; // store the command line
    bool backPipe = false;       // true if there is a pipe command behind of the command
    bool frontPipe = false;      // true if there is a pipe command in front of the command
    int FP = -1;                 // Front Pipe number
    int BP = -1;                 // Back Pipe number
    bool numberPipe = false;     // true if there is a number pipe command
    int numberPipeIndex = -1;    // 還在思考要用甚麼方式來儲存numberPipe
    bool errPipeNeed = false;    // true if there is a pipe command be
};

void executeFunction(myCommandLine tag);
int parserCommand(vector<string> SeperateInput);

void signalHandler(int sig)
{
    pid_t pid = wait(NULL);
}

int main()
{
    signal(SIGCHLD, signalHandler);
    clearenv();
    setenv("PATH", "bin:.", 1);

    // tmp variable
    string s;
    cout << "% ";
    while (getline(cin, s))
    {
 	out<<s<<endl;

       if (s == "")
        {
            cout << "% ";
            continue;
        }

        // 分割當前的指令
        vector<string> lineSplit;
        string delimiter = " ";

        size_t pos = 0;
        string token;
        while ((pos = s.find(delimiter)) != string::npos)
        {
            token = s.substr(0, pos);
            lineSplit.push_back(token);
            s.erase(0, pos + delimiter.length());
        }

        lineSplit.push_back(s);

        parserCommand(lineSplit);

        cout << "% ";
    }
}

vector<myNumberPipe> NumberPipeArray;
int GlobalPipe[1000][2];
bool GlobalPipeUsed[1000];

int findTheGlobalPipeCanUse()
{
    for (int i = 0; i < 1000; i++)
    {
        if (!GlobalPipeUsed[i])
        {
            GlobalPipeUsed[i] = !GlobalPipeUsed[i];

            if (pipe(GlobalPipe[i]) == -1)
            {
                cerr << "GlobalPipe gen failed" << endl;
                exit(-1);
            }

            return i;
        }
    }
    cerr << "All GlobalPipe Used" << endl;
    return -1;
}

int parserCommand(vector<string> SeperateInput)
{
    // TODO 將所有numberPipe向前一格
    for (int j = 0; j < NumberPipeArray.size(); j++)
    {
        NumberPipeArray[j].number = NumberPipeArray[j].number - 1;
    }

    if (SeperateInput[0] == "printenv")
    {
        if (getenv(SeperateInput[1].c_str()) != NULL)
        {
            printf("%s\n", getenv(SeperateInput[1].c_str()));
        }
        return 1;
    }
    else if (SeperateInput[0] == "setenv")
    {
        if (setenv(SeperateInput[1].c_str(), SeperateInput[2].c_str(), 1) == -1)
        {
            cerr << "setenv error" << endl;
            exit(-1);
        };
        return 1;
    }
    else if (SeperateInput[0] == "exit")
    {
        exit(0);
    }

    pid_t pid, wpid;
    int status = 0;

    int count = 0, parseCommandLine = 0, pipeNumber = 0;
    vector<myCommandLine> parseCommand;
    parseCommand.resize(1);

    bool hasNumberPipe = false;
    //用來儲存 NumberPipe後面的指令
    bool sameLine = false;
    vector<string> IfNumberPipeMiddle;

    while (count < SeperateInput.size())
    {
        if (SeperateInput[count][0] == '|' || SeperateInput[count][0] == '!')
        {
            if (SeperateInput[count][0] == '!')
            {
                parseCommand[parseCommandLine].errPipeNeed = true;
            }

            // * 實作numberPipe
            if (SeperateInput[count].size() > 1)
            {
                stringstream ss;
                SeperateInput[count].erase(SeperateInput[count].begin());
                ss << SeperateInput[count];
                int Number;
                ss >> Number;

                hasNumberPipe = true;
                bool hasPipe = false;
                parseCommand[parseCommandLine].numberPipe = true;

                for (int k = 0; k < NumberPipeArray.size(); k++)
                {
                    if (NumberPipeArray[k].number == Number)
                    {
                        hasPipe = true;
                        parseCommand[parseCommandLine].numberPipeIndex = k;
                    }
                }

                if (!hasPipe)
                {
                    int GlobalPipeIndex = findTheGlobalPipeCanUse();
                    myNumberPipe nP;
                    nP.number = Number;
                    nP.IndexOfGlobalPipe = GlobalPipeIndex;
                    NumberPipeArray.push_back(nP);
                    parseCommand[parseCommandLine].numberPipeIndex = GlobalPipeIndex;
                }

                if (count != SeperateInput.size() - 1)
                {
                    sameLine = true;
                    for (int j = count + 1; j < SeperateInput.size(); j++)
                    {
                        IfNumberPipeMiddle.push_back(SeperateInput[j]);
                    }
                    SeperateInput.erase(SeperateInput.begin() + count, SeperateInput.end());
                    break;
                }
            }

            //* 實作普通的pipe
            else if (count != SeperateInput.size() - 1)
            {
                parseCommand[parseCommandLine].backPipe = true;
                parseCommand[parseCommandLine].BP = pipeNumber;
                myCommandLine newCommand;
                newCommand.FP = pipeNumber;
                newCommand.frontPipe = true;
                parseCommand.push_back(newCommand);
                parseCommandLine++;
                pipeNumber++; // 紀錄需要創建幾個pipe
            }
            
            count++;
            if (count == SeperateInput.size())
            {
                break;
            }
        }
        // cout<<SeperateInput[count]<<endl;
        parseCommand[parseCommandLine].inputCommand.push_back(SeperateInput[count]);
        count++;
    }

    int pipeArray[pipeNumber][2];
    // cout<<pipeNumber<<endl<<endl;
    for (int i = 0; i < pipeNumber; i++)
    {
        int fdPipe = pipe(pipeArray[i]);
        if (fdPipe == -1)
        {
            cerr << "pipe generate failed" << endl;
        }
    }
    // cerr<<pipeNumber<<endl;

    int NumberPipeNeed = -1;
    for (int j = 0; j < NumberPipeArray.size(); j++)
    {
        if (NumberPipeArray[j].number == 0)
        {
            NumberPipeNeed = NumberPipeArray[j].IndexOfGlobalPipe;
            break;
        }
    }

    // cerr<<parseCommand.size()<<endl;
    for (int i = 0; i < parseCommand.size(); i++)
    {

        pid = fork();
        if (pid == 0) // child process
        {

            // * front Pipe
            if (parseCommand[i].frontPipe)
            {
                int FPNumber = parseCommand[i].FP;
                close(pipeArray[FPNumber][1]);
                dup2(pipeArray[FPNumber][0], 0);
                close(pipeArray[FPNumber][0]);
            }

            // * back Pipe
            if (parseCommand[i].backPipe)
            {
                int BPNumber = parseCommand[i].BP;
                close(pipeArray[BPNumber][0]);
                dup2(pipeArray[BPNumber][1], 1);
                if (parseCommand[i].errPipeNeed)
                {
                    dup2(GlobalPipe[BPNumber][1], 2);
                }
                close(pipeArray[BPNumber][1]);
            }

            // * number Pipe behind
            if (parseCommand[i].numberPipe)
            {
                // cerr<<"numberpipe"<<endl;
                int NumberPipeIndex = parseCommand[i].numberPipeIndex;
                close(GlobalPipe[NumberPipeIndex][0]);
                dup2(GlobalPipe[NumberPipeIndex][1], 1);
                if (parseCommand[i].errPipeNeed)
                {
                    dup2(GlobalPipe[NumberPipeIndex][1], 2);
                }
                close(GlobalPipe[NumberPipeIndex][1]);
            }

            //  * handle numberPipe stdIn
            if (i == 0 && NumberPipeNeed != -1)
            {
                int pipeIndex = NumberPipeArray[NumberPipeNeed].IndexOfGlobalPipe;
                close(GlobalPipe[pipeIndex][1]);
                dup2(GlobalPipe[pipeIndex][0], 0);
                close(GlobalPipe[pipeIndex][0]);
            }

            executeFunction(parseCommand[i]);
        }
        else if (pid > 0) // parent  process
        {
            if (parseCommand[i].frontPipe)
            {
                close(pipeArray[parseCommand[i].FP][0]);
                close(pipeArray[parseCommand[i].FP][1]);
            }

            // TODO 確定所有NumberPipe向前1格 並且在這邊將已經倒數到0的Pipe close 並將 globalPipeUsed 設為 false
            for (int j = 0; j < NumberPipeArray.size(); j++)
            {
                if (NumberPipeArray[j].number == 0) 
                {
                    int index = NumberPipeArray[j].IndexOfGlobalPipe;
                    close(GlobalPipe[index][0]);
                    close(GlobalPipe[index][1]);
                    GlobalPipeUsed[index] = false;
                    NumberPipeArray.erase(NumberPipeArray.begin() + j);
                }
            }
        }
        else // fork error
        {
            // cerr << "fork error" << endl;
            i--;
            continue;
        }
    }
    if (sameLine)
    {
        parserCommand(IfNumberPipeMiddle);
    }

    if (!hasNumberPipe)
    {
        while ((wpid = wait(&status)) > 0)
        {
        };
    }

    return 1;
}

void executeFunction(myCommandLine tag)
{
    const char **arg = new const char *[tag.inputCommand.size() + 1];
    for (int i = 0; i < tag.inputCommand.size(); i++)
    {
        if (tag.inputCommand[i] == ">")
        {
            int fd = open(tag.inputCommand[i + 1].c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
            if (fd < 0)
            {
                cerr << "open failed" << endl;
            }

            if (dup2(fd, 1) < 0)
            {
                cerr << "dup error" << endl;
            }
            close(fd);
            arg[i] = NULL;
            break;
        }

        arg[i] = tag.inputCommand[i].c_str();
    }
    arg[tag.inputCommand.size()] = NULL;

    if (execvp(tag.inputCommand[0].c_str(), (char **)arg) == -1)
    {
        cerr << "Unknown command: [" << tag.inputCommand[0] << "]." << endl;
        exit(-1);
    };
}
