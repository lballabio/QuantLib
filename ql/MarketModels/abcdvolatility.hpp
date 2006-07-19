/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2005, 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_abcd_volatility_hpp
#define quantlib_abcd_volatility_hpp

#include <ql/MarketModels/pseudoroot.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    //! Abcd functional form for instantaneous volatility
    /*! 
        \f[
        f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d 
        \f]

        following Rebonato notation.

        The class instantiates:
        \f[
        f(T-t) = [ ( a + b(T-t) ) e^{-c(T-t)} + d ]  [ ( a + b(S-t) ) e^{-c(S-t)} + d ]
        \f]


    */
    class Abcd : public std::unary_function<Real,Real> {
    public:
        Abcd(Real a, Real b, Real c, Real d, Real T, Real S) 
        : a_(a), b_(b), c_(c), d_(d), T_(T), S_(S)  { }
        Real operator()(Time u) const {
            return  ( (a_ + b_*(T_-u))*std::exp(-c_*(T_-u)) + d_ ) *
                    ( (a_ + b_*(S_-u))*std::exp(-c_*(S_-u)) + d_ );
        }

        Real primitive(Time u) const {
            const Real k1=std::exp(c_*u);
            const Real k2=std::exp(c_*S_);
            const Real k3=std::exp(c_*T_);

            return (b_*b_*(-1 - 2*c_*c_*S_*T_ - c_*(S_ + T_)
                         + k1*k1*(1 + c_*(S_ + T_ - 2*u) + 2*c_*c_*(S_ - u)*(T_ - u)))
                    + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                             +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*u)
                    + 2*b_*c_*(a_*(-1 - c_*(S_ + T_) + k1*k1*(1 + c_*(S_ + T_ - 2*u)))
                             -2*d_*(k3*(1 + c_*S_) + k2*(1 + c_*T_)
                                   - k1*k3*(1 + c_*(S_ - u))
                                   - k1*k2*(1 + c_*(T_ - u)))
                             )
                    ) / (4*c_*c_*c_*k2*k3);}

    private:
        Real a_, b_, c_, d_;
        Time S_, T_;
    };

    class AbcdVolatility : public PseudoRoot {
    public:
        AbcdVolatility(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements);
        const Array& initialRates() const ;// passed by the calibrator
        const Array& displacements() const ;// passed by the calibrator
        Size numberOfRates() const ;// n 
        Size numberOfFactors() const ;//F, A rank 
        // number of steps method?
        const Matrix& pseudoRoot(Size i) const ;

    private:
        Real a_, b_, c_, d_;
        std::vector<Real> ks_;
        Real longTermCorr_;
        Real beta_;
        Array rateTimes_, evolutionTimes_;
        Size numberOfFactors_;
        Array initialRates_;
        Array displacements_;
        std::vector<Matrix> pseudoRoots_;

    };
}

#endif
