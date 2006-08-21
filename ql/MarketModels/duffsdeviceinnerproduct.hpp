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

#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib {

  namespace dsd {

    // This inner_product allows for the second vector to be longer. The
    // hangover elements will be ignored.
    template <class T>
    inline T inner_product(const std::vector <T> &v,
                           const std::vector <T> &w) {
       const typename std::vector<T>::const_iterator &e = v.end();
             typename std::vector<T>::const_iterator  i = v.begin();
             typename std::vector<T>::const_iterator  j = w.begin();
       // Loop unrolling using Duff's device.
       T x = T();
       x = 0;
       switch ( v.size() % 8 ) while ( i != e ) {
       case  8: x += *i * *j; ++i; ++j;
       case  7: x += *i * *j; ++i; ++j;
       case  6: x += *i * *j; ++i; ++j;
       case  5: x += *i * *j; ++i; ++j;
       case  4: x += *i * *j; ++i; ++j;
       case  3: x += *i * *j; ++i; ++j;
       case  2: x += *i * *j; ++i; ++j;
       case  1: x += *i * *j; ++i; ++j;
       case  0: ;
       }
       return x;
    }

    inline Real inner_product(std::vector<Real>::const_iterator  i,
                              const std::vector<Real>::const_iterator &e,
                              Matrix::const_row_iterator  j,
                              Real startupValue) {
       Real x = startupValue;
       switch ( (e-i) % 8 ) while ( i != e ) {
       case  8: x += *i * *j; ++i; ++j;
       case  7: x += *i * *j; ++i; ++j;
       case  6: x += *i * *j; ++i; ++j;
       case  5: x += *i * *j; ++i; ++j;
       case  4: x += *i * *j; ++i; ++j;
       case  3: x += *i * *j; ++i; ++j;
       case  2: x += *i * *j; ++i; ++j;
       case  1: x += *i * *j; ++i; ++j;
       case  0: ;
       }
       return x;
    }

    inline Real inner_product(Matrix::const_row_iterator  i,
                              const Matrix::const_row_iterator &e,
                              Array::const_iterator  j,
                              Real startupValue) {
       Real x = startupValue;
       switch ( (e-i) % 8 ) while ( i != e ) {
       case  8: x += *i * *j; ++i; ++j;
       case  7: x += *i * *j; ++i; ++j;
       case  6: x += *i * *j; ++i; ++j;
       case  5: x += *i * *j; ++i; ++j;
       case  4: x += *i * *j; ++i; ++j;
       case  3: x += *i * *j; ++i; ++j;
       case  2: x += *i * *j; ++i; ++j;
       case  1: x += *i * *j; ++i; ++j;
       case  0: ;
       }
       return x;
    }

    inline Real inner_product(Matrix::const_column_iterator  i,
                              const Matrix::const_column_iterator &e,
                              Matrix::const_row_iterator  j,
                              Real startupValue) {
       Real x = startupValue;
       switch ( (e-i) % 8 ) while ( i != e ) {
       case  8: x += *i * *j; ++i; ++j;
       case  7: x += *i * *j; ++i; ++j;
       case  6: x += *i * *j; ++i; ++j;
       case  5: x += *i * *j; ++i; ++j;
       case  4: x += *i * *j; ++i; ++j;
       case  3: x += *i * *j; ++i; ++j;
       case  2: x += *i * *j; ++i; ++j;
       case  1: x += *i * *j; ++i; ++j;
       case  0: ;
       }
       return x;
    }

  }
}

#endif
