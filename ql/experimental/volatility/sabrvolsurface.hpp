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
#include <boost/array.hpp>

namespace QuantLib {


    //! SABR volatility (smile) surface
    /*! blah blah
    */
    class SabrVolSurface : public InterestRateVolSurface {
      public:
        SabrVolSurface(
                const ext::shared_ptr<InterestRateIndex>&,
                const Handle<BlackAtmVolCurve>&,
                const std::vector<Period>& optionTenors,
                const std::vector<Spread>& atmRateSpreads,
                const std::vector<std::vector<Handle<Quote> > >& volSpreads);
        //@}
        // All virtual methods of base classes must be forwarded
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const Date& referenceDate() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        const Handle<BlackAtmVolCurve>& atmCurve() const;
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
        std::vector<Volatility> volatilitySpreads(const Period&) const;
        std::vector<Volatility> volatilitySpreads(const Date&) const;
      protected:
        boost::array<Real, 4> sabrGuesses(const Date&) const;
      public:
        //@}
        //! \name BlackVolSurface interface
        //@{
        ext::shared_ptr<SmileSection> smileSectionImpl(Time) const;
        //@}
      protected:
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        virtual void update();
        //@}
      private:
        void registerWithMarketData();
        void checkInputs() const;
        void updateSabrGuesses(const Date& d, boost::array<Real, 4> newGuesses) const;
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
        mutable std::vector<boost::array<Real,4> > sabrGuesses_;
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
