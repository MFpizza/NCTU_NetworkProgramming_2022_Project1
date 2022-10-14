#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>
#include <stdlib.h>
#include <signal.h>
#include <vector>

using namespace std;

void sig_handler(int signum);

template <typename T>
void myCout(T s);

void executeFunction(vector<string> parm);