
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file scheduler.hpp
    \brief date scheduler

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.4  2001/07/06 18:24:17  nando
//  slight modifications to avoid VisualStudio warnings
//
//  Revision 1.3  2001/07/02 12:36:17  sigmud
//  pruned redundant header inclusions
//
//  Revision 1.2  2001/06/05 09:35:13  lballabio
//  Updated docs to use Doxygen 1.2.8
//

#ifndef quantlib_scheduler_hpp
#define quantlib_scheduler_hpp

#include "ql/calendar.hpp"
#include <vector>

namespace QuantLib {

    //! %Date scheduler
    class Scheduler {
      public:
        Scheduler(const Handle<Calendar>& calendar,
            const Date& startDate, const Date& endDate,
            int frequency, RollingConvention rollingConvention,
            bool isAdjusted, const Date& stubDate = Date());
        // inspectors
        int size() const { return dates_.size(); }
        const Date& date(int i) const;
        bool isRegular(int i) const;
        // iterators
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
      private:
        Date startDate_, endDate_, stubDate_;
        Handle<Calendar> calendar_;
        int frequency_;
        RollingConvention rollingConvention_;
        bool isAdjusted_, lastIsRegular_;
        std::vector<Date> dates_;
        bool isEndOfMonth(const Date&) const;
    };


    inline const Date& Scheduler::date(int i) const {
        #if defined(QL_DEBUG)
            QL_REQUIRE(i >= 0 && i <= int(dates_.size()),
                "date index out of bounds");
        #endif
        return dates_[i];
    }

}


#endif
