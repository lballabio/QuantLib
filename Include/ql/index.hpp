
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

/*! \file index.hpp
    \brief purely virtual base class for indexes

    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_index_hpp
#define quantlib_index_hpp

#include "ql/qldefines.hpp"
#include "ql/date.hpp"
#include "ql/calendar.hpp"
#include "ql/currency.hpp"
#include "ql/daycounter.hpp"
#include "ql/rate.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    //! purely virtual base class for indexes
    class Index {
      public:
        //! \name inspectors
        //@{
        virtual Currency currency() const = 0;
        virtual Handle<Calendar> calendar() const = 0;
        virtual bool isAdjusted() const = 0;
        virtual bool isModifiedFollowing() const = 0;
        virtual Handle<DayCounter> dayCounter() const = 0;
        virtual std::string name() const = 0;
        //@}
        /*! \brief return the fixing at the given date and tenor
            \note any date passed as arguments must be a value date, 
            i.e., the real calendar date advanced by a number of 
            settlement days.
        */
        virtual Rate fixing(const Date& fixingDate, 
            int n, TimeUnit unit) const = 0;
    };

}


#endif
