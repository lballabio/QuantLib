/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file numericaldifferentiation.hpp
    \brief numerical differentiation of arbitrary order
           and on irregular grids
*/

#ifndef quantlib_numerical_differentiation_hpp
#define quantlib_numerical_differentiation_hpp

#include <ql/math/array.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    //! Numerical Differentiation on arbitrarily spaced grids

    /*! References:

        B. Fornberg, 1988. Generation of Finite Difference Formulas
        on Arbitrarily Spaced Grids,
        http://amath.colorado.edu/faculty/fornberg/Docs/MathComp_88_FD_formulas.pdf
    */
    class NumericalDifferentiation {
      public:
        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Real argument_type;

        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Real result_type;

        enum Scheme { Central, Backward, Forward };

        NumericalDifferentiation(ext::function<Real(Real)> f,
                                 Size orderOfDerivative,
                                 Array x_offsets);

        NumericalDifferentiation(ext::function<Real(Real)> f,
                                 Size orderOfDerivative,
                                 Real stepSize,
                                 Size steps,
                                 Scheme scheme);

        Real operator()(Real x) const;
        const Array& offsets() const;
        const Array& weights() const;

      private:
        const Array offsets_, w_;
        const ext::function<Real(Real)> f_;
    };


    inline Real NumericalDifferentiation::operator()(Real x) const {
        Real s = 0.0;
        for (Size i=0; i < w_.size(); ++i) {
            if (std::fabs(w_[i]) > QL_EPSILON*QL_EPSILON) {
                s += w_[i] * f_(x+offsets_[i]);
            }
        }
        return s;
    }

    inline const Array& NumericalDifferentiation::weights() const {
        return w_;
    }

    inline const Array& NumericalDifferentiation::offsets() const {
        return offsets_;
    }
}


#endif

