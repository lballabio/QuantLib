
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file normaldistribution.hpp
    \brief normal, cumulative and inverse cumulative distributions

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.8  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "ql/dataformatters.hpp"
#include <functional>

namespace QuantLib {

    namespace Math {

        class NormalDistribution 
        : public std::unary_function<double,double> {
          public:
            NormalDistribution(double average = 0.0,
                               double sigma = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            static const double pi_;
            double average_, sigma_, normalizationFactor_, denominator_;
        };

        typedef NormalDistribution GaussianDistribution;


        class CumulativeNormalDistribution 
        : public std::unary_function<double,double> {
          public:
            CumulativeNormalDistribution(double average = 0.0,
                                         double sigma   = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            double average_, sigma_;
            static const double a1_;
            static const double a2_;
            static const double a3_;
            static const double a4_;
            static const double a5_;
            static const double gamma_;
            static const double precision_;
            NormalDistribution gaussian_;
        };


        class InvCumulativeNormalDistribution 
        : public std::unary_function<double,double> {
          public:
            InvCumulativeNormalDistribution(double average = 0.0,
                                            double sigma   = 1.0);
            // function
            double operator()(double x) const;
          private:
            double average_, sigma_;
            static const double p0_;
            static const double p1_;
            static const double p2_;
            static const double q1_;
            static const double q2_;
            static const double q3_;
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
