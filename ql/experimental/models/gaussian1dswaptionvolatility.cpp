/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/models/gaussian1dswaptionvolatility.hpp>
#include <ql/experimental/models/gaussian1dsmilesection.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {

Gaussian1dSwaptionVolatility::Gaussian1dSwaptionVolatility(
    const Calendar &cal, BusinessDayConvention bdc,
    const boost::shared_ptr<SwapIndex> &indexBase,
    const boost::shared_ptr<Gaussian1dModel> &model, const DayCounter &dc,
    const boost::shared_ptr<Gaussian1dSwaptionEngine> swaptionEngine)
    : SwaptionVolatilityStructure(model->termStructure()->referenceDate(), cal,
                                  bdc, dc),
      indexBase_(indexBase), model_(model), engine_(swaptionEngine),
      maxSwapTenor_(100 * Years) {}

boost::shared_ptr<SmileSection>
Gaussian1dSwaptionVolatility::smileSectionImpl(const Date &d,
                                               const Period &tenor) const {
    boost::shared_ptr<SmileSection> tmp =
        boost::make_shared<Gaussian1dSmileSection>(
            d, indexBase_->clone(tenor), model_, this->dayCounter(), engine_);
    return tmp;
}

boost::shared_ptr<SmileSection>
Gaussian1dSwaptionVolatility::smileSectionImpl(Time optionTime,
                                               Time swapLength) const {
    DateHelper hlp(*this, optionTime);
    NewtonSafe newton;
    Date d(static_cast<BigInteger>(newton.solve(
        hlp, 0.1,
        365.25 * optionTime + static_cast<Real>(referenceDate().serialNumber()),
        1.0)));
    Period tenor(
        static_cast<Integer>(Rounding(0).operator()(swapLength * 12.0)),
        Months);
    d = indexBase_->fixingCalendar().adjust(d);
    return smileSectionImpl(d, tenor);
}

Volatility Gaussian1dSwaptionVolatility::volatilityImpl(const Date &d,
                                                        const Period &tenor,
                                                        Rate strike) const {
    return smileSectionImpl(d, tenor)->volatility(strike);
}

Volatility Gaussian1dSwaptionVolatility::volatilityImpl(Time optionTime,
                                                        Time swapLength,
                                                        Rate strike) const {
    return smileSectionImpl(optionTime, swapLength)->volatility(strike);
}
}
