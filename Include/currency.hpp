
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file currency.hpp
    \brief Abstract currency class
*/

/*! \namespace QuantLib::Currencies
    \brief Concrete implementations of the Currency interface
*/

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include "qldefines.hpp"
#include "calendar.hpp"
#include "date.hpp"
#include "handle.hpp"
#include "null.hpp"
#include <string>

namespace QuantLib {

    //! Abstract currency class
    /*! This class is purely abstract and defines the interface of concrete
        currency classes which will be derived from this one.

        It provides methods for determining a number of market conventions
        which vary depending on the used currency.
    */
    class Currency {
      public:
        //! Returns the name of the currency.
        /*! \warning This method is used for output and comparison between
            currencies.
            It is <b>not</b> meant to be used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        //! \name Settlement conventions
        //@{
        //! Returns the calendar upon which the settlement days are calculated.
        virtual Handle<Calendar> settlementCalendar() const = 0;
        //! Returns the number of settlement days.
        virtual int settlementDays() const = 0;
        //! Returns the settlement date relative to a given actual date.
        Date settlementDate(const Date&) const;
        //@}
        // conventions for deposits or any other rates can be added
    };

    // comparison based on name

    bool operator==(const Handle<Currency>&, const Handle<Currency>&);
    bool operator!=(const Handle<Currency>&, const Handle<Currency>&);

    // inline definitions

    inline Date Currency::settlementDate(const Date& d) const {
        return settlementCalendar()->advance(d,settlementDays(),Days);
    }

    /*! Returns <tt>true</tt> iff the two currencies belong to the same derived
        class.
        \relates Currency
    */
    inline bool operator==(const Handle<Currency>& c1,
        const Handle<Currency>& c2) {
            return (c1->name() == c2->name());
    }

    /*! \relates Currency */
    inline bool operator!=(const Handle<Currency>& c1,
        const Handle<Currency>& c2) {
            return (c1->name() != c2->name());
    }

}


#endif
