
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file gaussianstatistics.hpp
    \brief statistics tool for gaussian-assumption risk measures
*/

// $Id$

#ifndef quantlib_gaussian_statistics_h
#define quantlib_gaussian_statistics_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace Math {

        //! Statistics tool for gaussian-assumption risk measures
        /*! It can calculate gaussian assumption risk measures (e.g.:
            value-at-risk, expected shortfall, etc.) based on the mean and
            variance provided by the template class

        */
        template<class Stat>
            class GaussianStatistics : public Stat {
          public:
            //! \name Gaussian risk measures
            //@{
            /*! gaussian-assumption y-th percentile, defined as the value x
                such that \f[ y = \frac{1}{\sqrt{2 \pi}}
                                      \int_{-\infty}^{x} \exp (-u^2/2) du \f]
            */
            double gaussianPercentile(double percentile) const;

            //! gaussian-assumption Potential-Upside at a given percentile
            double gaussianPotentialUpside(double percentile) const;

            //! gaussian-assumption Value-At-Risk at a given percentile
            double gaussianValueAtRisk(double percentile) const;

            //! gaussian-assumption Expected Shortfall at a given percentile
            double gaussianExpectedShortfall(double percentile) const;

            //! gaussian-assumption Shortfall (observations below target)
            double gaussianShortfall(double target) const;

            //! gaussian-assumption Average Shortfall (averaged shortfallness)
            double gaussianAverageShortfall(double target) const;
            //@}
        };


        /*! \pre percentile must be in range (0%-100%) extremes excluded */
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianPercentile(
            double percentile) const {

            QL_REQUIRE(percentile>0.0,
                       "GaussianStatistics::gaussianPercentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.0");
            QL_REQUIRE(percentile<1.0,
                       "GaussianStatistics::gaussianPercentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            Math::InverseCumulativeNormal gInverse(Stat::mean(),
                Stat::standardDeviation());
            return gInverse(percentile);
        }


        /*! \pre percentile must be in range [90%-100%) */
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianPotentialUpside(
            double percentile) const {

            QL_REQUIRE(percentile>=0.9,
                       "GaussianStatistics::gaussianPotentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "GaussianStatistics::gaussianPotentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result = gaussianPercentile(percentile);
            // PotenzialUpSide must be a gain
            // this means that it has to be MAX(dist(percentile), 0.0)
            return QL_MAX(result, 0.0);
        }


        /*! \pre percentile must be in range [90%-100%) */
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianValueAtRisk(
            double percentile) const {

            QL_REQUIRE(percentile>=0.9,
                       "GaussianStatistics::gaussianValueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "GaussianStatistics::gaussianValueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result = gaussianPercentile(1.0-percentile);
            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }


        /*! \pre percentile must be in range 90%-100% */
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianExpectedShortfall(
            double percentile) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "GaussianStatistics::expectedShortfall : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            double m = mean();
            double std = standardDeviation();
            Math::InverseCumulativeNormal gInverse(m, std);
            double var = gInverse(1.0-percentile);
            Math::NormalDistribution g(m, std);
            double result = m - std*std*g(var)/(1.0-percentile);
            // expectedShortfall must be a loss
            // this means that it has to be MIN(result, 0.0)
            // expectedShortfall must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }

        
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianShortfall(
            double target) const {
            Math::CumulativeNormalDistribution gIntegral(mean(),
                standardDeviation());
            return gIntegral(target);
        }

        
        template<class Stat>
        inline double GaussianStatistics<Stat>::gaussianAverageShortfall(
            double target) const {
            Math::CumulativeNormalDistribution gIntegral(mean(),
                standardDeviation());
            double m = mean();
            double std = standardDeviation();
            Math::NormalDistribution g(m, std);
            return ( (target-m)*gIntegral(target) + std*std*g(target) );
        }


    }

}


#endif
