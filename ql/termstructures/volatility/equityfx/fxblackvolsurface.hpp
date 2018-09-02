/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
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

/*! \file fxblackvolsurface.hpp
    \brief FX Black volatility surface that incorperates an FxSmile
  \ingroup termstructures
*/

#ifndef quantext_fx_black_vol_surface_hpp
#define quantext_fx_black_vol_surface_hpp

#include <ql/math/interpolation.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <qle/termstructures/fxvannavolgasmilesection.hpp>

using namespace QuantLib;

namespace QuantExt {

//! Fx Black volatility surface
/*! This class calculates time/strike dependent Black volatilities
  \ingroup termstructures
*/
class FxBlackVolatilitySurface : public BlackVolatilityTermStructure {
public:
    FxBlackVolatilitySurface(const Date& referenceDate, const std::vector<Date>& dates,
                             const std::vector<Volatility>& atmVols, const std::vector<Volatility>& rr25d,
                             const std::vector<Volatility>& bf25d, const DayCounter& dayCounter, const Calendar& cal,
                             const Handle<Quote>& fxSpot, const Handle<YieldTermStructure>& domesticTS,
                             const Handle<YieldTermStructure>& foreignTS);
    //! \name TermStructure interface
    //@{
    DayCounter dayCounter() const { return dayCounter_; }
    Date maxDate() const { return maxDate_; }
    //@}
    //! \name VolatilityTermStructure interface
    //@{
    Real minStrike() const { return 0; } // we allow 0 for ATM vols
    Real maxStrike() const { return QL_MAX_REAL; }
    //@}
    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&);
    //@}
    //! Return an FxSmile for the time t
    /*! Note the smile does not observe the spot or YTS handles, it will
     *  not update when they change
     */
    boost::shared_ptr<FxSmileSection> blackVolSmile(Time t) const;

protected:
    virtual Volatility blackVolImpl(Time t, Real strike) const;

    //! this must be implemented.
    virtual boost::shared_ptr<FxSmileSection> blackVolSmileImpl(Real spot, Real rd, Real rf, Time t, Volatility atm,
                                                                Volatility rr, Volatility bf) const = 0;

private:
    std::vector<Time> times_;
    DayCounter dayCounter_;
    Date maxDate_;
    Handle<Quote> fxSpot_;
    Handle<YieldTermStructure> domesticTS_;
    Handle<YieldTermStructure> foreignTS_;
    BlackVarianceCurve atmCurve_;
    std::vector<Volatility> rr25d_;
    std::vector<Volatility> bf25d_;
    Interpolation rrCurve_;
    Interpolation bfCurve_;
};

// inline definitions
inline void FxBlackVolatilitySurface::accept(AcyclicVisitor& v) {
    Visitor<FxBlackVolatilitySurface>* v1 = dynamic_cast<Visitor<FxBlackVolatilitySurface>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        BlackVolatilityTermStructure::accept(v);
}

//! Fx Black vanna volga volatility surface
/*! This class calculates time/strike dependent Black volatilities
     \ingroup termstructures
*/
class FxBlackVannaVolgaVolatilitySurface : public FxBlackVolatilitySurface {
public:
    FxBlackVannaVolgaVolatilitySurface(const Date& refDate, const std::vector<Date>& dates,
                                       const std::vector<Volatility>& atmVols, const std::vector<Volatility>& rr25d,
                                       const std::vector<Volatility>& bf25d, const DayCounter& dc, const Calendar& cal,
                                       const Handle<Quote>& fx, const Handle<YieldTermStructure>& dom,
                                       const Handle<YieldTermStructure>& fore)
        : FxBlackVolatilitySurface(refDate, dates, atmVols, rr25d, bf25d, dc, cal, fx, dom, fore) {}

protected:
    virtual boost::shared_ptr<FxSmileSection> blackVolSmileImpl(Real spot, Real rd, Real rf, Time t, Volatility atm,
                                                                Volatility rr, Volatility bf) const {
        return boost::shared_ptr<FxSmileSection>(new VannaVolgaSmileSection(spot, rd, rf, t, atm, rr, bf));
    }
};

} // namespace QuantExt

#endif
