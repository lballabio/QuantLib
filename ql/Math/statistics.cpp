
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file statistics.cpp
    \brief statistics tools

    \fullpath
    ql/Math/%statistics.cpp
*/

// $Id$

#include <ql/Math/statistics.hpp>

namespace QuantLib {

    namespace Math {

        Statistics::Statistics() {
            reset();
        }

        void Statistics::reset() {
            min_ = QL_MAX_DOUBLE;
            max_ = QL_MIN_DOUBLE;
            sampleNumber_ = 0;
            sampleWeight_ = 0.0;
            sum_ = 0.0;
            quadraticSum_ = 0.0;
            downsideQuadraticSum_ = 0.0;
            cubicSum_ = 0.0;
            fourthPowerSum_ = 0.0;
        }

        /*! \pre weights must be positive or null */
        void Statistics::add(double value, double weight) {
            QL_REQUIRE(weight>=0.0,
                "Statistics::add : negative weight (" +
                DoubleFormatter::toString(weight) + ") not allowed");

            Size oldSamples = sampleNumber_;
            sampleNumber_++;
            QL_ENSURE(sampleNumber_ > oldSamples,
                      "Statistics::add : maximum number of samples reached");

            sampleWeight_ += weight;

            double temp = weight*value;
            sum_ += temp;
            temp *= value;
            quadraticSum_ += temp;
            downsideQuadraticSum_ += value < 0.0 ? temp : 0.0;
            temp *= value;
            cubicSum_ += temp;
            temp *= value;
            fourthPowerSum_ += temp;
            min_=QL_MIN(value, min_);
            max_=QL_MAX(value, max_);
        }


        double Statistics::variance() const {
            QL_REQUIRE(sampleWeight_>0.0,
                       "Stat::variance() : sampleWeight_=0, unsufficient");
            QL_REQUIRE(sampleNumber_>1,
                       "Stat::variance() : sample number <=1, unsufficient");

            double m = mean();
            double v = quadraticSum_/sampleWeight_;
            v -= m*m;
            v *= sampleNumber_/(sampleNumber_-1.0);

//            if (QL_FABS(v) <= 1.0e-6)
//                v = 0.0;

            QL_ENSURE(v >= 0.0,
                      "Statistics: negative variance (" +
                      DoubleFormatter::toString(v,20) + ")");

            return v;
        }

        double Statistics::downsideVariance() const {
            QL_REQUIRE(sampleWeight_>0.0,
                       "Stat::variance() : sampleWeight_=0, unsufficient");
            QL_REQUIRE(sampleNumber_>1,
                       "Stat::variance() : sample number <=1, unsufficient");

            return (sampleNumber_/(sampleNumber_-1.0))*
                (downsideQuadraticSum_ /sampleWeight_);
        }

        double Statistics::skewness() const {
            QL_REQUIRE(sampleNumber_>2,
                       "Stat::skewness() : sample number <=2, unsufficient");
            double s = standardDeviation();

            if (s==0.0) return 0.0;

            double m = mean();
            double result = cubicSum_/sampleWeight_;
            result -= 3.0*m*(quadraticSum_/sampleWeight_);
            result += 2.0*m*m*m;
            result /= s*s*s;
            result *= sampleNumber_/(sampleNumber_-1.0);
            result *= sampleNumber_/(sampleNumber_-2.0);
            return result;
        }


        double Statistics::kurtosis() const {
            QL_REQUIRE(sampleNumber_>3,
                       "Stat::kurtosis() : sample number <=3, unsufficient");

            double m = mean();
            double v = variance();

            double c = (sampleNumber_-1.0)/(sampleNumber_-2.0);
            c *= (sampleNumber_-1.0)/(sampleNumber_-3.0);
            c *= 3.0;

            if (v==0) return c;

            double result = fourthPowerSum_/sampleWeight_;
            result -= 4.0*m*(cubicSum_/sampleWeight_);
            result += 6.0*m*m*(quadraticSum_/sampleWeight_);
            result -= 3.0*m*m*m*m;
            result /= v*v;
            result *= sampleNumber_/(sampleNumber_-1.0);
            result *= sampleNumber_/(sampleNumber_-2.0);
            result *= (sampleNumber_+1.0)/(sampleNumber_-3.0);


            return result-c;
        }

    }

}
