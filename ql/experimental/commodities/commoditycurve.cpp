/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

#include <ql/experimental/commodities/commoditycurve.hpp>
#include <ql/experimental/commodities/commoditypricinghelpers.hpp>

namespace QuantLib {

    CommodityCurve::CommodityCurve(const std::string& name,
                                   const CommodityType& commodityType,
                                   const Currency& currency,
                                   const UnitOfMeasure& unitOfMeasure,
                                   const Calendar& calendar,
                                   const std::vector<Date>& dates,
                                   const std::vector<Real>& prices,
                                   const DayCounter& dayCounter)
    : TermStructure(dates[0], calendar, dayCounter),
      name_(name), commodityType_(commodityType), unitOfMeasure_(unitOfMeasure),
      currency_(currency), dates_(dates), data_(prices),
      interpolator_(ForwardFlat()), basisOfCurveUomConversionFactor_(1) {

        QL_REQUIRE(dates_.size()>1, "too few dates");
        QL_REQUIRE(data_.size()==dates_.size(), "dates/prices count mismatch");

        times_.resize(dates_.size());
        times_[0]=0.0;
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }

        interpolation_ =
            interpolator_.interpolate(times_.begin(), times_.end(),
                                      data_.begin());
        interpolation_.update();
    }

    CommodityCurve::CommodityCurve(const std::string& name,
                                   const CommodityType& commodityType,
                                   const Currency& currency,
                                   const UnitOfMeasure& unitOfMeasure,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter)
    : TermStructure(0, calendar, dayCounter),
      name_(name), commodityType_(commodityType), unitOfMeasure_(unitOfMeasure),
      currency_(currency), interpolator_(ForwardFlat()),
      basisOfCurveUomConversionFactor_(1) {}

    void CommodityCurve::setPrices(std::map<Date, Real>& prices) {
        QL_REQUIRE(prices.size()>1, "too few prices");

        dates_.clear();
        data_.clear();
        for (std::map<Date, Real>::const_iterator i = prices.begin(); i != prices.end(); i++) {
            dates_.push_back(i->first);
            data_.push_back(i->second);
        }

        times_.resize(dates_.size());
        times_[0]=0.0;
        for (Size i = 1; i < dates_.size(); i++)
            times_[i] = dayCounter().yearFraction(dates_[0], dates_[i]);

        interpolation_ =
            interpolator_.interpolate(times_.begin(), times_.end(),
                                      data_.begin());
        interpolation_.update();
    }

    void CommodityCurve::setBasisOfCurve(
                      const boost::shared_ptr<CommodityCurve>& basisOfCurve) {
        basisOfCurve_ = basisOfCurve;
        basisOfCurveUomConversionFactor_ =
            CommodityPricingHelper::calculateUomConversionFactor(
                                                commodityType_,
                                                basisOfCurve_->unitOfMeasure_,
                                                unitOfMeasure_);
    }

    std::ostream& operator<<(std::ostream& out, const CommodityCurve& curve) {
        out << "[" << curve.name_ << "] (" << curve.currency_.code()
            << "/" << curve.unitOfMeasure_.code() << ")";
        if (curve.basisOfCurve_ != 0)
            out << "; basis to (" << (*curve.basisOfCurve_) << ")";
        return out;
    }

}
