
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file hstatistics.cpp
    \brief historical distribution statistics tool with risk measures
*/

// $Id$

#include "ql/Math/hstatistics.hpp"

namespace QuantLib {

    namespace Math {

        double HStatistics::weightSum() const {

            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it =
                samples_.begin();
            for (; it!=samples_.end(); it++) {
                result += it->second;
            }
            return result;
        }

        double HStatistics::mean() const {

            double result = 0.0, weightSum = 0.0;
            std::vector<std::pair<double,double> >::iterator it =
                samples_.begin();
            for (; it!=samples_.end(); it++) {
                result += it->second*it->first;
                weightSum += it->second;
            }
            QL_REQUIRE(weightSum>0.0,
                       "HStatistics::mean() : "
                       "empty sample (zero weight sum)");
            result /= weightSum;
            return result;
        }

        double HStatistics::variance() const {
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "HStatistics::variance() : "
                       "empty sample (zero weight sum)");

            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>1.0,
                       "HStatistics::variance() : "
                       "sample number <=1, unsufficient");

            double m = mean();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it =
                samples_.begin();
            for (; it!=samples_.end(); it++) {
                double temp = it->first;
                temp = temp*temp;
                result += it->second*temp;
            }
            result /= sampleWeight;
            result -= m*m;
            result *= sampleNumber/(sampleNumber-1.0);
            return result;
        }

        double HStatistics::skewness() const{
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>2,
                       "HStatistics::skewness() : "
                       "sample number <=2, unsufficient");

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it =
                samples_.begin();
            for (; it!=samples_.end(); it++) {
                double temp = (it->first-m)/s;
                temp = temp*temp*temp;
                result += it->second*temp;
            }
            result *= sampleNumber/weightSum();

            result *= sampleNumber/(sampleNumber-2.0);
            result /= (sampleNumber-1.0);

            return result;
        }

        double HStatistics::kurtosis() const{
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>3,
                       "HStatistics::kurtosis() : "
                       "sample number <=3, unsufficient");

            double c = (sampleNumber-1.0)/(sampleNumber-2.0);
            c *= (sampleNumber-1.0)/(sampleNumber-3.0);
            c *= 3.0;

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it =
                samples_.begin();
            for (; it!=samples_.end(); it++) {
                double temp = (it->first-m)/s;
                temp = temp*temp*temp*temp;
                result += it->second*temp;
            }
            result *= sampleNumber/weightSum();

            result *= (sampleNumber+1.0)/(sampleNumber-3.0);
            result *= sampleNumber/(sampleNumber-2.0);
            result /= (sampleNumber-1.0);

            return result-c;
        }

        double HStatistics::percentile(double percentile) const{
            QL_REQUIRE(percentile>0.0,
                       "HStatistics::percentile() : "
                       "percentile must be greater than zero");

            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "HStatistics::percentile() : "
                       "empty sample (zero weight sum)");

            // redundant if/when add() will add in order 
            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::iterator k =
                                            samples_.begin()-1;

            double integral = 0.0, perc=percentile*sampleWeight;
            while (integral < perc) {
                ++k;
                integral += k->second;
            }


            bool interpolate = false;
            // interpolating ... if possible and required
            if (k==samples_.begin() || (!interpolate))
                return k->first;
            else {
                // just in case there are more samples at value k->first
                double lastAddedWeight = k->second;
                std::vector<std::pair<double,double> >::iterator kk = k;
                kk++;
                while (kk!= samples_.end() && kk->first==k->first) {
                    lastAddedWeight += kk->second;
                    integral        += kk->second;
                    kk++;
                }
                double lambda = (integral - perc) / lastAddedWeight;
                return (1.0-lambda) * (k->first) + lambda * ((k-1)->first);
            }

        }

    }

}
