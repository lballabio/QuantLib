
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

/*! \file hstatisticss.hpp
    \brief historical distribution statistics tool with risk measures

    \fullpath
    ql/Math/%hstatisticss.hpp
*/

// $Id$

#include "ql/math/hstatistics.hpp"

namespace QuantLib {

    namespace Math {

        double HStatistics::percentile(double percentile) const{
            std::sort(samples_.begin(), samples_.end());

            std::vector<std::pair<double,double> >::const_iterator k =
                                            samples_.begin()-1;
            double lowIntegral = 1.0;
            while (lowIntegral > percentile) {
                ++k;
                lowIntegral -= k->second/sampleWeight_;
            }

            double result;
            // interpolating ... if possible
            if (k==samples_.begin())
                result = k->first;
            else {
                double lambda = (percentile - lowIntegral) /
                  (k->second/sampleWeight_);
                result = (1.0-lambda) * (k->first) + lambda * ((k-1)->first);
           }

           return result;
        }

        double HStatistics::potentialUpside(double percentile) const{
           // to be implemented
           return 0.0;
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
            QL_REQUIRE(sampleWeight_ > 0,
                "HStatistics::shortfall() : empty sample");

            double undertarget = 0.0 ;
            for (Size k=0; k < sampleNumber ; ++k)
                if (samples_[k].first < target)
                    undertarget += samples_[k].second ;

            return undertarget/sampleWeight_;
        }

        double HStatistics::averageShortfall(double target) const {
            QL_REQUIRE(sampleWeight_ > 0,
                "HStatistics::averageShortfall() : empty sample");

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
