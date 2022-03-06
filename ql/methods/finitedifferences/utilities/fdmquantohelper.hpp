/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

/*! \file fdmquantohelper.hpp
    \brief helper class storing market data needed for the quanto adjustment.
*/

#ifndef quantlib_fdm_quanto_helper_hpp
#define quantlib_fdm_quanto_helper_hpp

#include <ql/math/array.hpp>
#include <ql/patterns/observable.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class BlackVolTermStructure;

    class FdmQuantoHelper : public Observable {
    public:
      FdmQuantoHelper(ext::shared_ptr<YieldTermStructure> rTS,
                      ext::shared_ptr<YieldTermStructure> fTS,
                      ext::shared_ptr<BlackVolTermStructure> fxVolTS,
                      Real equityFxCorrelation,
                      Real exchRateATMlevel);

      Rate quantoAdjustment(Volatility equityVol, Time t1, Time t2) const;
      Disposable<Array> quantoAdjustment(const Array& equityVol, Time t1, Time t2) const;

      const ext::shared_ptr<YieldTermStructure> rTS_, fTS_;
      const ext::shared_ptr<BlackVolTermStructure> fxVolTS_;
      const Real equityFxCorrelation_;
      const Real exchRateATMlevel_;
    };
}

#endif


#ifndef id_d790958552c5d62ad7f060f9bd10145f
#define id_d790958552c5d62ad7f060f9bd10145f
inline bool test_d790958552c5d62ad7f060f9bd10145f(const int* i) {
    return i != nullptr;
}
#endif
