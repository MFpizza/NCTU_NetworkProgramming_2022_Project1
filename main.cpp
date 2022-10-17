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
using namespace std;

void executeFunction(vector<string> parm);
int parserCommand(vector<string> SeperateInput);
/*

TODO: numberPipe withOut !

* Seems like multiple pipe need to pass the inputCommand to there child process
* so that the parent process could do nothing until the all child processes are done
* then continue to get the next inputCommand from user
TODO: Pass the inputCommand to the child process

TODO: Make Try to make child process can fork there child process and execute the command

TODO: 讓每個child process在其parent process還在運行的時候就透過pipe去獲取parent的資料，以免沒法pipe太多的東西
* 參考: https://stackoverflow.com/questions/7292642/grabbing-output-from-exec

TODO: 我好像把numberPipe的一切想得太美好，回去可能有很多bug或inform要確認
! 使用vector去存numberPipe會遇到兩個以上的numberPipe沒法在第一個做完之後去erase numberPipeArray

*/

struct myCommandLine
{
    vector<string> inputCommand; // store the command line
    bool backPipe = false;       // true if there is a pipe command in front of the command
    bool frontPipe = false;      // true if there is a pipe command behind of the command
    int FP = -1;                 // Front Pipe number
    int BP = -1;                 // Back Pipe number

    //* use to implement NumberPipe
    bool numberPipe = false;  // true if there is a number pipe command
    int numberPipeIndex = -1; // 還在思考要用甚麼方式來儲存numberPipe
};

void outputMyCommandLineOfPipe(myCommandLine my)
{
    cout << "Pipe information:\n";
    cout << "backPipe is " << my.backPipe << ",number of Pipe is " << my.BP << endl;
    cout << "frontPipe is " << my.frontPipe << ",number of Pipe is " << my.FP << endl;
}

int main()
{
    clearenv();
    setenv("PATH", "bin:.", 1);

    // tmp variable
    string s;
    cout << "% ";
    while (getline(cin, s))
    {
        if (s == "")
        {
            cout << "% ";
            continue;
        }
        vector<string> lineSplit;
        // 分割當前的指令
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
        //--------------------------------------------------------

        cout << "% ";
    }
}

void executeFunction(myCommandLine tag)
{
    // cerr<< tag.inputCommand[0]<<endl;
    const char **arg = new const char *[tag.inputCommand.size() + 1];
    int fd;
    for (int i = 0; i < tag.inputCommand.size(); i++)
    {
        // TODO: File Rediretion
        if (tag.inputCommand[i] == ">")
        {
            fd = open(tag.inputCommand[i + 1].c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
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
        // cout<<tag[i].c_str()<<endl;
        // cout<<argv[i]<<endl;
    }
    arg[tag.inputCommand.size()] = NULL;

    char **show = (char **)arg;
    // cerr<<" ready to execute "<<tag.inputCommand[0]<<endl;
    if (execvp(tag.inputCommand[0].c_str(), (char **)arg) == -1)
    {
        cerr << "Unknown Command: [" << tag.inputCommand[0] << "]." << endl;
        exit(-1);
    };
    // cerr << " error with command: " << tag.inputCommand[0] << endl;
}

struct myNumberPipe
{
    int number;
    int IndexOfGlobalPipe;
};
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
        printf("%s\n", getenv(SeperateInput[1].c_str()));
        return 1;
    }
    else if (SeperateInput[0] == "setenv")
    {
        setenv(SeperateInput[1].c_str(), SeperateInput[2].c_str(), 1);
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

    //用來儲存 NumberPipe後面的指令
    bool sameLine = false;
    vector<string> IfNumberPipeMiddle;
    while (count < SeperateInput.size())
    {
        if (SeperateInput[count][0] == '|' || SeperateInput[count][0] == '!')
        {
            // * 實作numberPipe 在最尾端
            if (SeperateInput[count].size() > 1)
            {
                stringstream ss;
                SeperateInput[count].erase(SeperateInput[count].begin());
                ss << SeperateInput[count];
                int Number;
                ss >> Number;
                // cerr<<Number<<endl;
                // TODO 把剩餘的code丟到 IfNumberPipeMiddle 裡面去 從count+1開始到SeperateInput.size()-1
                

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
                    // cerr<<"GlobalPipeIndex: "<<GlobalPipeIndex<<endl;
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
                    SeperateInput.erase(SeperateInput.begin()+count, SeperateInput.end());
                    // cerr<<count<<" "<<SeperateInput.size()<<endl;
                    // for(int j= 0; j < IfNumberPipeMiddle.size(); j++){
                    //     cerr<<IfNumberPipeMiddle[j]<<endl;
                    // }
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

                // TODO numberPipe

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
    int NumberPipeNeed = -1;
    for (int j = 0; j < NumberPipeArray.size(); j++)
    {
        if (NumberPipeArray[j].number == 0)
        {
            NumberPipeNeed = NumberPipeArray[j].IndexOfGlobalPipe;
            break;
        }
    }

    for (int i = 0; i < parseCommand.size(); i++)
    {
        // outputMyCommandLineOfPipe(parseCommand[i]);

        if (parseCommand[i].backPipe)
        {
            int fdPipe = pipe(pipeArray[parseCommand[i].BP]);
            // cout<<fdPipe<<endl;
            if (fdPipe == -1)
            {
                cerr << "pipe generate failed" << endl;
            }
        }

        pid = fork();
        // cout<<"child pid "<< pid<<endl;
        if (pid == 0) // child process
        {

            // * front Pipe
            if (parseCommand[i].frontPipe)
            {
                int FPNumber = parseCommand[i].FP;
                // cout<<"front pipe "<< FPNumber<<endl;
                close(pipeArray[FPNumber][1]);
                dup2(pipeArray[FPNumber][0], 0);
                // cerr<<"dup2 fp endl"<<endl<<endl;
                close(pipeArray[FPNumber][0]);
            }

            // * back Pipe
            if (parseCommand[i].backPipe)
            {
                int BPNumber = parseCommand[i].BP;
                // cout<<"back Pipe "<< BPNumber<<endl;
                close(pipeArray[BPNumber][0]);
                dup2(pipeArray[BPNumber][1], 1);
                // cerr<<"dup2 bp endl"<<endl<<endl;
                close(pipeArray[BPNumber][1]);
            }

            // * number Pipe behind
            if (parseCommand[i].numberPipe)
            {
                // cerr<<parseCommand[i].inputCommand[0]<<" "<<parseCommand[i].numberPipeIndex<<endl;
                int NumberPipeIndex = parseCommand[i].numberPipeIndex;
                close(GlobalPipe[NumberPipeIndex][0]);
                dup2(GlobalPipe[NumberPipeIndex][1], 1);
                // cerr<<"dup2 bp endl"<<endl<<endl;
                close(GlobalPipe[NumberPipeIndex][1]);
            }

            //  * handle numberPipe stdIn
            if (i == 0 && NumberPipeNeed != -1)
            {
                int pipeIndex = NumberPipeArray[NumberPipeNeed].IndexOfGlobalPipe;
                close(GlobalPipe[pipeIndex][1]);
                dup2(GlobalPipe[pipeIndex][0], 0);
                // cerr<<"dup2 fp endl"<<endl<<endl;
                close(GlobalPipe[pipeIndex][0]);
            }

            executeFunction(parseCommand[i]);
            cerr << parseCommand[i].inputCommand[0] << " exec error" << endl;
            exit(0);
        }
        else if (pid > 0) // parent  process
        {
            if (parseCommand[i].frontPipe)
            {
                close(pipeArray[parseCommand[i].FP][0]);
                close(pipeArray[parseCommand[i].FP][1]);
            } // sleep(2);

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

            // cout << "parent process continue to run the next Command" << endl;
        }
        else // fork error
        {
            cerr << "fork error" << endl;
            exit(1);
        }
    }

    // cout << endl
    //      << "all command run" << endl;

    //* 等待所有child process exit
    while ((wpid = wait(&status)) > 0)
    {
    };
    // cout << endl
    //      << "child process every exit" << endl;
    if (sameLine)
    {
        parserCommand(IfNumberPipeMiddle);
    }
    return 1;
}
