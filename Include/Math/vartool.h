/*! \file vartool.h
    $Source$
    $Log$
    Revision 1.3  2001/03/05 10:42:19  nando
    Expected Shortfall added to classes HVarTool and HRiskStatistics.
    Expected Shortfall included in python test.

    Revision 1.2  2001/02/21 11:33:47  lballabio
    Inlined definition that was supposed to be

    Revision 1.1  2001/02/20 13:59:00  nando
    added class VarTool.
    RiskStatistics was derived from Statistics:
    now RiskStatistics includes VarTool and Statistics.
    VarTool is not based on Statistics, but requires
    mean and standardDeviation as input.


*/

#ifndef quantlib_vartool_h
#define quantlib_vartool_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include "normaldistribution.h"
#include <iostream>
#include <utility>
#include <vector>


namespace QuantLib {

    namespace Math {

        class VarTool {
          public:
            VarTool() {}
            double valueAtRisk(double percentile,
                               double mean,
                               double std) const ;
            double expectedShortfall(double percentile,
                                     double mean,
                                     double std) const ;
            double shortfall(double target,
                             double mean,
                             double std) const ;
            double averageShortfall(double target,
                                    double mean,
                                    double std) const ;
       };

        // inline definitions

        /*! \pre percentile must be in range 90%-100% */
        inline double VarTool::valueAtRisk(double percentile,
                                           double mean,
                                           double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "VarTool::valueAtRisk : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InvCumulativeNormalDistribution gInverse(mean, std);
            // VAR must be a loss
            // this means that it has to be MIN(dist(1.0-percentile), 0.0)
            // VAR must also be a positive quantity, so -MIN(*)
            return -QL_MIN(gInverse(1.0-percentile), 0.0);
        }

        /*! \pre percentile must be in range 90%-100% */
        inline double VarTool::expectedShortfall(double percentile,
                                                 double mean,
                                                 double std) const {
            QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                "VarTool::expectedShortfall : percentile (" +
                DoubleFormatter::toString(percentile) +
                ") out of range 90%-100%");

            Math::InvCumulativeNormalDistribution gInverse(mean, std);
            double var = gInverse(1.0-percentile);
            Math::NormalDistribution g(mean, std);
            double result = mean - std*std*g(var)/(1.0-percentile);
            // expectedShortfall must be a loss
            // this means that it has to be MIN(result, 0.0)
            // expectedShortfall must also be a positive quantity, so -MIN(*)
            return -QL_MIN(result, 0.0);
        }

        inline double VarTool::shortfall(double target,
                                         double mean,
                                         double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            return gIntegral(target);
        }

        inline double VarTool::averageShortfall(double target,
                                                double mean,
                                                double std) const {
            Math::CumulativeNormalDistribution gIntegral(mean, std);
            Math::NormalDistribution g(mean, std);
            return ( (target-mean)*gIntegral(target) + std*std*g(target) );
        }

    }

}


#endif
