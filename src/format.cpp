#include <string>
#include <sstream>
#include "format.h"
#include <iomanip>

using std::string;
using std::ostringstream;

// Helper function to convert seconds into HH:MM:SS format
string Format::ElapsedTime(long seconds) { 
    ostringstream result;
    int hours = seconds/3600;
    int minutes = (seconds - hours*3600)/60; 
    int second = seconds%60;
    result << std::setw(2) << std::setfill('0') << hours << ":" 
              << std::setw(2) << std::setfill('0') << minutes << ":"
              << std::setw(2) << std::setfill('0') << second;
    return result.str(); 
}