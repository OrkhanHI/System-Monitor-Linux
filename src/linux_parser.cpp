#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Return OS info
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Return Kernel info
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Return vector of pids
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

//Return simple memory utilization without cache info
float LinuxParser::MemoryUtilization() { 
  float memTotal{1}, memFree{0}, value;
  string line, key;
  bool isTotalFound{false}, isFreeFound{false};
  std::ifstream filestream(kProcDirectory+kMeminfoFilename);
  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if(key=="MemTotal"){
          memTotal = value;
          isTotalFound = true;
        } else if(key=="MemFree"){
          memFree = value;
          isFreeFound = true;
        }
      if(isTotalFound && isFreeFound) break;
      }
    }
  }  
  return (memTotal - memFree) / memTotal;
}

// Return the system uptime
long LinuxParser::UpTime() { 
  string line;
  float time1{0.0}, time2;
  std::ifstream upTimeFile(kProcDirectory+kUptimeFilename);
  if(upTimeFile.is_open()){
    std::getline(upTimeFile, line);
    std::istringstream timeLine(line);
    timeLine >> time1 >> time2;
  }
  return long(time1);
}

// Return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read PID Stat file
vector<long> processCPU(int pid){
  string line, fString, sString;
  long value;
  long uTime, sTime, cuTime, csTime, startTime;
  std::ifstream pidStat(LinuxParser::kProcDirectory+to_string(pid)+LinuxParser::kStatFilename);
  if(pidStat.is_open()){
    std::getline(pidStat, line);
    std::istringstream linestream(line);
    linestream >> value >> fString >> sString;
    int i=0;
    while(linestream >> value){
      if(i==10){ //10, 11, 12, 13, 18
        uTime = value;
      } else if(i==11){
        sTime = value;
      } else if(i==12){
        cuTime = value;
      } else if(i==13){
        csTime = value;
      } else if(i==18){
        startTime = value;
      }
      i+=1;
    } 
  }
  return {uTime, sTime, cuTime, csTime, startTime};
}

enum CPUTime {kUtime=0, kStime, kCUtime, kCStime, kStartTime};

// Return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  vector<long> pidResult = processCPU(pid);
  float totalTime = pidResult[CPUTime::kUtime] + pidResult[CPUTime::kStime] + pidResult[CPUTime::kCUtime] + pidResult[CPUTime::kCStime];
  return totalTime;
}

// Return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<string> allValues = LinuxParser::CpuUtilization();
  long systemAllTime = std::stol(allValues[kSystem_]) + std::stol(allValues[kIRQ_]) + std::stol(allValues[kSoftIRQ_]);
  long virtAllTime = std::stol(allValues[kGuest_])+std::stol(allValues[kGuestNice_]); 
  long userTime = std::stol(allValues[kUser_]) - std::stol(allValues[kGuest_]);
  long niceTime = std::stol(allValues[kNice_]) - std::stol(allValues[kGuestNice_]);
  return systemAllTime + virtAllTime + userTime + niceTime + std::stol(allValues[kSteal_]);
}

// Return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> allValues = LinuxParser::CpuUtilization();
  return std::stol(allValues[kIdle_]) + std::stol(allValues[kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() { 
  string line, cpu, value;
  vector<string> result;
  std::ifstream cpuUtilFile(kProcDirectory+kStatFilename);
  if(cpuUtilFile.is_open()){
    std::getline(cpuUtilFile, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while(linestream >> value){
      result.emplace_back(value);
    }
  }
  return result; 
}

// Return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line, key;
  int value{0};
  std::ifstream totalProcesses(kProcDirectory+kStatFilename);
  if(totalProcesses.is_open()){
    while(std::getline(totalProcesses, line)){
      std::istringstream statLine(line);
      while(statLine >> key >> value){
        if(key == "processes"){
          return value;
        }
      }
    }
  }
  return value; 
}

// Return the number of running processes
int LinuxParser::RunningProcesses() { 
  string line, key;
  int value{0};
  std::ifstream totalProcesses(kProcDirectory+kStatFilename);
  if(totalProcesses.is_open()){
    while(std::getline(totalProcesses, line)){
      std::istringstream statLine(line);
      while(statLine >> key >> value){
        if(key == "procs_running"){
          return value;
        }
      }
    }
  }
  return value; 
}

// Return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream commandLine(kProcDirectory+to_string(pid)+kCmdlineFilename);
  if(commandLine.is_open()){
    std::getline(commandLine, line);
    return line;
  }
  return line;
}

// Return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line, key; 
  int value{0};
  std::ifstream ramStatus(kProcDirectory+to_string(pid)+kStatusFilename);
  if(ramStatus.is_open()){
    while(std::getline(ramStatus, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if(key=="VmSize"){
          value /= 1024;
          return to_string(value);
        }
      }
    }
  }
  return to_string(value);
}

// Return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line, key, value;
  std::ifstream pidStatus(kProcDirectory+to_string(pid)+kStatusFilename);
  if(pidStatus.is_open()){
   while(std::getline(pidStatus, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if(key=="Uid"){
          return value;
        }
      }
   }
  }
  return value; 
}

// Return the user associated with a process
string LinuxParser::User(int pid) { 
  string uId = LinuxParser::Uid(pid);
  string newLine, x, userName, uIdNumber;
  std::ifstream etcPassword(kPasswordPath);
  if(etcPassword.is_open()){
    while(std::getline(etcPassword, newLine)){
      std::replace(newLine.begin(), newLine.end(), ':', ' ');
      std::istringstream linestream(newLine);
      while(linestream >> userName >> x >> uIdNumber){
        if(uIdNumber==uId){
          return userName;
        }
      }
    }
  }
  return userName; 
}

// Return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  vector<long> pidResult = processCPU(pid);
  return pidResult[CPUTime::kStartTime]/sysconf(_SC_CLK_TCK);
}
