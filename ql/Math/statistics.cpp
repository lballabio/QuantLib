
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

/*! \file statistics.cpp
    \brief statistics tool with risk measures
*/

// $Id$

#include "ql/Math/statistics.hpp"

namespace QuantLib {

    namespace Math {

        double Statistics::weightSum() const {

            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                result += it->second;
            }
            return result;
        }

        double Statistics::mean() const {

            double result = 0.0, weightSum = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                result += it->second*it->first;
                weightSum += it->second;
            }
            QL_REQUIRE(weightSum>0.0,
                       "Statistics::mean() : "
                       "empty sample (zero weight sum)");
            result /= weightSum;
            return result;
        }

        double Statistics::variance() const {
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "Statistics::variance() : "
                       "empty sample (zero weight sum)");

            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>1.0,
                       "Statistics::variance() : "
                       "sample number <=1, unsufficient");

            double m = mean();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                double temp = it->first;
                temp = temp*temp;
                result += it->second*temp;
            }
            result /= sampleWeight;
            result -= m*m;
            result *= sampleNumber/(sampleNumber-1.0);
            return result;
        }

        double Statistics::downsideVariance() const {
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "Statistics::downsideVariance() : "
                       "empty sample (zero weight sum)");

            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>1.0,
                       "Statistics::downsideVariance() : "
                       "sample number <=1, unsufficient");

            double m = mean();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                double temp = it->first;
                // temp < 0.0 or temp<m ????
                result += ( temp<0.0 ? it->second*temp*temp : 0);
            }
            result /= sampleWeight;
//            result -= m*m;
            result *= sampleNumber/(sampleNumber-1.0);
            return result;
        }

        double Statistics::skewness() const{
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>2,
                       "Statistics::skewness() : "
                       "sample number <=2, unsufficient");

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                double temp = (it->first-m)/s;
                temp = temp*temp*temp;
                result += it->second*temp;
            }
            result *= sampleNumber/weightSum();

            result *= sampleNumber/(sampleNumber-2.0);
            result /= (sampleNumber-1.0);

            return result;
        }

        double Statistics::kurtosis() const{
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>3,
                       "Statistics::kurtosis() : "
                       "sample number <=3, unsufficient");

            double c = (sampleNumber-1.0)/(sampleNumber-2.0);
            c *= (sampleNumber-1.0)/(sampleNumber-3.0);
            c *= 3.0;

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
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

    }

}
