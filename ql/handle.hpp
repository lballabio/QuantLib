
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file handle.hpp
    \brief Reference-counted pointer
*/

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include <ql/errors.hpp>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

/*! \def Handle
    \deprecated This define will disappear in next release
*/
#define Handle boost::shared_ptr

namespace QuantLib {

    /*! \deprecated shared_ptrs can be tested for null by using them
                    in a boolean context, as in:
                    \code
                    boost::shared_ptr<Foo> ptr = ...;
                    if (ptr)
                        ...
                    if (!ptr)
                        ...
                    \endcode
    */
    template <class T>
    bool IsNull(const boost::shared_ptr<T>&);

    template <class T>
    inline bool IsNull(const boost::shared_ptr<T>& p) {
        return !p;
    }

}


#endif
