
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
            for (Size i = 0; i<sampleNumber_; i++) {
                result += samples_[i].second;
            }
            return result;
        }

        double HStatistics::mean() const {

            double result = 0.0, weightSum = 0.0;
            for (Size i = 0; i<sampleNumber_; i++) {
                result += samples_[i].second*samples_[i].first;
                weightSum += samples_[i].second;
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
            QL_REQUIRE(sampleNumber>1,
                       "HStatistics::variance() : "
                       "sample number <=1, unsufficient");

            double m = mean();
            double result = 0.0;
            for (Size i = 0; i<sampleNumber_; i++) {
                double temp = samples_[i].first;
                temp = temp*temp;
                result += samples_[i].second*temp;
            }
            result /= sampleWeight;
            result -= m*m;
            result *= sampleNumber_/(sampleNumber_-1.0);
            return result;
        }

        double HStatistics::downsideVariance() const{
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "HStatistics::downsideVariance() : "
                       "empty sample (zero weight sum)");

            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>1,
                       "HStatistics::downsideVariance() : "
                       "sample number <=1, unsufficient");

            double m = mean();
            double result = 0.0;
            for (Size i = 0; i<sampleNumber_; i++) {
                double temp = (samples_[i].first < 0.0 ? samples_[i].first : 0.0);
                temp = temp*temp;
                result += samples_[i].second * temp;
            }
            result /= sampleWeight;
            result *= sampleNumber_/(sampleNumber_-1.0);
            return result;
        }

        double HStatistics::skewness() const{
            QL_REQUIRE(sampleNumber_>2,
                       "HStatistics::skewness() : "
                       "sample number <=2, unsufficient");

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            for (Size i = 0; i<sampleNumber_; i++) {
                double temp = (samples_[i].first-m)/s;
                temp = temp*temp*temp;
                result += samples_[i].second*temp;
            }
            result *= sampleNumber_/weightSum();

            result *= sampleNumber_/(sampleNumber_-2.0);
            result /= (sampleNumber_-1.0);

            return result;
        }

        double HStatistics::kurtosis() const{
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>3,
                       "HStatistics::kurtosis() : "
                       "sample number <=3, unsufficient");

            double c = (sampleNumber_-1.0)/(sampleNumber_-2.0);
            c *= (sampleNumber_-1.0)/(sampleNumber_-3.0);
            c *= 3.0;

            double m = mean();
            double s = standardDeviation();
            double result = 0.0;
            for (Size i = 0; i<sampleNumber_; i++) {
                double temp = (samples_[i].first-m)/s;
                temp = temp*temp*temp*temp;
                result += samples_[i].second*temp;
            }
            result *= sampleNumber_/weightSum();

            result *= (sampleNumber_+1.0)/(sampleNumber_-3.0);
            result *= sampleNumber_/(sampleNumber_-2.0);
            result /= (sampleNumber_-1.0);

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

            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::const_iterator k =
                                            samples_.begin()-1;

            double hiIntegral = 0.0, perc=percentile*sampleWeight;
            while (hiIntegral < perc) {
                ++k;
                hiIntegral += k->second;
            }

            // just in case there are more samples at value k->first
            double lastAddedWeight = k->second;
            std::vector<std::pair<double,double> >::const_iterator kk = k+1;
            while (kk!= samples_.end() && kk->first==k->first) {
                lastAddedWeight += kk->second;
                hiIntegral      += kk->second;
            }

            bool interpolate = false;
            double result;
            // interpolating ... if possible and required
            if (k==samples_.begin() || (!interpolate))
                result = k->first;
            else {
                double lambda = (hiIntegral - perc) / (k->second);
                result = (1.0-lambda) * (k->first) + lambda * ((k-1)->first);
            }

           return result;
        }

        double HStatistics::potentialUpside(double y) const{
            QL_REQUIRE(y<1.0 && y>=0.9,
                "HStatistics::potentialUpside : percentile (" +
                DoubleFormatter::toString(y) +
                ") out of range 90%-100%");

            // potentialUpside must be a gain: this means that it has to be
            // MAX(dist(percentile), 0.0)
            return QL_MAX(percentile(y), 0.0);
        }

        double HStatistics::valueAtRisk(double y) const{
            QL_REQUIRE(y<1.0 && y>=0.9,
                "HStatistics::valueAtRisk : percentile (" +
                DoubleFormatter::toString(y) +
                ") out of range 90%-100%");

            // VAR must be a loss: this means that it has to be
            // MIN(dist(1.0-percentile), 0.0)
            // It must also be a positive quantity, so -MIN(*)
            return -QL_MIN(percentile(1.0-y), 0.0);
        }

        double HStatistics::expectedShortfall(double percentile) const{
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "HStatistics::expectedShortfall : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::const_iterator k =
                                            samples_.begin()-1;
            double lowIntegral = 1.0;
            double tailAverage = 0.0;
            while (lowIntegral > percentile) {
                ++k;
                lowIntegral -= k->second/sampleWeight_;
                tailAverage += k->first * k->second/sampleWeight_;
            }
            double lambda = (percentile - lowIntegral) / (k->second/sampleWeight_);
            tailAverage -= k->first * k->second/sampleWeight_ * lambda;
            tailAverage /= (1.0-percentile);

            // Expected Shortfall must be a loss
            // this means that it has to be MIN(tailAverage, 0.0)
            // Expected Shortfall must also be a positive quantity, so -MIN(*)
            return -QL_MIN(tailAverage, 0.0);
        }


        double HStatistics::shortfall(double target) const {
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber > 0,
                "HStatistics::shortfall() : "
                "empty sample");

            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "HStatistics::shortfall() : "
                       "empty sample (zero weight sum)");

            double undertarget = 0.0 ;
            for (Size k=0; k < sampleNumber ; ++k)
                if (samples_[k].first < target)
                    undertarget += samples_[k].second ;

            return undertarget/sampleWeight_;
        }

        double HStatistics::averageShortfall(double target) const {
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "HStatistics::averageShortfall() : "
                       "empty sample (zero weight sum)");

            double weightedUndertarget = 0.0;
            Size sampleNumber = samples_.size();
            for (Size k=0; k < sampleNumber ; ++k)
                if (samples_[k].first < target)
                    weightedUndertarget += samples_[k].second*
                                (target - samples_[k].first);

            return weightedUndertarget/sampleWeight_;

        }

    }

}
