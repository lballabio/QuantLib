
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file index.hpp
    \brief purely virtual base class for indexes

    \fullpath
    ql/%index.hpp
*/

// $Id$

#ifndef quantlib_index_hpp
#define quantlib_index_hpp

#include <ql/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/daycounter.hpp>

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
