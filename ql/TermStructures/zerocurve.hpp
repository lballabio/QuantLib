
/*
  Copyright (C) 2003 RiskMap.

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

/*! \file zerocurve.hpp
    \brief pre-bootstrapped zero curve structure
*/

#ifndef quantlib_zero_curve_hpp
#define quantlib_zero_curve_hpp

#include <ql/termstructure.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Math/linearinterpolation.hpp>

namespace QuantLib {

    //! Term structure based on linear interpolation of zero yields
    /*! \ingroup yieldtermstructures */
    class ZeroCurve : public ZeroYieldStructure {
      public:
        // constructor
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor without today's date */
        ZeroCurve(const Date& todaysDate,
                  const std::vector<Date>& dates,
                  const std::vector<Rate>& yields,
                  const DayCounter& dayCounter = Actual365Fixed());
        #endif
        ZeroCurve(const std::vector<Date>& dates,
                  const std::vector<Rate>& yields,
                  const DayCounter& dayCounter = Actual365Fixed());
        // inspectors
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter dayCounter() const { return dayCounter_; }
        #endif
        Calendar calendar() const;
        const std::vector<Date>& dates() const;
        Date maxDate() const;
        const std::vector<Time>& times() const;
        Time maxTime() const;
      protected:
        Rate zeroYieldImpl(Time t) const;
      private:
        std::vector<Date> dates_;
        std::vector<Rate> yields_;
        DayCounter dayCounter_;
        std::vector<Time> times_;
        Interpolation interpolation_;
    };

    // inline definitions

    inline Calendar ZeroCurve::calendar() const {
        return Calendar();
    }

    inline const std::vector<Date>& ZeroCurve::dates() const {
        return dates_;
    }

    inline Date ZeroCurve::maxDate() const {
        return dates_.back();
    }

    inline const std::vector<Time>& ZeroCurve::times() const {
        return times_;
    }

    inline Time ZeroCurve::maxTime() const {
        return times_.back();
    }

}


#endif
