/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
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

/*! \file abcdatmvolcurve.hpp
    \brief Abcd-interpolated at-the-money (no-smile) interest rate vol curve
*/

#ifndef quantlib_abcd_atm_vol_curve_hpp
#define quantlib_abcd_atm_vol_curve_hpp

#include <ql/termstructures/volatility/blackatmvolcurve.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/abcdinterpolation.hpp>
#include <ql/quote.hpp>
#include <ql/patterns/lazyobject.hpp>

namespace QuantLib {

    //! Abcd-interpolated at-the-money (no-smile) volatility curve
    /*! blah blah
    */
    class AbcdAtmVolCurve : public BlackAtmVolCurve,
                            public LazyObject {
      public:
        //! floating reference date, floating market data
        AbcdAtmVolCurve(Natural settlementDays,
                        const Calendar& cal,
                        const std::vector<Period>& optionTenors,
                        const std::vector<Handle<Quote> >& vols,
                        BusinessDayConvention bdc = Following,
                        const DayCounter& dc = Actual365Fixed());
        std::vector<Real> k() const;
        Real a() const;
        Real b() const;
        Real c() const;
        Real d() const;
        Real error() const;
        Real maxError() const;
        //! \name TermStructure interface
        //@{
        virtual Date maxDate() const;
        //@}
        //! \name LazyObject interface
        //@{
        void update();
        void performCalculations() const;
        //@}
        //! \name some inspectors
        //@{
        const std::vector<Period>& optionTenors() const;
        const std::vector<Date>& optionDates() const;
        const std::vector<Time>& optionTimes() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        //! \name BlackAtmVolCurve interface
        //@{
        //! spot at-the-money variance calculation
        virtual Real atmVarianceImpl(Time t) const;
        //! spot at-the-money volatility calculation
        virtual Volatility atmVolImpl(Time t) const;
        //@}
      private:
        void checkInputs() const;
        void initializeOptionDatesAndTimes() const;
        void registerWithMarketData();
        void interpolate();

        Size nOptionTenors_;
        std::vector<Period> optionTenors_;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        Date evaluationDate_;

        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Volatility> vols_;

        boost::shared_ptr<AbcdInterpolation> interpolation_;
    };

    // inline

    inline Date AbcdAtmVolCurve::maxDate() const {
        calculate();
        return optionDateFromTenor(optionTenors_.back());
    }

    inline Real AbcdAtmVolCurve::atmVarianceImpl(Time t) const {
        Volatility vol = atmVolImpl(t);
        return vol*vol*t;
    }

    inline Volatility AbcdAtmVolCurve::atmVolImpl(Time t) const {
        calculate();
        return interpolation_->operator() (t, true);
    }

    inline const std::vector<Period>& AbcdAtmVolCurve::optionTenors() const {
         return optionTenors_;
    }

    inline
    const std::vector<Date>& AbcdAtmVolCurve::optionDates() const {
        return optionDates_;
    }

    inline
    const std::vector<Time>& AbcdAtmVolCurve::optionTimes() const {
        return optionTimes_;
    }

    inline
    std::vector<Real> AbcdAtmVolCurve::k() const {
        return interpolation_->k();
    }
    
    inline Real AbcdAtmVolCurve::a() const {
        return interpolation_->a();
    }

    inline Real AbcdAtmVolCurve::b() const {
        return interpolation_->b();
    }

    inline Real AbcdAtmVolCurve::c() const {
        return interpolation_->c();
    }

    inline Real AbcdAtmVolCurve::d() const {
        return interpolation_->d();
    }

    inline Real AbcdAtmVolCurve::error() const {
        return interpolation_->interpolationError();
    }
    inline Real AbcdAtmVolCurve::maxError() const {
        return interpolation_->interpolationMaxError();
    }
}

#endif
