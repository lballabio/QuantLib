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

/*! \file fdmblackscholesmultistrikemesher.hpp
    \brief 1-d mesher for the Black-Scholes process (in ln(S))
*/

#ifndef quantlib_fdm_black_scholes_multi_strike_mesher_hpp
#define quantlib_fdm_black_scholes_multi_strike_mesher_hpp

#include <ql/instruments/dividendschedule.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/handle.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class GeneralizedBlackScholesProcess;

    class FdmBlackScholesMultiStrikeMesher : public Fdm1dMesher {
      public:
        FdmBlackScholesMultiStrikeMesher(
            Size size,
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            Time maturity, const std::vector<Real>& strikes,
            Real eps = 0.0001,
            Real scaleFactor = 1.5,
            const std::pair<Real, Real>& cPoint
                        = (std::pair<Real, Real>(Null<Real>(), Null<Real>())));

        static boost::shared_ptr<GeneralizedBlackScholesProcess> processHelper(
             const Handle<Quote>& s0,
             const Handle<YieldTermStructure>& rTS,
             const Handle<YieldTermStructure>& qTS,
             Volatility vol);
    };
}
#endif
