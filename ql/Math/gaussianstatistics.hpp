
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

/*! \file gaussianstatistics.hpp
    \brief statistics tool for gaussian-assumption risk measures
*/

#ifndef quantlib_gaussian_statistics_h
#define quantlib_gaussian_statistics_h

#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    //! Statistics tool for gaussian-assumption risk measures
    /*! It can calculate gaussian assumption risk measures (e.g.:
        value-at-risk, expected shortfall, etc.) based on the mean and
        variance provided by the template class
    */
    template<class Stat>
    class GaussianStatistics : public Stat {
      public:
        GaussianStatistics() {}
        GaussianStatistics(const Stat& s) : Stat(s) {}
        //! \name Gaussian risk measures
        //@{
        /*! returns the downside variance, defined as
            \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
            where \f$ \theta \f$ = 0 if x > 0 and
            \f$ \theta \f$ =1 if x <0
        */
        double gaussianDownsideVariance() const {
            return gaussianRegret(0.0);
        }

        /*! returns the downside deviation, defined as the
            square root of the downside variance.
        */
        double gaussianDownsideDeviation() const {
            return QL_SQRT(gaussianDownsideVariance());
        }

        /*! returns the variance of observations below target 
            \f[ \frac{\sum w_i (min(0, x_i-target))^2 }{\sum w_i}. \f]

            See Dembo, Freeman "The Rules Of Risk", Wiley (2001)
        */
        double gaussianRegret(double target) const;


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


    //! Helper class for precomputed distributions
    class StatsHolder {
    public:
        StatsHolder(double mean,
                    double standardDeviation)
                    : mean_(mean), standardDeviation_(standardDeviation) {}
        ~StatsHolder() {}
        double mean() const { return mean_; }
        double standardDeviation() const { return standardDeviation_; }
    private:
        double mean_, standardDeviation_;
    };


    // inline definitions

    template<class Stat>
    inline
    double GaussianStatistics<Stat>::gaussianRegret(double target) const {
        double m = mean();
        double std = standardDeviation();
        double variance = std*std;
        CumulativeNormalDistribution gIntegral(m, std);
        NormalDistribution g(m, std);
        double firstTerm = variance + m*m - 2.0*target*m + target*target;
        double alfa = gIntegral(target);
        double secondTerm = m - target;
        double beta = variance*g(target);
        double result = alfa*firstTerm - beta*secondTerm;
        return result/alfa;
    }

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

        InverseCumulativeNormal gInverse(Stat::mean(),
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
        InverseCumulativeNormal gInverse(m, std);
        double var = gInverse(1.0-percentile);
        NormalDistribution g(m, std);
        double result = m - std*std*g(var)/(1.0-percentile);
        // expectedShortfall must be a loss
        // this means that it has to be MIN(result, 0.0)
        // expectedShortfall must also be a positive quantity, so -MIN(*)
        return -QL_MIN(result, 0.0);
    }


    template<class Stat>
    inline double GaussianStatistics<Stat>::gaussianShortfall(
                                                        double target) const {
        CumulativeNormalDistribution gIntegral(mean(),
                                               standardDeviation());
        return gIntegral(target);
    }


    template<class Stat>
    inline double GaussianStatistics<Stat>::gaussianAverageShortfall(
                                                        double target) const {
        double m = mean();
        double std = standardDeviation();
        CumulativeNormalDistribution gIntegral(m, std);
        NormalDistribution g(m, std);
        return ( (target-m) + std*std*g(target)/gIntegral(target) );
    }

}


#endif
