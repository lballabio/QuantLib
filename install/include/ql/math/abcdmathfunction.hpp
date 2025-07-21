/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
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

#ifndef quantlib_abcd_math_function_hpp
#define quantlib_abcd_math_function_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    //! %Abcd functional form
    /*! \f[ f(t) = [ a + b*t ] e^{-c*t} + d \f]
        following Rebonato's notation. */
    class AbcdMathFunction {

      public:
        AbcdMathFunction(Real a = 0.002,
                         Real b = 0.001, 
                         Real c = 0.16,
                         Real d = 0.0005);
        AbcdMathFunction(std::vector<Real> abcd);

        //! function value at time t: \f[ f(t) \f]
        Real operator()(Time t) const;

        //! time at which the function reaches maximum (if any)
        Time maximumLocation() const;

        //! maximum value of the function
        Real maximumValue() const;

        //! function value at time +inf: \f[ f(\inf) \f]
        Real longTermValue() const { return d_; }

        /*! first derivative of the function at time t
            \f[ f'(t) = [ (b-c*a) + (-c*b)*t) ] e^{-c*t} \f] */
        Real derivative(Time t) const;
        
        /*! indefinite integral of the function at time t
            \f[ \int f(t)dt = [ (-a/c-b/c^2) + (-b/c)*t ] e^{-c*t} + d*t \f] */
        Real primitive(Time t) const;
        
        /*! definite integral of the function between t1 and t2
            \f[ \int_{t1}^{t2} f(t)dt \f] */
        Real definiteIntegral(Time t1, Time t2) const;

        /*! Inspectors */
        Real a() const { return a_; }
        Real b() const { return b_; }
        Real c() const { return c_; }
        Real d() const { return d_; }
        const std::vector<Real>& coefficients() { return abcd_; }
        const std::vector<Real>& derivativeCoefficients() { return dabcd_; }
        // the primitive is not abcd

        /*! coefficients of a AbcdMathFunction defined as definite
            integral on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteIntegralCoefficients(Time t,
                                                       Time t2) const;

        /*! coefficients of a AbcdMathFunction defined as definite
            derivative on a rolling window of length tau, with tau = t2-t */
        std::vector<Real> definiteDerivativeCoefficients(Time t,
                                                         Time t2) const;

        static void validate(Real a,
                             Real b,
                             Real c,
                             Real d);
      protected:
        Real a_, b_, c_, d_;
      private:
        void initialize_();
        std::vector<Real> abcd_;
        std::vector<Real> dabcd_;
        Real da_, db_;
        Real pa_, pb_, K_;

        Real dibc_, diacplusbcc_;
    };

    // inline AbcdMathFunction
    inline Real AbcdMathFunction::operator()(Time t) const {
        //return (a_ + b_*t)*std::exp(-c_*t) + d_;
        return t<0 ? 0.0 : Real((a_ + b_*t)*std::exp(-c_*t) + d_);
    }

    inline Real AbcdMathFunction::derivative(Time t) const {
        //return (da_ + db_*t)*std::exp(-c_*t);
        return t<0 ? 0.0 : Real((da_ + db_*t)*std::exp(-c_*t));
    }

    inline Real AbcdMathFunction::primitive(Time t) const {
        //return (pa_ + pb_*t)*std::exp(-c_*t) + d_*t + K_;
        return t<0 ? 0.0 : Real((pa_ + pb_*t)*std::exp(-c_*t) + d_*t + K_);
    }

    inline Real AbcdMathFunction::maximumValue() const {
        if (b_==0.0 || a_<=0.0)
            return d_;
        return (*this)(maximumLocation());
    }

}

#endif
