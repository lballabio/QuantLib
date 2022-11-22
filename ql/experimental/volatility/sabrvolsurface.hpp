/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file sabrvolsurface.hpp
    \brief SABR volatility (smile) surface
*/

#ifndef quantlib_sabr_vol_surface_hpp
#define quantlib_sabr_vol_surface_hpp

#include <ql/experimental/volatility/interestratevolsurface.hpp>
#include <ql/experimental/volatility/blackatmvolcurve.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/sabrinterpolatedsmilesection.hpp>
#include <array>

namespace QuantLib {


    //! SABR volatility (smile) surface
    /*! blah blah
    */
    class SabrVolSurface : public InterestRateVolSurface {
      public:
        SabrVolSurface(const ext::shared_ptr<InterestRateIndex>&,
                       Handle<BlackAtmVolCurve>,
                       const std::vector<Period>& optionTenors,
                       std::vector<Spread> atmRateSpreads,
                       std::vector<std::vector<Handle<Quote> > > volSpreads);
        //@}
        // All virtual methods of base classes must be forwarded
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Date maxDate() const override;
        Time maxTime() const override;
        const Date& referenceDate() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        const Handle<BlackAtmVolCurve>& atmCurve() const;
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        std::vector<Volatility> volatilitySpreads(const Period&) const;
        std::vector<Volatility> volatilitySpreads(const Date&) const;
      protected:
        std::array<Real, 4> sabrGuesses(const Date&) const;
      public:
        //@}
        //! \name BlackVolSurface interface
        //@{
        ext::shared_ptr<SmileSection> smileSectionImpl(Time) const override;
        //@}
      protected:
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        void update() override;
        //@}
      private:
        void registerWithMarketData();
        void checkInputs() const;
        void updateSabrGuesses(const Date& d, std::array<Real, 4> newGuesses) const;
        Handle<BlackAtmVolCurve> atmCurve_;
        std::vector<Period> optionTenors_;
        std::vector<Time> optionTimes_;
        std::vector<Date> optionDates_;
        std::vector<Spread> atmRateSpreads_;
        std::vector<std::vector<Handle<Quote> > > volSpreads_;
        //
        bool isAlphaFixed_;
        bool isBetaFixed_;
        bool isNuFixed_;
        bool isRhoFixed_;
        bool vegaWeighted_;
        //
        mutable std::vector<std::array<Real,4>> sabrGuesses_;
    };

    // inline

    inline DayCounter SabrVolSurface::dayCounter() const {
        return atmCurve_->dayCounter();
    }

    inline Date SabrVolSurface::maxDate() const {
        return atmCurve_->maxDate();
    }

    inline Time SabrVolSurface::maxTime() const {
        return atmCurve_->maxTime();
    }

    inline const Date& SabrVolSurface::referenceDate() const {
        return atmCurve_->referenceDate();
    }

    inline Calendar SabrVolSurface::calendar() const {
        return atmCurve_->calendar();
    }

    inline Natural SabrVolSurface::settlementDays() const {
        return atmCurve_->settlementDays();
    }

    inline Real SabrVolSurface::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real SabrVolSurface::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline const Handle<BlackAtmVolCurve>& SabrVolSurface::atmCurve() const {
        return atmCurve_;
    }

    inline std::vector<Volatility>
    SabrVolSurface::volatilitySpreads(const Period& p) const {
        return volatilitySpreads(optionDateFromTenor(p));
    }
}

#endif
