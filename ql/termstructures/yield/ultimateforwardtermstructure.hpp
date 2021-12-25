/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Marcin Rybacki

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

/*! \file ultimateforwardtermstructure.hpp
    \brief Ultimate Forward Rate term structure
*/

#ifndef quantlib_ultimate_forward_term_structure_hpp
#define quantlib_ultimate_forward_term_structure_hpp

#include <ql/quote.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <utility>

namespace QuantLib {

    //! Ultimate forward term structure

    /*! Dutch regulatory term structure for pension funds with a
        parametrized extrapolation mechanism designed for
        discounting long dated liabilities.

        Relevant documentation can be found on the Dutch Central
        Bank website:

        FTK term structure documentation (Financieel toetsingskader):
        https://www.toezicht.dnb.nl/binaries/50-212329.pdf

        UFR 2015 term structure documentation:
        https://www.toezicht.dnb.nl/binaries/50-234028.pdf

        UFR 2019 term structure documentation:
        https://www.rijksoverheid.nl/documenten/kamerstukken/2019/06/11/advies-commissie-parameters

        This term structure will remain linked to the original
        structure, i.e., any changes in the latter will be
        reflected in this structure as well.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned zero rates is tested by
          checking them against reference values obtained
          from the official source.
        - extrapolated forward is validated.
        - rates on the cut-off point are checked against those
          implied by the base curve.
        - inspectors are tested against the base curve.
        - incorrect input for cut-off point should raise an exception.
        - observability against changes in the underlying term
          structure and the additional components is checked.
    */

    class UltimateForwardTermStructure : public ZeroYieldStructure {
      public:
        UltimateForwardTermStructure(Handle<YieldTermStructure>,
                                     Handle<Quote> lastLiquidForwardRate,
                                     Handle<Quote> ultimateForwardRate,
                                     const Period& firstSmoothingPoint,
                                     Real alpha);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        const Date& referenceDate() const override;
        Date maxDate() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
      protected:
        //! returns the UFR extended zero yield rate
        Rate zeroYieldImpl(Time) const override;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> llfr_;
        Handle<Quote> ufr_;
        Period fsp_;
        Real alpha_;
    };

    // inline definitions

    inline UltimateForwardTermStructure::UltimateForwardTermStructure(
        Handle<YieldTermStructure> h,
        Handle<Quote> lastLiquidForwardRate,
        Handle<Quote> ultimateForwardRate,
        const Period& firstSmoothingPoint,
        Real alpha)
    : originalCurve_(std::move(h)), llfr_(std::move(lastLiquidForwardRate)),
      ufr_(std::move(ultimateForwardRate)), fsp_(firstSmoothingPoint), alpha_(alpha) {
        QL_REQUIRE(fsp_.length() > 0,
                   "first smoothing point must be a period with positive length");
        if (!originalCurve_.empty())
            enableExtrapolation(originalCurve_->allowsExtrapolation());
        registerWith(originalCurve_);
        registerWith(llfr_);
        registerWith(ufr_);
    }

    inline DayCounter UltimateForwardTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar UltimateForwardTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural UltimateForwardTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline const Date& UltimateForwardTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date UltimateForwardTermStructure::maxDate() const { return Date::maxDate(); }

    inline void UltimateForwardTermStructure::update() {
        if (!originalCurve_.empty()) {
            YieldTermStructure::update();
            enableExtrapolation(originalCurve_->allowsExtrapolation());
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            // NOLINTNEXTLINE(bugprone-parent-virtual-call)
            TermStructure::update();
        }
    }

    inline Rate UltimateForwardTermStructure::zeroYieldImpl(Time t) const {
        Time cutOffTime = originalCurve_->timeFromReference(referenceDate() + fsp_);
        Time deltaT = t - cutOffTime;
        /* If time to maturity (T) exceeds the cut-off point (T_c),
           i.e. the first smoothing point, the forward rate f is
           extrapolated as follows:

           f(t,T_c,T) = UFR(t) + (LLFR(t) - UFR(t)) * B(T-T_c),

           where:
           UFR(t) - Ultimate Forward Rate quote,
           LLFR(t) - Last Liquid Forward Rate quote,
           B(t-T_c) = [1 - exp(-a * (T-T_c))] / [a * (T-T_c)],
           with a being the growth factor (alpha). */
        if (deltaT > 0.0) {
            InterestRate baseRate = originalCurve_->zeroRate(cutOffTime, Continuous, NoFrequency);
            Real beta = (1.0 - std::exp(-alpha_ * deltaT)) / (alpha_ * deltaT);
            Rate extrapolatedForward = ufr_->value() + (llfr_->value() - ufr_->value()) * beta;
            return (cutOffTime * baseRate + deltaT * extrapolatedForward) / t;
        }
        return originalCurve_->zeroRate(t, Continuous, NoFrequency);
    }
}

#endif
