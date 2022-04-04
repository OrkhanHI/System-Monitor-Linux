#include "processor.h"
#include <linux_parser.h>

// Aggregate CPU utilization
float Processor::Utilization() {
    float idleJiffiesDelta = LinuxParser::IdleJiffies() - prevIdle_;
    float totalJiffiesDelta = LinuxParser::Jiffies() - prevTotal_;
    prevIdle_ = LinuxParser::IdleJiffies();
    prevTotal_ = LinuxParser::Jiffies();
    return (totalJiffiesDelta - idleJiffiesDelta)/totalJiffiesDelta;
}