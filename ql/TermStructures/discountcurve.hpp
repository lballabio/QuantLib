
/*
  Copyright (C) 2002, 2003 Decillion Pty(Ltd).
  Copyright (C) 2005 StatPro Italia srl

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it under the
  terms of the QuantLib license.  You should have received a copy of the
  license along with this program; if not, please email quantlib-dev@lists.sf.net
  The license is also available online at http://quantlib.org/html/license.html

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discountcurve.hpp
    \brief pre-bootstrapped discount factor structure
*/

#ifndef quantlib_discount_curve_hpp
#define quantlib_discount_curve_hpp

#include <ql/termstructure.hpp>
#include <ql/Math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    //! Term structure based on interpolation of discount factors
    /*! Log-linear interpolation (the default) guarantees
        piecewise-constant forward rates.

        \ingroup yieldtermstructures
    */
    class DiscountCurve : public YieldTermStructure {
      public:
        DiscountCurve(const std::vector<Date>& dates,
                      const std::vector<DiscountFactor>& dfs,
                      const DayCounter& dayCounter);
        //! \name Inspectors
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<DiscountFactor>& discounts() const;
        //@}
        //! \name Modifiers
        //@{
        template <class Traits>
        #if defined(QL_PATCH_MSVC6)
        void setInterpolation(const Traits&) {
        #else
        void setInterpolation() {
        #endif
            interpolation_ =
                Traits::make_interpolation(times_.begin(), times_.end(),
                                           discounts_.begin());
            update();
        }
        //@}
      protected:
        DiscountCurve(const DayCounter&);
        DiscountCurve(const Date& referenceDate, const DayCounter&);
        DiscountCurve(Integer settlementDays, const Calendar&,
                      const DayCounter&);
        DiscountFactor discountImpl(Time) const;
        Size referenceNode(Time) const;
        DayCounter dayCounter_;
        mutable std::vector<Date> dates_;
        mutable std::vector<DiscountFactor> discounts_;
        mutable std::vector<Time> times_;
        mutable Interpolation interpolation_;
    };


    // inline definitions

    inline DayCounter DiscountCurve::dayCounter() const {
        return dayCounter_;
    }

    inline Date DiscountCurve::maxDate() const {
        return dates_.back();
    }

    inline Time DiscountCurve::maxTime() const {
        return times_.back();
    }

    inline const std::vector<Time>& DiscountCurve::times() const {
        return times_;
    }

    inline const std::vector<Date>& DiscountCurve::dates() const {
        return dates_;
    }

    inline const std::vector<DiscountFactor>&
    DiscountCurve::discounts() const {
        return discounts_;
    }

}


#endif
