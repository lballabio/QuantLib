
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file currency.h
    \brief Abstract currency class

    $Source$
    $Name$
    $Log$
    Revision 1.8  2001/02/09 19:21:09  lballabio
    removed QL_DECLARE_TEMPLATE_SPECIALIZATION macro

    Revision 1.7  2001/01/17 14:37:54  nando
    tabs removed

    Revision 1.6  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

/*! \namespace QuantLib::Currencies
    \brief Concrete implementations of the Currency interface
*/

#ifndef quantlib_currency_h
#define quantlib_currency_h

#include "qldefines.h"
#include "calendar.h"
#include "date.h"
#include "handle.h"
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
        /*!    \warning This method is used for output and comparison between currencies.
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

    QL_TEMPLATE_SPECIALIZATION
    bool operator==(const Handle<Currency>&, const Handle<Currency>&);
    
    QL_TEMPLATE_SPECIALIZATION
    bool operator!=(const Handle<Currency>&, const Handle<Currency>&);


    // inline definitions

    inline Date Currency::settlementDate(const Date& d) const {
        return settlementCalendar()->advance(d,settlementDays(),Days);
    }

    /*! Returns <tt>true</tt> iff the two currencies belong to the same derived class.
        \relates Currency
    */
    QL_TEMPLATE_SPECIALIZATION
    inline bool operator==(const Handle<Currency>& c1, 
        const Handle<Currency>& c2) {
            return (c1->name() == c2->name());
    }

    /*! \relates Currency */
    QL_TEMPLATE_SPECIALIZATION
    inline bool operator!=(const Handle<Currency>& c1, 
        const Handle<Currency>& c2) {
            return (c1->name() != c2->name());
    }

}


#endif
