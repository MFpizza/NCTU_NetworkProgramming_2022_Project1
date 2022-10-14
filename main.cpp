#include "main.h"

/*


* Seems like multiple pipe need to pass the inputCommand to there child process
* so that the parent process could do nothing until the all child processes are done
* then continue to get the next inputCommand from user
TODO: Pass the inputCommand to the child process

TODO: Make Try to make child process can fork there child process and execute the command

TODO: 讓每個child process在其parent process還在運行的時候就透過pipe去獲取parent的資料，以免沒法pipe太多的東西
* 參考: https://stackoverflow.com/questions/7292642/grabbing-output-from-exec

*/

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

    int count = 0, parseCommandLine = 0;
    vector<vector<string>> parseCommand;
    parseCommand.resize(1);
    while (count < SeperateInput.size())
    {
        if (SeperateInput[count][0] == '|' || SeperateInput[count][0] == '!')
        {
            vector<string> newCommand;
            parseCommand.push_back(newCommand);
            parseCommandLine++;

            //! 這個只是暫時先跳過pipe
            count++;
            //! 還需要紀錄pipe前後的資訊
            //! 但我暫時還沒要處理pipe所以先不管
        }
        // cout<<SeperateInput[count]<<endl;
        parseCommand[parseCommandLine].push_back(SeperateInput[count]);
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
        pid = fork();

        if (pid == 0) // child process
        {
            //! 暫時還沒處理pipe 連接stdin stdout的問題
            // TODO: if 還有多的指令，可能是採用continue去繼續fork並運行 else 結束子processes

            executeFunction(parseCommand[i]);
            exit(0);
        }
        else if (pid > 0) // parent  process
        {
        }
        else // fork error
        {
            cerr << "fork error" << endl;
            exit(1);
        }
    }

    //* 等待所有child process exit
    while ((wpid = wait(&status)) > 0)
    {
    };
    return 1;
}

void executeFunction(vector<string> tag)
{
    //cout << parm.size() << endl;
    const char **arg = new const char* [tag.size()];

    for (int i = 0; i < tag.size(); i++)
    {
        arg[i] = tag[i].c_str();
        // cout<<tag[i].c_str()<<endl;
        // cout<<argv[i]<<endl;
    }
    // arg[tag.size()] = NULL;
    // for (int i = 0; i < parm.size() - 1; i++)
    // {
    //     cout << (argv[i]) << endl;
    // }
    // cout << "parm[0]:" << parm[0].c_str() << endl;
    if (execvp(tag[0].c_str(), (char**)arg) == -1)
    {
        cerr << "Unknown Command" << endl;
        exit(-1);
    };
}

int main()
{
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