
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

/*! \file riskmeasures.hpp
    \brief Risk functions
*/

// $Id$

#ifndef quantlib_risk_measures_h
#define quantlib_risk_measures_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/normaldistribution.hpp>


namespace QuantLib {

    namespace Math {

        //! \deprecated use Statistics instead
        class RiskMeasures {
          public:
            RiskMeasures() {}
            template<class DataIterator>
            double percentile(double percentile,
                              DataIterator begin,
                              DataIterator end) const ;
            double gaussianPercentile(double percentile,
                                      double mean,
                                      double std) const ;

            template<class DataIterator>
            double potentialUpside(double percentile,
                                   DataIterator begin,
                                   DataIterator end) const ;
            double gaussianPotentialUpside(double percentile,
                                           double mean,
                                           double std) const ;

            template<class DataIterator>
            double valueAtRisk(double percentile,
                               DataIterator begin,
                               DataIterator end) const ;
            double gaussianValueAtRisk(double percentile,
                                       double mean,
                                       double std) const ;

            double gaussianExpectedShortfall(double percentile,
                                             double mean,
                                             double std) const ;

            double gaussianShortfall(double target,
                                     double mean,
                                     double std) const ;

            double gaussianAverageShortfall(double target,
                                            double mean,
                                            double std) const ;
       };

        // inline definitions

        
        /*! \pre percentile must be in range (0%-100%] */
        template<class DataIterator>
        inline double RiskMeasures::percentile(double percentile,
            DataIterator begin, DataIterator end) const{

            QL_REQUIRE(percentile>0.0,
                       "RiskMeasures::percentile() : "
                       "percentile must be greater than zero");
            QL_REQUIRE(percentile<=1.0,
                       "RiskMeasures::percentile() : "
                       "percentile must be <=1.0");

            std::sort(begin, end);

            DataIterator k;
            double sampleWeight = 0;
            for (k=begin; k!=end; k++)
                sampleWeight += k->second;

            QL_REQUIRE(sampleWeight>0.0,
                       "RiskMeasures::percentile() : "
                       "empty sample (zero weight sum)");


            double integral = 0.0, perc=percentile*sampleWeight;
            k=begin--;
            do {
                k++;
                integral += k->second;
            } while (integral<perc && k!=end--);


            bool interpolate = false;
            // interpolating ... if possible and required
            if (k==begin || (!interpolate))
                return k->first;
            else {
                // just in case there are more samples at value k->first
                double lastAddedWeight = k->second;
                DataIterator kk = k;
                kk++;
                while (kk!=end && kk->first==k->first) {
                    lastAddedWeight += kk->second;
                    integral        += kk->second;
                    kk++;
                }
                double lambda = (integral - perc) / lastAddedWeight;
                return (1.0-lambda) * (k->first) + lambda * ((k-1)->first);
            }

        }


        /*! \pre percentile must be in range (0%-100%) extremes excluded */
        inline double RiskMeasures::gaussianPercentile(
            double percentile, double mean, double std) const {
            QL_REQUIRE(percentile>0.0,
                       "RiskMeasures::gaussianPercentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.0");
            QL_REQUIRE(percentile<1.0,
                       "RiskMeasures::gaussianPercentile() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            Math::InverseCumulativeNormal gInverse(mean, std);
            return gInverse(percentile);
        }



        
        /*! \pre y must be in range [90%-100%) */
        template<class DataIterator>
        inline double RiskMeasures::potentialUpside(double y,
            DataIterator begin, DataIterator end) const{

            QL_REQUIRE(percentile>=0.9,
                       "RiskMeasures::potentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "RiskMeasures::potentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result=percentile(y, begin, end);

            // PotenzialUpSide must be a gain
            // this means that it has to be MAX(dist(percentile), 0.0)
            return QL_MAX(result, 0.0);
        }

        /*! \pre percentile must be in range [90%-100%) */
        inline double RiskMeasures::gaussianPotentialUpside(
            double percentile, double mean, double std) const {

            QL_REQUIRE(percentile>0.9,
                       "RiskMeasures::gaussianPotentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.0");
            QL_REQUIRE(percentile<1.0,
                       "RiskMeasures::gaussianPotentialUpside() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            Math::InverseCumulativeNormal gInverse(mean, std);
            // PotenzialUpSide must be a gain
            // this means that it has to be MAX(dist(percentile), 0.0)
            return QL_MAX(gInverse(percentile), 0.0);
        }


        

        /*! \pre y must be in range [90%-100%) */
        template<class DataIterator>
        inline double RiskMeasures::valueAtRisk(double y,
            DataIterator begin, DataIterator end) const{

            QL_REQUIRE(percentile>=0.9,
                       "RiskMeasures::valueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "RiskMeasures::valueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            double result=percentile(1.0-y, begin, end);

            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }

        /*! \pre percentile must be in range [90%-100%) */
        inline double RiskMeasures::gaussianValueAtRisk(
            double percentile, double mean, double std) const {

            QL_REQUIRE(percentile>=0.9,
                       "RiskMeasures::gaussianValueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be >= 0.90");
            QL_REQUIRE(percentile<1.0,
                       "RiskMeasures::gaussianValueAtRisk() : "
                       "percentile (" +
                        DoubleFormatter::toString(percentile) +
                       ") must be < 1.0");

            Math::InverseCumulativeNormal gInverse(mean, std);
            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(gInverse(1.0-percentile), 0.0);
        }



        /*! \pre percentile must be in range 90%-100% */
        inline double RiskMeasures::gaussianExpectedShortfall(
            double percentile, double mean, double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "RiskMeasures::expectedShortfall : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InverseCumulativeNormal gInverse(mean, std);
            double var = gInverse(1.0-percentile);
            Math::NormalDistribution g(mean, std);
            double result = mean - std*std*g(var)/(1.0-percentile);
            // expectedShortfall must be a loss
            // this means that it has to be MIN(result, 0.0)
            // expectedShortfall must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }



        
        inline double RiskMeasures::gaussianShortfall(
            double target, double mean, double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            return gIntegral(target);
        }


        
        
        inline double RiskMeasures::gaussianAverageShortfall(
            double target, double mean, double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            Math::NormalDistribution g(mean, std);
            return ( (target-mean)*gIntegral(target) + std*std*g(target) );
        }

    }

}


#endif
