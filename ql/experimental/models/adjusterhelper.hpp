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

/*! \file adjusterhelper.hpp
    \brief Calibration helper for adjusters
*/

#ifndef quantlib_adjuster_helper_hpp
#define quantlib_adjuster_helper_hpp

#include <ql/models/calibrationhelper.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/instruments/floatfloatswaption.hpp>

namespace QuantLib {

class AdjusterHelper : public CalibrationHelperBase, public LazyObject {
  public:
    AdjusterHelper(const boost::shared_ptr<InterestRateIndex> &index,
                   const Date &fixingDate, const Date &paymentDate,
                   const Real cappedRate = Null<Real>(),
                   const Real flooredRate = Null<Real>())
        : index_(index), fixingDate_(fixingDate), paymentDate_(paymentDate),
          cappedRate_(cappedRate), flooredRate_(flooredRate) {
        registerWith(index_);
        registerWith(Settings::instance().evaluationDate());
    }

    /*! pricer to generate the reference price */
    void
    setCouponPricer(const boost::shared_ptr<FloatingRateCouponPricer> &pricer) {
        unregisterWith(referencePricer_);
        referencePricer_ = pricer;
        registerWith(referencePricer_);
    }

    /*! the model engine must use adjuster to make this helper meaningful */
    void setPricingEngine(const boost::shared_ptr<PricingEngine> &engine) {
        modelEngine_ = engine;
    }

    void performCalculations() const;

    Real referenceValue() const {
        calculate();
        return referenceValue_;
    }
    Real modelValue() const;

    Real calibrationError() { return referenceValue() - modelValue(); }

    const Date fixingDate() { return fixingDate_; }
    const Date paymentDate() { return paymentDate_; }

  private:
    boost::shared_ptr<InterestRateIndex> index_;
    const Date fixingDate_, paymentDate_;
    const Real cappedRate_, flooredRate_;
    mutable boost::shared_ptr<FloatFloatSwap> instrument_;
    mutable boost::shared_ptr<FloatFloatSwaption> dummyOption_;
    boost::shared_ptr<FloatingRateCouponPricer> referencePricer_;
    boost::shared_ptr<PricingEngine> modelEngine_;
    mutable Real referenceValue_;
}; // class AdjusterHelper

} // namespace QuantLib

#endif
