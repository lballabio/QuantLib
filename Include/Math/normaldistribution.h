
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
#include <functional>

namespace QuantLib {

    namespace Math {

        class NormalDistribution : public std::unary_function<double,double> {
          public:
            NormalDistribution(double average = 0.0, double sigma = 1.0);
            // function
            double operator()(double x) const;
          private:
            static const double pi_;
            double average_, sigma_, normalizationFactor_, denominator_;
        };
        
        typedef NormalDistribution GaussianDistribution;
        
        class CumulativeNormalDistribution : 
          public std::unary_function<double,double> {
          public:
            CumulativeNormalDistribution();
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            static const double a1_, a2_, a3_, a4_, a5_, gamma_, precision_;
            NormalDistribution gaussian_;
        };
        
        class InverseCumulativeNormalDistribution : 
          public std::unary_function<double,double> {
          public:
            InverseCumulativeNormalDistribution() {}
            // function
            double operator()(double x) const;
          private:
            static const double p0_, p1_, p2_, q1_, q2_, q3_;
        };

        // inline definitions

        inline NormalDistribution::NormalDistribution(double average, 
          double sigma)
        : average_(average), sigma_(sigma) {
            normalizationFactor_ = 1.0/(sigma_*QL_SQRT(2.0*pi_));
            denominator_ = 2.0*sigma_*sigma_;
        }

        inline double NormalDistribution::operator()(double x) const {
            double deltax = x-average_;
            return normalizationFactor_*QL_EXP(-deltax*deltax/denominator_);
        }

        inline CumulativeNormalDistribution::CumulativeNormalDistribution() {}
        
        inline double CumulativeNormalDistribution::derivative(double x) const {
            return gaussian_(x);
        }

    }
    
}


#endif
