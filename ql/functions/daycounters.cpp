

/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file daycounters.cpp
    \brief day counters functions

    \fullpath
    ql/functions/%daycounters.cpp
*/

// $Id$

#include <ql/functions/daycounters.hpp>

using QuantLib::DayCounter;
using QuantLib::DayCounters::Actual360;
using QuantLib::DayCounters::Actual365;
using QuantLib::DayCounters::ActualActual;
using QuantLib::DayCounters::Thirty360;

namespace QuantLib {

    namespace Functions {
		int accrualDays(Date d1, Date d2, DayCounter::Type type) {
			int result;
			DayCounter dc = Actual365();
			switch(type) {
			  case DayCounter::Actual365:
				  dc = Actual365();
				  result = dc.dayCount(d1, d2);
				  break;
			  case DayCounter::Actual360:
				  dc = Actual360();
				  result = dc.dayCount(d1, d2);
				  break;
			  case DayCounter::ActActEuro:
				  dc = ActualActual(ActualActual::Euro);
				  result = dc.dayCount(d1, d2);
  				  break;
			  case DayCounter::Thirty360:
			  case DayCounter::Thirty360ISDA:
                  dc = Thirty360(Thirty360::USA);
				  result = dc.dayCount(d1, d2);
  				  break;
			  case DayCounter::ActActBond:
				  dc = ActualActual(ActualActual::Bond);
				  result = dc.dayCount(d1, d2);
  				  break;
			  case DayCounter::ActActISDA:
				  dc = ActualActual(ActualActual::ISDA);
				  result = dc.dayCount(d1, d2);
  				  break;
			  case DayCounter::ThirtyE360:
			  case DayCounter::ThirtyE360ISDA:
				  dc = Thirty360(Thirty360::European);
				  result = dc.dayCount(d1, d2);
  				  break;
			  case DayCounter::ThirtyEplus360:
				  dc = Thirty360(Thirty360::Italian);
				  result = dc.dayCount(d1, d2);
  				  break;
			  default:
				  throw IllegalArgumentError(
					  "accrualDays: unknown DayCounter::Type");
			}
			return result;
		}


		double accrualFactor(Date d1, Date d2, DayCounter::Type type) {
			double result;
			DayCounter dc = Actual365();
			switch(type) {
			  case DayCounter::Actual365:
				  dc = Actual365();
				  result = dc.yearFraction(d1, d2);
				  break;
			  case DayCounter::Actual360:
				  dc = Actual360();
				  result = dc.yearFraction(d1, d2);
				  break;
			  case DayCounter::ActActEuro:
				  dc = ActualActual(ActualActual::Euro);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  case DayCounter::Thirty360:
			  case DayCounter::Thirty360ISDA:
                  dc = Thirty360(Thirty360::USA);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  case DayCounter::ActActBond:
				  dc = ActualActual(ActualActual::Bond);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  case DayCounter::ActActISDA:
				  dc = ActualActual(ActualActual::ISDA);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  case DayCounter::ThirtyE360:
			  case DayCounter::ThirtyE360ISDA:
				  dc = Thirty360(Thirty360::European);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  case DayCounter::ThirtyEplus360:
				  dc = Thirty360(Thirty360::Italian);
				  result = dc.yearFraction(d1, d2);
  				  break;
			  default:
				  throw IllegalArgumentError(
					  "accrualFactor: unknown DayCounter::Type");
			}
			return result;
		}
    }

}
