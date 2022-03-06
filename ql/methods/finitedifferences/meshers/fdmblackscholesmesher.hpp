/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file fdmblackscholesmesher.hpp
    \brief 1-d mesher for the Black-Scholes process (in ln(S))
*/

#ifndef quantlib_fdm_black_scholes_mesher_hpp
#define quantlib_fdm_black_scholes_mesher_hpp

#include <ql/instruments/dividendschedule.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>

#include <ql/handle.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class FdmQuantoHelper;
    class YieldTermStructure;
    class GeneralizedBlackScholesProcess;

    class FdmBlackScholesMesher : public Fdm1dMesher {
      public:
        FdmBlackScholesMesher(
            Size size,
            const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Time maturity, Real strike,
            Real xMinConstraint = Null<Real>(),
            Real xMaxConstraint = Null<Real>(),
            Real eps = 0.0001,
            Real scaleFactor = 1.5,
            const std::pair<Real, Real>& cPoint
                = (std::pair<Real, Real>(Null<Real>(), Null<Real>())),
            const DividendSchedule& dividendSchedule = DividendSchedule(),
            const ext::shared_ptr<FdmQuantoHelper>& fdmQuantoHelper
                = ext::shared_ptr<FdmQuantoHelper>(),
            Real spotAdjustment = 0.0);

        static ext::shared_ptr<GeneralizedBlackScholesProcess> processHelper(
             const Handle<Quote>& s0,
             const Handle<YieldTermStructure>& rTS,
             const Handle<YieldTermStructure>& qTS,
             Volatility vol);
    };
}

#endif


#ifndef id_628ef478719c84c200a45f97869896d5
#define id_628ef478719c84c200a45f97869896d5
inline bool test_628ef478719c84c200a45f97869896d5(const int* i) {
    return i != nullptr;
}
#endif
