
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file normaldistribution.h
    \brief normal, cumulative and inverse cumulative distributions

    $Source$
    $Name$
    $Log$
    Revision 1.8  2001/01/16 11:22:17  nando
    fixed typo in error message

    Revision 1.7  2001/01/11 18:06:09  nando
    generalized to sigma<>1 and average<>0
    Also added NormalDistribution.derivative().
    Improved Python test now also uses Finite Difference
    first and second order operators

    Revision 1.6  2001/01/08 16:22:32  nando
    added InverseCumulativeNormalDistribution

    Revision 1.5  2001/01/04 11:59:29  lballabio
    Line wraps fixed when stumbling upon them

    Revision 1.4  2000/12/27 17:18:35  lballabio
    Changes for compiling under Linux and Alpha Linux

    Revision 1.3  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "qldefines.h"
#include "dataformatters.h"
#include <functional>

namespace QuantLib {

    namespace Math {

        class NormalDistribution : public std::unary_function<double,double> {
          public:
            NormalDistribution(double average = 0.0, double sigma = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            static const double pi_;
            double average_, sigma_, normalizationFactor_, denominator_;
        };

        typedef NormalDistribution GaussianDistribution;


        class CumulativeNormalDistribution :
          public std::unary_function<double,double> {
          public:
            CumulativeNormalDistribution(double average = 0.0,
                                         double sigma   = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            double average_, sigma_;
            static const double a1_, a2_, a3_, a4_, a5_, gamma_, precision_;
            NormalDistribution gaussian_;
        };


        class InvCumulativeNormalDistribution :
          public std::unary_function<double,double> {
          public:
            InvCumulativeNormalDistribution(double average = 0.0,
                                                double sigma   = 1.0);
            // function
            double operator()(double x) const;
          private:
            double average_, sigma_;
            static const double p0_, p1_, p2_, q1_, q2_, q3_;
        };


        // inline definitions

        inline NormalDistribution::NormalDistribution(double average,
            double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
                "NormalDistribution::NormalDistribution"
                ": sigma must be greater than 0.0 ("
                + DoubleFormatter::toString(sigma_) + " not allowed");

            normalizationFactor_ = 1.0/(sigma_*QL_SQRT(2.0*pi_));
            denominator_ = 2.0*sigma_*sigma_;
        }

        inline double NormalDistribution::operator()(double x) const {
            double deltax = x-average_;
            return normalizationFactor_*QL_EXP(-deltax*deltax/denominator_);
        }

        inline double NormalDistribution::derivative(double x) const {
            return (*this)(x) * (average_ - x) / sigma_;
        }

        inline CumulativeNormalDistribution::CumulativeNormalDistribution(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
                "CumulativeNormalDistribution::CumulativeNormalDistribution"
                ": sigma must be greater than 0.0 ("
                + DoubleFormatter::toString(sigma_) + " not allowed");
        }

        inline double CumulativeNormalDistribution::derivative(double x) const {
            double xn = (x - average_) / sigma_;
            return gaussian_(xn) / sigma_;
        }

        inline InvCumulativeNormalDistribution::InvCumulativeNormalDistribution(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
              "InvCumulativeNormalDistribution::InvCumulativeNormalDistribution"
                ": sigma must be greater than 0.0 ("
                + DoubleFormatter::toString(sigma_) + " not allowed");
        }

    }

}


#endif
