
/*
  Copyright (C) 2000, 2001, 2002 RiskMap srl

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it under the
  terms of the QuantLib license.  You should have received a copy of the
  license along with this program; if not, please email ferdinando@ametrano.net
  The license is also available online at http://quantlib.org/html/license.html

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file discountcurve.hpp
  \brief pre-bootstrapped discount factor structure

  \fullpath
  ql/TermStructures/%discountcurve.hpp
*/

// $Id$

#ifndef quantlib_discount_curve_h
#define quantlib_discount_curve_h

#include <ql/termstructure.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/loglinearinterpolation.hpp>

namespace QuantLib
{

   using Math::LogLinearInterpolation;

   namespace TermStructures
   {

      class DiscountCurve:public DiscountStructure
      {
       public:
         // constructor
         DiscountCurve(const Date & todaysDate,
                       const Calendar & calendar,
                       int settlementDays,
                       const DayCounter & dayCounter,
                       Currency currency,
                       const std::vector < Date > &dates,
                       const std::vector < DiscountFactor > &discounts);

         Date todaysDate() const;
         Date settlementDate() const;
         int settlementDays() const;
         Calendar calendar() const;
         DayCounter dayCounter() const;
         Currency currency() const;
         Date minDate() const;
         Date maxDate() const;
         Time minTime() const;
         Time maxTime() const;
         DiscountFactor discountImpl(Time t, bool extrapolate = false) const;
         Rate forwardImpl(Time t, bool extrapolate = false) const;
         Rate zeroYieldImpl(Time t, bool extrapolate = false) const;
       private:
         const Date todaysDate_;
         const Calendar calendar_;
         int settlementDays_;
         const DayCounter dayCounter_;
         Currency currency_;
         const std::vector < Date > &dates_;
         const std::vector < DiscountFactor > discounts_;
	 std::vector < Time > times_;
         typedef LogLinearInterpolation <
            std::vector < Time >::const_iterator,
            std::vector < double >::const_iterator > DfInterpolation;
	 Handle < DfInterpolation > interpolation_;
      };

      // inline definitions

      inline Date DiscountCurve::todaysDate() const
      {
         return todaysDate_;
      }
      inline Date DiscountCurve::settlementDate() const
      {
         return calendar_.advance(todaysDate_, settlementDays_, Days);
      }
      inline int DiscountCurve::settlementDays() const
      {
         return settlementDays_;
      }
      inline Calendar DiscountCurve::calendar() const
      {
         return calendar_;
      }
      inline DayCounter DiscountCurve::dayCounter() const
      {
         return dayCounter_;
      }
      inline Currency DiscountCurve::currency() const
      {
         return currency_;
      }
      inline Date DiscountCurve::minDate() const
      {
         return settlementDate();
      }
      inline Date DiscountCurve::maxDate() const
      {
         return dates_.back();
      }
      inline Time DiscountCurve::minTime() const
      {
         return 0.0;
      }
      inline Time DiscountCurve::maxTime() const
      {
         return times_.back();
      }
   }
}
#endif
