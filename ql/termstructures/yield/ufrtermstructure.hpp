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

/*! \file ufrtermstructure.hpp
    \brief Ultimate Forward Rate term structure
*/

#ifndef quantlib_ufr_term_structure_hpp
#define quantlib_ufr_term_structure_hpp

#include <ql/quote.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>

namespace QuantLib {

    //! Ultimate Forward Rate term structure

    /*! Dutch regulatory term structure for pension funds with a
        parametrized extrapolation mechanism designed for
        discounting long date liabilities.

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
        - tbd
    */

    class UFRTermStructure : public ZeroYieldStructure {
      public:
        UFRTermStructure(const Handle<YieldTermStructure>&,
                         const Handle<Quote>& lastLiquidForwardRate,
                         const Handle<Quote>& ultimateForwardRate,
                         Time firstSmoothingPoint,
                         Real alpha);
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        const Date& referenceDate() const;
        Date maxDate() const;
        Time maxTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the UFR extended zero yield rate
        Rate zeroYieldImpl(Time) const;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> llfr_;
        Handle<Quote> ufr_;
        Time fsp_;
        Real alpha_;
    };

    // inline definitions

    inline UFRTermStructure::UFRTermStructure(const Handle<YieldTermStructure>& h,
                                              const Handle<Quote>& lastLiquidForwardRate,
                                              const Handle<Quote>& ultimateForwardRate,
                                              Time firstSmoothingPoint,
                                              Real alpha)
    : originalCurve_(h), llfr_(lastLiquidForwardRate), ufr_(ultimateForwardRate),
      fsp_(firstSmoothingPoint), alpha_(alpha) {
        if (!originalCurve_.empty())
            enableExtrapolation(originalCurve_->allowsExtrapolation());
        registerWith(originalCurve_);
        registerWith(llfr_);
        registerWith(ufr_);
    }

    inline DayCounter UFRTermStructure::dayCounter() const { return originalCurve_->dayCounter(); }

    inline Calendar UFRTermStructure::calendar() const { return originalCurve_->calendar(); }

    inline Natural UFRTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline const Date& UFRTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date UFRTermStructure::maxDate() const { return originalCurve_->maxDate(); }

    inline Time UFRTermStructure::maxTime() const { return originalCurve_->maxTime(); }

    inline void UFRTermStructure::update() {
        if (!originalCurve_.empty()) {
            YieldTermStructure::update();
            enableExtrapolation(originalCurve_->allowsExtrapolation());
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            TermStructure::update();
        }
    }

    inline Rate UFRTermStructure::zeroYieldImpl(Time t) const {
        Time deltaT = t - fsp_;
        if (deltaT > 0.0) {
            InterestRate baseRate = originalCurve_->zeroRate(fsp_, Continuous, NoFrequency, true);
            Real beta = (1.0 - std::exp(-alpha_ * deltaT)) / (alpha_ * deltaT);
            Rate extrapolatedForward = ufr_->value() + (llfr_->value() - ufr_->value()) * beta;
            return (fsp_ * baseRate + deltaT * extrapolatedForward) / t;
        }
        return originalCurve_->zeroRate(t, Continuous, NoFrequency, true);
    }
}

#endif