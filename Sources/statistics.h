
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_statistic_h
#define quantlib_statistic_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include <iostream>

QL_BEGIN_NAMESPACE(QuantLib)

class Statistics {
  public:
    Statistics();
    // Accessors
    int samples() const;
    double sampleWeight() const;
    double mean() const;
    // Variance, uses N-1 divisor
    double variance() const;
    // Standard deviation, uses N-1 divisor
    double standardDeviation() const;
    // Skew (Excel definition).  Zero for normal distribution.
    double skewness() const;
    // Kurtosis (Excel definition). Zero for normal distribution.
    double kurtosis() const;
    // Minimum sample value
    double min() const;
    // Maximum sample value
    double max() const;

    // Modifiers
    void add(double value, double weight = 1.0);
    template <class DataIterator>
    void addSequence(DataIterator begin, DataIterator end) {
      for (;begin!=end;begin++)
        add(*begin);
    }
    template <class DataIterator, class WeightIterator>
    void addSequence(DataIterator begin, DataIterator end, WeightIterator wbegin) {
      for (;begin!=end;begin++,wbegin++)
        add(*begin, *wbegin);
    }
    void reset();
  private:
    int theSampleNumber;
    double theSampleWeight;
    double theSum, theQuadraticSum, theCubicSum, theFourthPowerSum;
    double theMin, theMax;
};

// inline
inline void Statistics::add(double value, double weight) {
  theSampleNumber += 1;
  theSampleWeight += weight;
  double token, temp;
  token= temp = weight*value;
  theSum += temp;
  temp *= token;
  theQuadraticSum += temp;
  temp *= token;
  theCubicSum += temp;
  temp *= token;
  theFourthPowerSum += temp;
  theMin=QL_MIN(value, theMin);
  theMax=QL_MAX(value, theMax);
}

inline int Statistics::samples() const {
  return theSampleNumber;
}

inline double Statistics::sampleWeight() const {
  return theSampleWeight;
}

inline double Statistics::mean() const {
  Require(theSampleWeight>0.0, "Stat::mean() : theSampleWeight=0, unsufficient");

  return theSum/theSampleWeight;
}

inline double Statistics::variance() const {
  Require(theSampleNumber>1, "Stat::variance() : sample number <=1, unsufficient");
  double m = mean();

  return (theSampleWeight)/(theSampleNumber-1.0)*
    (theQuadraticSum/theSampleWeight -
    m*m);
}

inline double Statistics::standardDeviation() const {
  return QL_SQRT(variance());
}

inline double Statistics::skewness() const {
  Require(theSampleNumber>2, "Stat::skewness() : sample number <=2, unsufficient");
  double s = standardDeviation();
  Require(s>0.0, "Stat::skewness() : standard_dev=0.0, skew undefined");
  double m = mean();

  double coeff = (theSampleNumber*theSampleWeight/((theSampleNumber-1.0)*(theSampleNumber-2.0)*s*s*s));
  double a1 = theCubicSum/theSampleWeight;
  double a2 = -3.0*m*(theQuadraticSum/theSampleWeight);
  double a3 = 2.0*m*m*m;

  std::cout << coeff << ", " << a1 << ", " << a2 << ", " << a3 << ", ";
  return coeff*(a1+a2+a3);
}

inline double Statistics::kurtosis() const {
  Require(theSampleNumber>3, "Stat::kurtosis() : sample number <=3, unsufficient");
  double m = mean();
  double v = variance();
  Require(v>0.0, "Stat::kurtosis() : variance=0.0, kurtosis undefined");

  return theSampleNumber*theSampleWeight*(theSampleNumber+1.0)/
            ((theSampleNumber-1.0)*(theSampleNumber-2.0)*(theSampleNumber-3.0)*v*v)*
    (theFourthPowerSum/theSampleWeight-
    4.0*m*(theCubicSum/theSampleWeight)+
    6.0*m*m*(theQuadraticSum/theSampleWeight)+
    5.0*m*m*m*m)-
    (3.0*(theSampleNumber-1.0)*(theSampleNumber-1.0)/
    ((theSampleNumber-2.0)*(theSampleNumber-3.0)));
}

inline double Statistics::min() const {
  Require(theSampleNumber>=0, "Stat::min() : empty sample");
  return theMin;
}

inline double Statistics::max() const {
  Require(theSampleNumber>=0, "Stat::max() : empty sample");
  return theMax;
}

QL_END_NAMESPACE(QuantLib)

#endif
