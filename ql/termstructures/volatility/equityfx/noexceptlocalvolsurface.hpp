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

/*! \file noexceptlocalvolsurface.hpp
    \brief wrapper around Dupire local volatility surface,
           which does not throw exception if local volatility becomes negative
*/

#ifndef quantlib_no_except_localvolsurface_hpp
#define quantlib_no_except_localvolsurface_hpp

#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>

namespace QuantLib {

    class NoExceptLocalVolSurface : public LocalVolSurface {
      public:
        NoExceptLocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                                const Handle<YieldTermStructure>& riskFreeTS,
                                const Handle<YieldTermStructure>& dividendTS,
                                const Handle<Quote>& underlying,
                                Real illegalLocalVolOverwrite)
        : LocalVolSurface(blackTS, riskFreeTS, dividendTS, underlying),
          illegalLocalVolOverwrite_(illegalLocalVolOverwrite) { }

        NoExceptLocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                                const Handle<YieldTermStructure>& riskFreeTS,
                                const Handle<YieldTermStructure>& dividendTS,
                                Real underlying,
                                Real illegalLocalVolOverwrite)
        : LocalVolSurface(blackTS, riskFreeTS, dividendTS, underlying),
          illegalLocalVolOverwrite_(illegalLocalVolOverwrite) { }

      protected:
        Volatility localVolImpl(Time t, Real s) const override {
            Volatility vol;
            try {
                vol = LocalVolSurface::localVolImpl(t, s);
            } catch (Error&) {
                vol = illegalLocalVolOverwrite_;
            }

            return vol;
        }

      private:
        const Real illegalLocalVolOverwrite_;
    };
}

#endif


#ifndef id_8cfaf2a12c8383c4372b54b48bd38a17
#define id_8cfaf2a12c8383c4372b54b48bd38a17
inline bool test_8cfaf2a12c8383c4372b54b48bd38a17(const int* i) {
    return i != nullptr;
}
#endif
