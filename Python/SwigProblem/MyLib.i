
%module MyLib

%{
#include "helloworld.h"
#include "normaldistribution.h"
using QuantLib::Math::NormalDistribution;
%}


void HelloWorld();


class NormalDistribution {
  public:
    NormalDistribution(double average = 0.0, double sigma = 1.0);
    ~NormalDistribution();
};

