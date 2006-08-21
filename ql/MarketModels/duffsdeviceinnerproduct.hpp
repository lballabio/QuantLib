/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Peter Jaeckel

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_duffs_device_inner_product_hpp
#define quantlib_duffs_device_inner_product_hpp

#include <numeric>

namespace QuantLib {

  namespace dsd {

    template <class InputIterator1, class InputIterator2, class T>
    inline T inner_product(InputIterator1 first1, InputIterator1 last1,
                           InputIterator2 first2, T init) {
       switch ( (last1-first1) % 8 ) while ( first1 != last1 ) {
       case  8: init = init + *first1 * *first2; ++first1; ++first2;
       case  7: init = init + *first1 * *first2; ++first1; ++first2;
       case  6: init = init + *first1 * *first2; ++first1; ++first2;
       case  5: init = init + *first1 * *first2; ++first1; ++first2;
       case  4: init = init + *first1 * *first2; ++first1; ++first2;
       case  3: init = init + *first1 * *first2; ++first1; ++first2;
       case  2: init = init + *first1 * *first2; ++first1; ++first2;
       case  1: init = init + *first1 * *first2; ++first1; ++first2;
       case  0: ;
       }
       return init;
    }

  }
}

#endif
