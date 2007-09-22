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

#include <ql/voltermstructures/interestratevolsurface.hpp>
#include <ql/voltermstructures/blackatmvolcurve.hpp>
#include <ql/quote.hpp>
#include <boost/array.hpp>

namespace QuantLib {


    //! SABR volatility (smile) surface
    /*! blah blah
    */
    class SabrVolSurface : public InterestRateVolSurface {
      public:
        SabrVolSurface(
                const boost::shared_ptr<InterestRateIndex>&,
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
        const Handle<BlackAtmVolCurve>& atmCurve() const;
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        std::vector<Volatility> volatilitySpreads(const Date&) const;
        boost::array<Real, 4> sabrGuesses(const Date&) const;
        //! \name BlackVolSurface interface
        //@{
        boost::shared_ptr<SmileSection> smileSectionImpl(Time) const;
        //@}
      private:
        Handle<BlackAtmVolCurve> atmCurve_;
        std::vector<Period> optionTenors_;
        std::vector<Spread> atmRateSpreads_;
        std::vector<std::vector<Handle<Quote> > > volSpreads_;
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

    inline const Handle<BlackAtmVolCurve>& SabrVolSurface::atmCurve() const {
        return atmCurve_;
    }
}

#endif
