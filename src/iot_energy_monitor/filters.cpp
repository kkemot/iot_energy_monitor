#include <list>
using namespace std;
#include "filters.h"

int simpleFilter::setFilterSize(int val) {
  filter_size = val;
}

int simpleFilter::getNumSamples(void) {
  return List.size();
}

void simpleFilter::add(float value) {
  List.push_back(value);
  if (List.size()> filter_size)
    List.pop_front();
}

void simpleFilter::clear(void) {
  List.clear();
}

float simpleFilter::get(void) {
  int listSize = List.size();

  if (listSize == 0) {
    return -1;
  }
  else if (List.size() == 1) {
    return List.back();
  }
  else if (listSize < filter_size) {
    float sum = 0;
    for (list<float>::iterator it = List.begin(); it != List.end(); it++)
        sum += *it;
      return sum/listSize;
  }
  else {
    int lowLimit = (30*filter_size)/100 ;
    int maxLimit = (70*filter_size)/100;
    float sum = 0;

    list<float> tempList(List);
    tempList.sort();

    //cut low limit
    for (int i = 0; i< lowLimit ; i++) {
      tempList.pop_front();
    }

    //cut max limit
    for (int i = maxLimit; i< filter_size; i++) {
        tempList.pop_back();
      }

      for (list<float>::iterator it = tempList.begin(); it != tempList.end(); it++)
        sum += *it;

      float res = sum/tempList.size();
      return res;
  }
}
