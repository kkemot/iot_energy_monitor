#pragma once
#include <list>
using namespace std;

class simpleFilter {
private:
  list<float> List;
  int filter_size;
public:
  int setFilterSize(int val);
  void add(float value);
  void clear(void);
  float get(void);
  int getNumSamples(void);
};

