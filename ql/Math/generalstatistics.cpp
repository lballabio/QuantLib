
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

/*! \file generalstatistics.cpp
    \brief statistics tool with empirical-distribution risk measures
*/

// $Id$

#include "ql/Math/generalstatistics.hpp"

namespace QuantLib {

    namespace Math {

        double GeneralStatistics::weightSum() const {

            double result = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                result += it->second;
            }
            return result;
        }

        double GeneralStatistics::mean() const {

            double result = 0.0, weightSum = 0.0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                result += it->second*it->first;
                weightSum += it->second;
            }
            QL_REQUIRE(weightSum>0.0,
                       "GeneralStatistics::mean() : "
                       "empty sample (zero weight sum)");
            result /= weightSum;
            return result;
        }

        double GeneralStatistics::variance() const {
            double sampleWeight = weightSum();
            QL_REQUIRE(sampleWeight>0.0,
                       "GeneralStatistics::variance() : "
                       "empty sample (zero weight sum)");

            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>1.0,
                       "GeneralStatistics::variance() : "
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

        double GeneralStatistics::regret(double target) const {

            double result = 0.0, sampleWeight = 0.0;
            Size sampleNumber = 0;
            std::vector<std::pair<double,double> >::iterator it;
            for (it=samples_.begin(); it!=samples_.end(); it++) {
                double temp = it->first - target;
                if (temp<0.0) {
                    result       += it->second*temp*temp;
                    sampleWeight += it->second;
                    sampleNumber++;
                }
            }

            if (sampleWeight=0.0) {
                QL_REQUIRE(weightSum()>0.0,
                       "GeneralStatistics::regret() : "
                       "empty sample (zero weight sum)");
                return 0.0;
            }

            QL_REQUIRE(sampleNumber>1.0,
                       "GeneralStatistics::regret() : "
                       "sample under target <=1, unsufficient");

            result /= sampleWeight;
            result *= sampleNumber/(sampleNumber-1.0);
            return result;
        }

        double GeneralStatistics::skewness() const {
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>2,
                       "GeneralStatistics::skewness() : "
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

        double GeneralStatistics::kurtosis() const {
            Size sampleNumber = samples_.size();
            QL_REQUIRE(sampleNumber>3,
                       "GeneralStatistics::kurtosis() : "
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

        /*! \pre percent must be in range (0%-100%] */
        double GeneralStatistics::percentile(double percent) const {

            QL_REQUIRE(percent>0.0,
                       "GeneralStatistics::percentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percent) +
                       ") must be > 0.0");
            QL_REQUIRE(percent<=1.0,
                       "GeneralStatistics::percentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percent) +
                       ") must be <= 1.0");

            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::iterator k;
            double sampleWeight = 0;
            for (k=samples_.begin(); k!=samples_.end(); k++)
                sampleWeight += k->second;

            QL_REQUIRE(sampleWeight>0.0,
                       "GeneralStatistics::percentile() : "
                       "empty sample (zero weight sum)");


            double integral = 0.0, perc=percent*sampleWeight;
            k=samples_.begin()-1;
            do {
                k++;
                integral += k->second;
            } while (integral<perc && k!=samples_.end()-1);


            bool interpolate = false;
            // interpolating ... if possible and required
            if (k==samples_.begin() || (!interpolate))
                return k->first;
            else {
                // just in case there are more samples at value k->first
                double lastAddedWeight = k->second;
                std::vector<std::pair<double,double> >::iterator kk = k;
                kk++;
                while (kk!=samples_.end() && kk->first==k->first) {
                    lastAddedWeight += kk->second;
                    integral        += kk->second;
                    kk++;
                }
                double lambda = (integral - perc) / lastAddedWeight;
                return (1.0-lambda) * (k->first) + lambda * ((k-1)->first);
            }

        }

        /*! \pre percentile must be in range [90%-100%) */
        double GeneralStatistics::potentialUpside(double percentile) const {

            QL_REQUIRE(percentile>=0.9,
                       "GeneralStatistics::potentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "GeneralStatistics::potentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result=GeneralStatistics::percentile(percentile);

            // PotenzialUpSide must be a gain
            // this means that it has to be MAX(dist(percentile), 0.0)
            return QL_MAX(result, 0.0);
        }

        /*! \pre percentile must be in range [90%-100%) */
        double GeneralStatistics::valueAtRisk(double percentile) const {

            QL_REQUIRE(percentile>=0.9,
                       "GeneralStatistics::valueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "GeneralStatistics::valueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result=GeneralStatistics::percentile(1.0-percentile);

            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }

        /*! \pre percentile must be in range [90%-100%) */
        double GeneralStatistics::expectedShortfall(double percentile) const {

            QL_REQUIRE(percentile>=0.9,
                       "GeneralStatistics::expectedShortfall() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "GeneralStatistics::expectedShortfall() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::iterator k;
            double sampleWeight = 0;
            for (k=samples_.begin(); k!=samples_.end(); k++)
                sampleWeight += k->second;

            QL_REQUIRE(sampleWeight>0.0,
                       "GeneralStatistics::expectedShortfall() : "
                       "empty sample (zero weight sum)");


            double nextToLastResult, result = 0.0;
            double integral = 0.0, perc=(1.0-percentile)*sampleWeight;
            k=samples_.begin()-1;
            do {
                k++;
                nextToLastResult = result;
                integral += k->second;
                result   += k->second * k->first;
            } while (integral<perc && k!=samples_.end()-1);


            bool interpolate = false;
            // interpolating ... if possible and required
            if (k==samples_.begin() || (!interpolate)) {
                result /= integral;
            } else {
                // just in case there are more samples at value k->first
                double lastAddedWeight = k->second;
                std::vector<std::pair<double,double> >::iterator kk = k;
                kk++;
                while (kk!=samples_.end() && kk->first==k->first) {
                    lastAddedWeight += kk->second;
                    integral        += kk->second;
                    result          += kk->second * k->first;
                    kk++;
                }
                double lambda = (integral - perc) / lastAddedWeight;
                result = (1.0-lambda) * result + lambda * nextToLastResult;
                result /= perc;
            }


            // Expected Shortfall must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }


        double GeneralStatistics::shortfall(double target) const {

            std::vector<std::pair<double,double> >::iterator k;
            double sampleWeight = 0;
            for (k=samples_.begin(); k!=samples_.end(); k++)
                sampleWeight += k->second;

            QL_REQUIRE(sampleWeight>0.0,
                       "GeneralStatistics::shortfall() : "
                       "empty sample (zero weight sum)");

            double result = 0.0;
            for (k=samples_.begin(); k!=samples_.end(); k++)
                result += ( k->first<target ? k->second : 0.0);

            return result/sampleWeight;
        }


        double GeneralStatistics::averageShortfall(double target) const {

            std::vector<std::pair<double,double> >::iterator k;
            double sampleWeight = 0;
            for (k=samples_.begin(); k!=samples_.end(); k++)
                sampleWeight += k->second;

            QL_REQUIRE(sampleWeight>0.0,
                       "GeneralStatistics::averageShortfall() : "
                       "empty sample (zero weight sum)");

            double shortfallness, result = 0.0;
            for (k=samples_.begin(); k!=samples_.end(); k++) {
                shortfallness = target - k->first;
                result += (shortfallness>0 ? shortfallness * k->second : 0.0);
            }

            return result/sampleWeight;
        }


    }

}
