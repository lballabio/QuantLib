
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file normaldistribution.hpp
    \brief normal, cumulative and inverse cumulative distributions

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/05/09 11:06:18  nando
    A few comments modified/removed

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.9  2001/02/28 12:44:45  lballabio
    Fixed typo in error messages

    Revision 1.8  2001/01/16 11:22:17  nando
    fixed typo in error message

*/

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "ql/qldefines.hpp"
#include "ql/dataformatters.hpp"
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
                "NormalDistribution: sigma must be greater than 0.0 (" +
                    DoubleFormatter::toString(sigma_) + " not allowed)");

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
                "CumulativeNormalDistribution: "
                    "sigma must be greater than 0.0 (" +
                    DoubleFormatter::toString(sigma_) + " not allowed)");
        }

        inline double CumulativeNormalDistribution::derivative(double x) const {
            double xn = (x - average_) / sigma_;
            return gaussian_(xn) / sigma_;
        }

        inline InvCumulativeNormalDistribution::InvCumulativeNormalDistribution(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
              "InvCumulativeNormalDistribution: "
                "sigma must be greater than 0.0 (" +
                DoubleFormatter::toString(sigma_) + " not allowed)");
        }

    }

}


#endif
