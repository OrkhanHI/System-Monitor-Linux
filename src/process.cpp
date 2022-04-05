#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using namespace std;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const {  
    float seconds = Process::UpTime();
    float totalTimeSeconds = LinuxParser::ActiveJiffies(pid_)/sysconf(_SC_CLK_TCK);
    float cpuUsage = float(totalTimeSeconds/seconds);

    return cpuUsage;
}

string Process::Command() { 
  	string commandLine = LinuxParser::Command(pid_);
  	commandLine.resize(40);
    return commandLine+"..."; 
}

string Process::Ram() { 
    return LinuxParser::Ram(pid_); 
}

string Process::User() { 
    return LinuxParser::User(pid_);
}

long int Process::UpTime() const { 
    return LinuxParser::UpTime() - LinuxParser::UpTime(pid_); 
}

bool Process::operator<(Process const& a) { 
    return a.CpuUtilization() < this->CpuUtilization();
}