
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file scheduler.hpp

    \fullpath
    Include/ql/%scheduler.hpp
    \brief date scheduler

*/

// $Id$
//  $Log$
//  Revision 1.1  2001/09/03 13:54:20  nando
//  source (*.hpp and *.cpp) moved under topdir/ql
//
//  Revision 1.12  2001/08/31 15:23:45  sigmud
//  refining fullpath entries for doxygen documentation
//
//  Revision 1.11  2001/08/09 14:59:45  sigmud
//  header modification
//
//  Revision 1.10  2001/08/08 11:07:48  sigmud
//  inserting \fullpath for doxygen
//
//  Revision 1.9  2001/08/07 11:25:53  sigmud
//  copyright header maintenance
//
//  Revision 1.8  2001/07/25 15:47:27  sigmud
//  Change from quantlib.sourceforge.net to quantlib.org
//
//  Revision 1.7  2001/07/19 14:27:27  sigmud
//  warnings purged
//
//  Revision 1.6  2001/07/13 14:39:52  nando
//  warning pruning action ....
//
//  Revision 1.5  2001/07/13 14:29:07  sigmud
//  removed a few gcc compile warnings
//
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
        unsigned int size() const { return dates_.size(); }
        const Date& date(int i) const;
        bool isRegular(unsigned int i) const;
        // iterators
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
      private:
        Handle<Calendar> calendar_;
        Date startDate_, endDate_;
        int frequency_;
        RollingConvention rollingConvention_;
        bool isAdjusted_;
        Date stubDate_;
        bool lastIsRegular_;
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
