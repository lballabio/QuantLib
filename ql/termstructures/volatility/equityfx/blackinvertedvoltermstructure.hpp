/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file blackinvertedvoltermstructure.hpp
    \brief Black volatility surface that inverts an existing surface.
    \ingroup termstructures
*/

#ifndef quantext_black_inverted_vol_termstructure_hpp
#define quantext_black_inverted_vol_termstructure_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>

using namespace QuantLib;

namespace QuantExt {

//! Black volatility surface that inverts an existing surface.
/*! This class is used when one wants a USD/EUR volatility, at a given USD/EUR strike
    when only a EUR/USD volatility surface is present.

            \ingroup termstructures
*/
class BlackInvertedVolTermStructure : public BlackVolTermStructure {
public:
    //! Constructor takes a BlackVolTermStructure and takes everything from that
    /*! This will work with both a floating and fixed reference date underlying surface,
        since we are reimplementing the reference date and update methods */
    BlackInvertedVolTermStructure(const Handle<BlackVolTermStructure>& vol)
        : BlackVolTermStructure(vol->businessDayConvention(), vol->dayCounter()), vol_(vol) {
        registerWith(vol_);
    }

    //! return the underlying vol surface
    const Handle<BlackVolTermStructure>& underlyingVol() const { return vol_; }

    //! \name TermStructure interface
    //@{
    const Date& referenceDate() const { return vol_->referenceDate(); }
    Date maxDate() const { return vol_->maxDate(); }
    Natural settlementDays() const { return vol_->settlementDays(); }
    Calendar calendar() const { return vol_->calendar(); }
    //! \name Observer interface
    //@{
    void update() { notifyObservers(); }
    //@}
    //! \name VolatilityTermStructure interface
    //@{
    Real minStrike() const {
        Real min = vol_->minStrike();
        if (min == QL_MIN_REAL || min == 0)
            return 0; // we allow ATM calls
        else
            return 1 / vol_->maxStrike();
    }
    Real maxStrike() const {
        Real min = vol_->minStrike();
        if (min == QL_MIN_REAL || min == 0)
            return QL_MAX_REAL;
        else
            return 1 / min;
    }
    //@}
    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}
protected:
    // we pass through non-recipical values (0 and Null<Real>) assuming they mean ATMF.
    Real invertedStrike(Real strike) const { return (strike == 0.0 || strike == Null<Real>()) ? strike : 1.0 / strike; }
    virtual Real blackVarianceImpl(Time t, Real strike) const { return vol_->blackVariance(t, invertedStrike(strike)); }
    virtual Volatility blackVolImpl(Time t, Real strike) const { return vol_->blackVol(t, invertedStrike(strike)); }

private:
    Handle<BlackVolTermStructure> vol_;
};

// inline definitions
inline void BlackInvertedVolTermStructure::accept(AcyclicVisitor& v) {
    Visitor<BlackInvertedVolTermStructure>* v1 = dynamic_cast<Visitor<BlackInvertedVolTermStructure>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        BlackInvertedVolTermStructure::accept(v);
}
} // namespace QuantExt

#endif
