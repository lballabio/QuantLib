
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

/*! \file index.hpp
    \brief purely virtual base class for indexes

    \fullpath
    ql/%index.hpp
*/

// $Id$

#ifndef quantlib_index_hpp
#define quantlib_index_hpp

#include "ql/calendar.hpp"
#include "ql/currency.hpp"
#include "ql/daycounter.hpp"

/*! \namespace QuantLib::Indexes
    \brief Concrete implementations of the Index interface

    See sect. \ref indexes
*/

namespace QuantLib {

    //! purely virtual base class for indexes
    class Index {
      public:
        virtual ~Index() {}
        //! Returns the name of the index.
        /*! \warning This method is used for output and comparison between
            indexes.
            It is <b>not</b> meant to be used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        /*! \brief returns the fixing at the given date
            \note any date passed as arguments must be a value date,
            i.e., the real calendar date advanced by a number of
            settlement days.
        */
        virtual Rate fixing(const Date& fixingDate) const = 0;
    };

}


#endif
