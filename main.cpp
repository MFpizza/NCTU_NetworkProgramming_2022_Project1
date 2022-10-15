#include "main.h"

/*

TODO: pipe without numberPipe


* Seems like multiple pipe need to pass the inputCommand to there child process
* so that the parent process could do nothing until the all child processes are done
* then continue to get the next inputCommand from user
TODO: Pass the inputCommand to the child process

TODO: Make Try to make child process can fork there child process and execute the command

TODO: 讓每個child process在其parent process還在運行的時候就透過pipe去獲取parent的資料，以免沒法pipe太多的東西
* 參考: https://stackoverflow.com/questions/7292642/grabbing-output-from-exec

*/

struct myCommandLine
{
    vector<string> inputCommand; // store the command line
    bool backPipe = false;       // true if there is a pipe command in front of the command
    bool frontPipe = false;      // true if there is a pipe command behind of the command
    int FP = -1;                 // Front Pipe number
    int BP = -1;                 // Back Pipe number

    //* use to implement NumberPipe
    bool numberPipe = false;
    int numberPipeIndex = -1;
};

void outputMyCommandLineOfPipe(myCommandLine my)
{
    cout << "Pipe information:\n";
    cout << "backPipe is " << my.backPipe << ",number of Pipe is " << my.BP << endl;
    cout << "frontPipe is " << my.frontPipe << ",number of Pipe is " << my.FP << endl;
}
struct sigaction siga;

void f(int sig) {
    printf("Caught signal %d\n", sig);
}

// sets f as handler to all the possible signals.
void myfunct(void(*f)(int sig)) {
    siga.sa_handler = f;
    for (int sig = 1; sig <= SIGRTMAX; ++sig) {
        // this might return -1 and set errno, but we don't care
        sigaction(sig, &siga, NULL);
    }
}
int main()
{
    //myfunct(f);
    clearenv();
    setenv("PATH", "bin:.", 1);
    // printf("%s\n", getenv("PATH"));
    //  about fork and process variable

    // tmp variable
    string s;
    stringstream ss;
    char inputLine[15000];
    cout << "% ";

    while (getline(cin, s))
    {
        vector<string> lineSplit;
        // 分割當前的指令
        string delimiter = " ";

        size_t pos = 0;
        string token;
        while ((pos = s.find(delimiter)) != string::npos)
        {
            token = s.substr(0, pos);
            lineSplit.push_back(token);
            // myCout(token);
            s.erase(0, pos + delimiter.length());
        }

        lineSplit.push_back(s);
        // myCout(s);

        parseCommand(lineSplit);
        //--------------------------------------------------------

        cout << "% ";
    }
}

template <typename T>
void myCout(T s)
{
    bool needCout = true;
    if (needCout)
        cout << s << endl;
    else
        return;
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
    // cerr<<" ready to execute"<<endl;
    if (execvp(tag.inputCommand[0].c_str(), (char **)arg) == -1)
    {
        cerr << "Unknown Command" << endl;
        exit(-1);
    };
}

int parseCommand(vector<string> SeperateInput)
{

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
    while (count < SeperateInput.size())
    {
        if (SeperateInput[count][0] == '|' || SeperateInput[count][0] == '!')
        {
            parseCommand[parseCommandLine].backPipe = true;
            parseCommand[parseCommandLine].BP = pipeNumber;
            myCommandLine newCommand;
            newCommand.FP = pipeNumber;
            newCommand.frontPipe = true;
            parseCommand.push_back(newCommand);
            parseCommandLine++;

            count++;
            pipeNumber++; // 紀錄需要創建幾個pipe

            //! 正在處理pipe 所以跳過pipe資訊並跳過numberPipe
        }
        // cout<<SeperateInput[count]<<endl;
        parseCommand[parseCommandLine].inputCommand.push_back(SeperateInput[count]);
        count++;
    }

    // for (int i = 0; i < parseCommand.size(); i++)
    // {
    //     cout << "i:" << i << endl;
    //     for (int j = 0; j < parseCommand[i].size(); j++)
    //     {
    //         cout << "line " << j << ":" << parseCommand[i][j] << " ";
    //         cout << endl;
    //     }

    // }

    for (int i = 0; i < parseCommand.size(); i++)
    {
        int pipeArray[pipeNumber][2];

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
                dup2(pipeArray[BPNumber][1],1);
                // cerr<<"dup2 bp endl"<<endl<<endl;
                close(pipeArray[BPNumber][1]);
            }

            //! 暫時還沒處理pipe 連接stdin stdout的問題
            // TODO: 如果有pipe就看是要將輸出對應到哪個pipe

            executeFunction(parseCommand[i]);
            cerr << parseCommand[i].inputCommand[0] << " exec error" << endl;
            exit(0);
        }
        else if (pid > 0) // parent  process
        {
            // close(pipeArray[0][0]);
            // close(pipeArray[0][1]);
            sleep(2);
            // cout << "parent process continue to run the next Command" << endl;
        }
        else // fork error
        {
            cerr << "fork error" << endl;
            exit(1);
        }
    }

    cout << endl
         << "all command run" << endl;

    //* 等待所有child process exit
    while ((wpid = wait(&status)) > 0)
    {
    };
    cout << endl
         << "child process every exit" << endl;
    return 1;
}
