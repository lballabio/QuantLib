/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmaffinemodeltermstructure.cpp
*/

#include <ql/methods/finitedifferences/utilities/fdmaffinemodeltermstructure.hpp>
#include <ql/models/model.hpp>
#include <utility>

namespace QuantLib {

    FdmAffineModelTermStructure::FdmAffineModelTermStructure(Array r,
                                                             const Calendar& cal,
                                                             const DayCounter& dayCounter,
                                                             const Date& referenceDate,
                                                             const Date& modelReferenceDate,
                                                             ext::shared_ptr<AffineModel> model)
    : YieldTermStructure(referenceDate, cal, dayCounter), r_(std::move(r)),
      t_(dayCounter.yearFraction(modelReferenceDate, referenceDate)), model_(std::move(model)) {
        registerWith(model_);
    }

    Date FdmAffineModelTermStructure::maxDate() const {
        return Date::maxDate();
    }

    void FdmAffineModelTermStructure::setVariable(const Array& r) {
        r_ = r;
        notifyObservers();
    }

    DiscountFactor FdmAffineModelTermStructure::discountImpl(Time T) const {
        return model_->discountBond(t_, T+t_, r_);
    }
}
