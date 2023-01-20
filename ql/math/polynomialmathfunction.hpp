/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

#ifndef quantlib_polynomial_math_function_hpp
#define quantlib_polynomial_math_function_hpp

#include <ql/math/matrix.hpp>

#include <vector>

namespace QuantLib {
    
    //! %Cubic functional form
    /*! \f[ f(t) = \sum_{i=0}^n{c_i t^i} \f] */
    class PolynomialFunction {

      public:
        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Time argument_type;

        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Real result_type;

        PolynomialFunction(const std::vector<Real>& coeff);

        //! function value at time t: \f[ f(t) = \sum_{i=0}^n{c_i t^i} \f]
        Real operator()(Time t) const;

        /*! first derivative of the function at time t
            \f[ f'(t) = \sum_{i=0}^{n-1}{(i+1) c_{i+1} t^i} \f] */
        Real derivative(Time t) const;

        /*! indefinite integral of the function at time t
            \f[ \int f(t)dt = \sum_{i=0}^n{c_i t^{i+1} / (i+1)} + K \f] */
        Real primitive(Time t) const;

        /*! definite integral of the function between t1 and t2
            \f[ \int_{t1}^{t2} f(t)dt \f] */
        Real definiteIntegral(Time t1,
                              Time t2) const;

        /*! Inspectors */
        Size order() const { return order_; }
        const std::vector<Real>& coefficients() { return c_; }
        const std::vector<Real>& derivativeCoefficients() { return derC_; }
        const std::vector<Real>& primitiveCoefficients() { return prC_; }

        /*! coefficients of a PolynomialFunction defined as definite
            integral on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteIntegralCoefficients(Time t,
                                                       Time t2) const;

        /*! coefficients of a PolynomialFunction defined as definite
            derivative on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteDerivativeCoefficients(Time t,
                                                         Time t2) const;

      private:
        Size order_;
        std::vector<Real> c_, derC_, prC_;
        Real K_;
        mutable Matrix eqs_;
        void initializeEqs_(Time t,
                            Time t2) const;
    };

}

#endif
