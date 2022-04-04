#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const {  
    float seconds = LinuxParser::UpTime() -  LinuxParser::UpTime(pid_);
    float totalTimeSeconds = LinuxParser::ActiveJiffies(pid_)/sysconf(_SC_CLK_TCK);
    float cpuUsage = float(totalTimeSeconds/seconds);
    //Restrict the boundaries
    if(cpuUsage*100 >= 100.0) {
        return 1.0;
    } else if(cpuUsage*100 < 0.0){
        return 0.0;
    }
    return cpuUsage;
}

string Process::Command() { 
    return LinuxParser::Command(pid_); 
}

string Process::Ram() { 
    return LinuxParser::Ram(pid_); 
}

string Process::User() { 
    return LinuxParser::User(pid_);
}

long int Process::UpTime() const { 
    return LinuxParser::UpTime(pid_); 
}

bool Process::operator<(Process const& a) { 
    return a.CpuUtilization() < this->CpuUtilization();
}