/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file impliedvoltermstructure.hpp
    \brief Implied Black Vol Term Structure
*/

#ifndef quantlib_implied_vol_term_structure_hpp
#define quantlib_implied_vol_term_structure_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Implied vol term structure at a given date in the future
    /*! The given date will be the implied reference date.
        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be reflected
              in this structure as well.

        \warning It doesn't make financial sense to have an
                 asset-dependent implied Vol Term Structure.  This
                 class should be used with term structures that are
                 time-dependent only.
    */
    class ImpliedVolTermStructure : public BlackVarianceTermStructure {
      public:
        ImpliedVolTermStructure(Handle<BlackVolTermStructure> origTS, const Date& referenceDate);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override { return originalTS_->dayCounter(); }
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Real blackVarianceImpl(Time t, Real strike) const override;

      private:
        Handle<BlackVolTermStructure> originalTS_;
    };


    // inline definitions

    inline ImpliedVolTermStructure::ImpliedVolTermStructure(
        Handle<BlackVolTermStructure> originalTS, const Date& referenceDate)
    : BlackVarianceTermStructure(referenceDate), originalTS_(std::move(originalTS)) {
        registerWith(originalTS_);
    }

    inline Date ImpliedVolTermStructure::maxDate() const {
        return originalTS_->maxDate();
    }

    inline Real ImpliedVolTermStructure::minStrike() const {
        return originalTS_->minStrike();
    }

    inline Real ImpliedVolTermStructure::maxStrike() const {
        return originalTS_->maxStrike();
    }

    inline void ImpliedVolTermStructure::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ImpliedVolTermStructure>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

    inline Real ImpliedVolTermStructure::blackVarianceImpl(Time t,
                                                           Real strike) const {
        /* timeShift (and/or variance) variance at evaluation date
           cannot be cached since the original curve could change
           between invocations of this method */
        Time timeShift =
            dayCounter().yearFraction(originalTS_->referenceDate(),
                                      referenceDate());
        /* t is relative to the current reference date
           and needs to be converted to the time relative
           to the reference date of the original curve */
        return originalTS_->blackForwardVariance(timeShift,
                                                 timeShift+t,
                                                 strike,
                                                 true);
    }

}

#endif
