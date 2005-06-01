/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*! \file dividendschedule.hpp
    \brief Schedule of dividend dates
*/

#ifndef quantlib_dividend_schedule_hpp
#define quantlib_dividend_schedule_hpp

#include <ql/date.hpp>
#include <vector>

namespace QuantLib {

    class DividendSchedule {
      public:
        std::vector<Date> dividendDates;
        std::vector<Real> dividends;
    };

}

#if 0

// This is a much better schedule class, that tboafo came up with.  

class Dividend{
public:
    enum Type { Cash, Yield};
    Real amount;
    Type type;
    Date date;
};
 
typedef vector<Dividend> DividendSchedule;

#endif

#endif

