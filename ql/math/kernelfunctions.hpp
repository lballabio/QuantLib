/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Dimitri Reiswich

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file kernelfunctions.hpp
    \brief Kernel functions
*/

#ifndef quantlib_kernel_functions_hpp
#define quantlib_kernel_functions_hpp

#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    /*! Kernel function in the statistical sense, e.g. a nonnegative,
        real-valued function which integrates to one and is symmetric.

        Derived classes will serve as functors.
    */
    class KernelFunction {
      public:
        virtual ~KernelFunction() {}
        virtual Real operator()(Real x) const = 0;
    };


    //! Gaussian kernel function
    class GaussianKernel : public KernelFunction {
      public:
        GaussianKernel(Real average, Real sigma)
        : nd_(average,sigma), cnd_(average,sigma) {
            // normFact is \sqrt{2*\pi}.
            normFact_ = M_SQRT2*M_SQRTPI;
        }

        Real operator()(Real x) const{
            return nd_(x)*normFact_;
        }

        Real derivative(Real x) const{
            return nd_.derivative(x)*normFact_;
        }

        Real primitive(Real x) const{
            return cnd_(x)*normFact_;
        }

      private:
        NormalDistribution nd_;
        CumulativeNormalDistribution cnd_;
        Real normFact_;
    };

}


#endif
