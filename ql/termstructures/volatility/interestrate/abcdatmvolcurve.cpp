/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/termstructures/volatility/interestrate/abcdatmvolcurve.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    // floating reference date, floating market data
    AbcdAtmVolCurve::AbcdAtmVolCurve(Natural settlDays,
                                     const Calendar& cal,
                                     const std::vector<Period>& optionTenors,
                                     const std::vector<Handle<Quote> >& vols,
                                     BusinessDayConvention bdc,
                                     const DayCounter& dc)
    : BlackAtmVolCurve(settlDays, cal, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      volHandles_(vols),
      vols_(vols.size()),
      interpolation_(boost::shared_ptr<AbcdInterpolation>()) // do not initialize with nOptionTenors_
    {
        checkInputs();
        initializeOptionDatesAndTimes();
        registerWithMarketData();
        for (Size i=0; i<vols_.size(); ++i)
            vols_[i] = volHandles_[i]->value();
        interpolate();
    }

    void AbcdAtmVolCurve::checkInputs() const
    {
        QL_REQUIRE(!optionTenors_.empty(), "empty option tenor vector");
        QL_REQUIRE(nOptionTenors_==vols_.size(),
                   "mismatch between number of option tenors (" <<
                   nOptionTenors_ << ") and number of volatilities (" <<
                   vols_.size() << ")");
        QL_REQUIRE(optionTenors_[0]>0*Days,
                   "negative first option tenor: " << optionTenors_[0]);
        for (Size i=1; i<nOptionTenors_; ++i)
            QL_REQUIRE(optionTenors_[i]>optionTenors_[i-1],
                       "non increasing option tenor: " << io::ordinal(i-1) <<
                       " is " << optionTenors_[i-1] << ", " <<
                       io::ordinal(i) << " is " << optionTenors_[i]);
    }

    void AbcdAtmVolCurve::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            registerWith(volHandles_[i]);
    }

    void AbcdAtmVolCurve::interpolate()
    {
        interpolation_ = boost::shared_ptr<AbcdInterpolation>(new
                            AbcdInterpolation(optionTimes_.begin(),
                                              optionTimes_.end(),
                                              vols_.begin()));
    }

    void AbcdAtmVolCurve::accept(AcyclicVisitor& v) {
        Visitor<AbcdAtmVolCurve>* v1 =
            dynamic_cast<Visitor<AbcdAtmVolCurve>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a AbcdAtmVolCurve visitor");
    }

    void AbcdAtmVolCurve::update()
    {
        // recalculate dates if necessary...
        if (moving_) {
            Date d = Settings::instance().evaluationDate();
            if (evaluationDate_ != d) {
                evaluationDate_ = d;
                initializeOptionDatesAndTimes();
            }
        }
        BlackAtmVolCurve::update();
        LazyObject::update();
    }

    void AbcdAtmVolCurve::initializeOptionDatesAndTimes() const
    {
        for (Size i=0; i<nOptionTenors_; ++i) {
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }
    }

    void AbcdAtmVolCurve::performCalculations() const
    {
        // check if date recalculation must be called here

        for (Size i=0; i<vols_.size(); ++i)
            vols_[i] = volHandles_[i]->value();

        interpolation_->update();
    }

}
