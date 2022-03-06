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

/*! \file gaussian1dswaptionvolatility.hpp
    \brief swaption volatility implied by a gaussian 1d model
*/

#ifndef quantlib_swaption_gaussian1d_swaption_volatility_hpp
#define quantlib_swaption_gaussian1d_swaption_volatility_hpp

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/time/period.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/models/shortrate/onefactormodels/gaussian1dmodel.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>

namespace QuantLib {

class Quote;

class Gaussian1dSwaptionVolatility : public SwaptionVolatilityStructure {
  public:
    Gaussian1dSwaptionVolatility(const Calendar& cal,
                                 BusinessDayConvention bdc,
                                 ext::shared_ptr<SwapIndex> indexBase,
                                 const ext::shared_ptr<Gaussian1dModel>& model,
                                 const DayCounter& dc,
                                 ext::shared_ptr<Gaussian1dSwaptionEngine> swaptionEngine =
                                     ext::shared_ptr<Gaussian1dSwaptionEngine>());
    //@{
    Date maxDate() const override { return Date::maxDate(); }
    //@}
    //! \name VolatilityTermStructure interface
    //@{
    Real minStrike() const override { return 0.0; }
    Real maxStrike() const override { return QL_MAX_REAL; }
    //@}
    //! \name SwaptionVolatilityStructure interface
    //@{
    const Period& maxSwapTenor() const override { return maxSwapTenor_; }
    //@}
  protected:
    ext::shared_ptr<SmileSection> smileSectionImpl(const Date&, const Period&) const override;
    ext::shared_ptr<SmileSection> smileSectionImpl(Time, Time) const override;
    Volatility volatilityImpl(const Date&, const Period&, Rate) const override;
    Volatility volatilityImpl(Time, Time, Rate) const override;

  private:
    ext::shared_ptr<SwapIndex> indexBase_;
    ext::shared_ptr<Gaussian1dModel> model_;
    ext::shared_ptr<Gaussian1dSwaptionEngine> engine_;
    const Period maxSwapTenor_;

    class DateHelper;
    friend class DateHelper;
    class DateHelper {
      public:
        DateHelper(const TermStructure &ts, const Time t) : ts_(ts), t_(t) {}
        Real operator()(Real date) const {
            Date d1(static_cast<Date::serial_type>(date));
            Date d2(static_cast<Date::serial_type>(date) + 1);
            Real t1 = ts_.timeFromReference(d1) - t_;
            Real t2 = ts_.timeFromReference(d2) - t_;
            Real h = date - static_cast<Date::serial_type>(date);
            return h * t2 + (1.0 - h) * t1;
        }
        Real derivative(Real date) const {
            // use fwd difference to avoid dates before reference date
            return (operator()(date + 1E-6) - operator()(date)) * 1E6;
        }
        const TermStructure &ts_;
        const Time t_;
    };
};
}

#endif


#ifndef id_19647887126ae220800bafa5b2d524ca
#define id_19647887126ae220800bafa5b2d524ca
inline bool test_19647887126ae220800bafa5b2d524ca(int* i) { return i != 0; }
#endif
