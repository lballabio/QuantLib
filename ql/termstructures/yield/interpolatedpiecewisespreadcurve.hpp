/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Roland Lichters
 Copyright (C) 2006, 2008, 2014 StatPro Italia srl
 Copyright (C) 2010 Robert Philipp

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

/*! \file interpolatedpiecewisespreadcurve.hpp
    \brief Interpolated Piecewise Spread Curve
*/

#ifndef quantlib_interpolated_piecewise_spread_curve_hpp
#define quantlib_interpolated_piecewise_spread_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yield/forwardstructure.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    template <class Interpolator>
    class InterpolatedPiecewiseSpreadCurve : public virtual Observer,
                                             public virtual Observable {
        public:
            InterpolatedPiecewiseSpreadCurve(std::vector<Handle<Quote>> spreads,
                                            const std::vector<Date>& dates,
                                            const Date& referenceDate,
                                            const DayCounter& dc = DayCounter(),
                                            const Interpolator& factory = Interpolator());
            
            //! \name Observer interface
            //@{
            void update() override;
            //@}
        protected:
            Time timeFromReference(const Date& d) const;
            void updateInterpolation();
            Real calcSpread(Time t) const;
        private:
            std::vector<Handle<Quote>> spreads_;
            std::vector<Date> dates_;
            DayCounter dc_;
            Date refDate_;
            std::vector<Time> times_;
            std::vector<Spread> spreadValues_;
            Interpolator factory_;
            Interpolation interpolator_;
    };

    //inline definitions

    template <class T>
    inline InterpolatedPiecewiseSpreadCurve<
        T>::InterpolatedPiecewiseSpreadCurve(std::vector<Handle<Quote> > spreads,
                                            const std::vector<Date>& dates,
                                            const Date& referenceDate,
                                            const DayCounter& dc,
                                            const T& factory)
    : spreads_(std::move(spreads)), dates_(std::move(dates)), dc_(dc), refDate_(referenceDate),
      times_(dates.size()), spreadValues_(dates.size()), factory_(factory) {
        QL_REQUIRE(!spreads_.empty(), "no spreads given");
        QL_REQUIRE(spreads_.size() == dates_.size(),
                   "spread and date vector have different sizes");
        for (auto& spread : spreads_)
            registerWith(spread);
        updateInterpolation();
    }

    template <class T>
    void InterpolatedPiecewiseSpreadCurve<T>::update() {
        updateInterpolation();
        notifyObservers();
    }

    template <class T>
    inline Time InterpolatedPiecewiseSpreadCurve<T>::timeFromReference(const Date& d) const {
        return dc_.yearFraction(refDate_, d);
    }

    template <class T>
    inline Spread
    InterpolatedPiecewiseSpreadCurve<T>::calcSpread(Time t) const {
        if (t <= times_.front()) {
            return spreads_.front()->value();
        } else if (t >= times_.back()) {
            return spreads_.back()->value();
        } else {
            return interpolator_(t, true);
        }
    }

    template <class T>
    inline void InterpolatedPiecewiseSpreadCurve<T>::updateInterpolation() {
        for (Size i = 0; i < dates_.size(); i++) {
            times_[i] = timeFromReference(dates_[i]);
            spreadValues_[i] = spreads_[i]->value();
        }
        interpolator_ = factory_.interpolate(times_.begin(),
                                             times_.end(),
                                             spreadValues_.begin());
    }

}

#endif
