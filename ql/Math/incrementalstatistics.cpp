
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Math/incrementalstatistics.hpp>

namespace QuantLib {

    IncrementalStatistics::IncrementalStatistics() {
        reset();
    }

    void IncrementalStatistics::reset() {
        min_ = QL_MAX_DOUBLE;
        max_ = QL_MIN_DOUBLE;
        sampleNumber_ = 0;
        downsideSampleNumber_ = 0;
        sampleWeight_ = 0.0;
        downsideSampleWeight_ = 0.0;
        sum_ = 0.0;
        quadraticSum_ = 0.0;
        downsideQuadraticSum_ = 0.0;
        cubicSum_ = 0.0;
        fourthPowerSum_ = 0.0;
    }

    void IncrementalStatistics::add(double value, double weight) {
        QL_REQUIRE(weight>=0.0,
                   "IncrementalStatistics::add : "
                   "negative weight (" +
                   DoubleFormatter::toString(weight) + ") not allowed");

        Size oldSamples = sampleNumber_;
        sampleNumber_++;
        QL_ENSURE(sampleNumber_ > oldSamples,
                  "IncrementalStatistics::add : "
                  "maximum number of samples reached");

        sampleWeight_ += weight;

        double temp = weight*value;
        sum_ += temp;
        temp *= value;
        quadraticSum_ += temp;
        if (value<0.0) {
            downsideQuadraticSum_ += temp;
            downsideSampleNumber_++;
            downsideSampleWeight_ += weight;
        }
        temp *= value;
        cubicSum_ += temp;
        temp *= value;
        fourthPowerSum_ += temp;
        if (oldSamples == 0) {
            min_ = max_ = value;
        } else {
            min_=QL_MIN(value, min_);
            max_=QL_MAX(value, max_);
        }
    }


    double IncrementalStatistics::variance() const {
        QL_REQUIRE(sampleWeight_>0.0,
                   "IncrementalStatistics::variance() : "
                   "sampleWeight_=0, unsufficient");
        QL_REQUIRE(sampleNumber_>1,
                   "IncrementalStatistics::variance() : "
                   "sample number <=1, unsufficient");

        double m = mean();
        double v = quadraticSum_/sampleWeight_;
        v -= m*m;
        v *= sampleNumber_/(sampleNumber_-1.0);


        QL_ENSURE(v >= 0.0,
                  "IncrementalStatistics::variance :"
                  "negative variance (" +
                  DoubleFormatter::toString(v,20) + ")");

        return v;
    }

    double IncrementalStatistics::downsideVariance() const {
        if (downsideSampleWeight_==0.0) {
            QL_REQUIRE(sampleWeight_>0.0,
                       "IncrementalStatistics::downsideVariance() : "
                       "sampleWeight_=0, unsufficient");
            return 0.0;
        }

        QL_REQUIRE(downsideSampleNumber_>1,
                   "IncrementalStatistics::downsideVariance() : "
                   "sample number below zero <=1, unsufficient");

        return (downsideSampleNumber_/(downsideSampleNumber_-1.0))*
            (downsideQuadraticSum_ /downsideSampleWeight_);
    }

    double IncrementalStatistics::skewness() const {
        QL_REQUIRE(sampleNumber_>2,
                   "IncrementalStatistics::skewness() : "
                   "sample number <=2, unsufficient");
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


    double IncrementalStatistics::kurtosis() const {
        QL_REQUIRE(sampleNumber_>3,
                   "IncrementalStatistics::kurtosis() : "
                   "sample number <=3, unsufficient");

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
