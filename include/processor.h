#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
    float prevIdle_{0}, prevTotal_{0};
};

#endif