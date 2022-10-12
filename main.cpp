#include "main.h"

int main()
{
    // about fork and process variable
    pid_t pid;
    string binPath = ".";

    // tmp variable
    string s;
    stringstream ss;
    char inputLine[15000];
    cout << "% ";
    vector<string> lineSplit;

    while (getline(cin, s))
    {
        // 分割當前的指令
        string delimiter = " ";

        size_t pos = 0;
        string token;
        while ((pos = s.find(delimiter)) != string::npos)
        {
            token = s.substr(0, pos);
            lineSplit.push_back(token);
            myCout(token);
            s.erase(0, pos + delimiter.length());
        }
        lineSplit.push_back(s);
        myCout(s);
        //--------------------------------------------------------
        
        

        cout << "% ";
    }
}

void sig_handler_parent(int signum)
{
    printf("Parent : Received a response signal from child \n");
}

void sig_handler_child(int signum)
{
    printf("Child : Received a signal from parent \n");
    sleep(1);
    kill(getppid(), SIGUSR1);
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