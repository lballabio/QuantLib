/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

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

/*! \file blackvariancesurfacefromsmilesections.hpp
    \brief Black volatility surface built from smile sections
*/

#ifndef quantlib_black_variance_surface_from_smile_sections_hpp
#define quantlib_black_variance_surface_from_smile_sections_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    //! Black volatility surface built from smile sections
    /*! This class builds a Black volatility surface from a set of
        smile sections, one per tenor. It interpolates linearly in
        total variance between tenors for a given strike.

    */
    class BlackVarianceSurfaceFromSmileSections
        : public BlackVarianceTermStructure {
      public:
        BlackVarianceSurfaceFromSmileSections(
            const Date& referenceDate,
            const std::vector<Date>& dates,
            std::vector<ext::shared_ptr<SmileSection>> smileSections,
            DayCounter dayCounter = DayCounter());

        DayCounter dayCounter() const override;
        Date maxDate() const override;
        Real minStrike() const override;
        Real maxStrike() const override;
        void accept(AcyclicVisitor&) override;

      protected:
        Real blackVarianceImpl(Time t, Real strike) const override;

      private:
        DayCounter dayCounter_;
        Date maxDate_;
        std::vector<Time> times_;
        std::vector<ext::shared_ptr<SmileSection>> smileSections_;
    };


    // inline definitions

    inline DayCounter
    BlackVarianceSurfaceFromSmileSections::dayCounter() const {
        return dayCounter_;
    }

    inline Date
    BlackVarianceSurfaceFromSmileSections::maxDate() const {
        return maxDate_;
    }

    inline Real
    BlackVarianceSurfaceFromSmileSections::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real
    BlackVarianceSurfaceFromSmileSections::maxStrike() const {
        return QL_MAX_REAL;
    }

}

#endif
