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

/*! \file fdmquantohelper.cpp
\brief quanto helper to store market data needed for the quanto adjustment.
*/

#include <ql/methods/finitedifferences/utilities/fdmquantohelper.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

    FdmQuantoHelper::FdmQuantoHelper(ext::shared_ptr<YieldTermStructure> rTS,
                                     ext::shared_ptr<YieldTermStructure> fTS,
                                     ext::shared_ptr<BlackVolTermStructure> fxVolTS,
                                     Real equityFxCorrelation,
                                     Real exchRateATMlevel)
    : rTS_(std::move(rTS)), fTS_(std::move(fTS)), fxVolTS_(std::move(fxVolTS)),
      equityFxCorrelation_(equityFxCorrelation), exchRateATMlevel_(exchRateATMlevel) {}

    Rate FdmQuantoHelper::quantoAdjustment(Volatility equityVol,
                                                 Time t1, Time t2) const {
        const Rate rDomestic = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate rForeign  = fTS_->forwardRate(t1, t2, Continuous).rate();
        const Volatility fxVol
            = fxVolTS_->blackForwardVol(t1, t2, exchRateATMlevel_);

        return rDomestic - rForeign + equityVol*fxVol*equityFxCorrelation_;
    }

    Disposable<Array> FdmQuantoHelper::quantoAdjustment(
        const Array& equityVol, Time t1, Time t2) const {

        const Rate rDomestic = rTS_->forwardRate(t1, t2, Continuous).rate();
        const Rate rForeign  = fTS_->forwardRate(t1, t2, Continuous).rate();
        const Volatility fxVol
            = fxVolTS_->blackForwardVol(t1, t2, exchRateATMlevel_);

        Array retVal(equityVol.size());
        for (Size i=0; i < retVal.size(); ++i) {
            retVal[i]
                = rDomestic - rForeign + equityVol[i]*fxVol*equityFxCorrelation_;
        }
        return retVal;
    }
}
