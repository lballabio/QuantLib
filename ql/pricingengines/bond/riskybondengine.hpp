/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2021 Lew Wei Hao

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

/*! \file riskybondengine.hpp
    \brief risky bond engine
*/

#ifndef quantlib_risky_bond_engine_hpp
#define quantlib_risky_bond_engine_hpp

#include <ql/instruments/bond.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    //! Risky pricing engine for bonds
    /*! The value of each cashflow is contingent to survival, i.e., the knockout
        probability is considered.

        In each of the \f$n\f$ coupon periods, we can calculate the value
        in the case of survival and default, assuming that the issuer
        can only default in the middle of a coupon period. We denote this time
        \f$T_{i}^{mid}=\frac{T_{i-1}+T_{i}}{2}\f$.

        Given survival we receive the full cash flow (both coupons and notional).
        The time \f$t\f$ value of these payments are given by
        \f[
            \sum_{i=1}^{n}CF_{i}P(t,T_{i})Q(T_{i}<\tau)
        \f]
        where \f$P(t,T)\f$ is the time \f$T\f$ discount bond
        and \f$Q(T<\tau)\f$ is the time \f$T\f$ survival probability.
        \f$n\f$ is the number of coupon periods. This takes care of the payments
        in the case of survival.

        Given default we receive only a fraction of the notional at default.
        \f[
            \sum_{i=1}^{n}Rec N(T_{i}^{mid}) P(t,T_{i}^{mid})Q(T_{i-1}<\tau\leq T_{i})
        \f]
        where \f$Rec\f$ is the recovery rate and \f$N(T)\f$ is the time T notional. The default probability can be
        rewritten as
        \f[
            Q(T_{i-1}<\tau\leq T_{i})=Q(T_{i}<\tau)-Q(T_{i-1}<\tau)=(1-Q(T_{i}\geq\tau))-(1-Q(T_{i-1}\geq\tau))=Q(T_{i-1}\geq\tau)-Q(T_{i}\geq\tau)
        \f]
    */
    class RiskyBondEngine : public Bond::engine {
      public:
        RiskyBondEngine(Handle<DefaultProbabilityTermStructure> defaultTS,
                        Real recoveryRate,
                        Handle<YieldTermStructure> yieldTS);
        void calculate() const override;
        Handle<DefaultProbabilityTermStructure> defaultTS() const;
        Real recoveryRate() const;
        Handle<YieldTermStructure> yieldTS() const;
      private:
        Handle<DefaultProbabilityTermStructure> defaultTS_;
        Real recoveryRate_;
        Handle<YieldTermStructure> yieldTS_;
    };


    inline Handle<DefaultProbabilityTermStructure> RiskyBondEngine::defaultTS() const {
        return defaultTS_;
    }

    inline Real RiskyBondEngine::recoveryRate() const { return recoveryRate_; }

    inline Handle<YieldTermStructure> RiskyBondEngine::yieldTS() const { return yieldTS_; }

}

#endif


#ifndef id_d06d2fa4c5eb702b8ce65f2440de3c14
#define id_d06d2fa4c5eb702b8ce65f2440de3c14
inline bool test_d06d2fa4c5eb702b8ce65f2440de3c14(int* i) { return i != 0; }
#endif
