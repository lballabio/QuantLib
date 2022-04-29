/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_duffs_device_inner_product_hpp
#define quantlib_duffs_device_inner_product_hpp

#include <ql/qldefines.hpp>
#include <numeric>

namespace QuantLib {

  namespace dsd {

    //! inner_product implementation using Duff's device
    /*! See http://www.lysator.liu.se/c/duffs-device.html
        and http://en.wikipedia.org/wiki/Duff's_device

        \todo investigate the possible advantages of using
              boost implementation, see:
              <boost/numeric/ublas/detail/duff.hpp>
              <boost/numeric/ublas/functional.hpp>
              <boostnumeric/ublas/detail/matrix_assign.hpp>
              <boost/numeric/ublas/detail/vector_assign.hpp>

    \deprecated To be removed as unused.
                Copy it in your codebase if you need it.
                Deprecated in version 1.27.

    */
    template <class InputIterator1, class InputIterator2, class T>
    QL_DEPRECATED
    inline T inner_product(InputIterator1 f1, InputIterator1 l1,
                           InputIterator2 f2, T init) {

        switch ((l1-f1) % 8) {
        case  0: while (f1 != l1) { init = init + *f1 * *f2; ++f1; ++f2;
        case  7:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  6:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  5:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  4:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  3:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  2:                    init = init + *f1 * *f2; ++f1; ++f2;
        case  1:                    init = init + *f1 * *f2; ++f1; ++f2;
                 }
        }

        // slightly different implementation
        //switch ( (l1-f1) % 8 ) while ( f1 != l1 ) {
        //case  8: init = init + *f1 * *f2; ++f1; ++f2;
        //case  7: init = init + *f1 * *f2; ++f1; ++f2;
        //case  6: init = init + *f1 * *f2; ++f1; ++f2;
        //case  5: init = init + *f1 * *f2; ++f1; ++f2;
        //case  4: init = init + *f1 * *f2; ++f1; ++f2;
        //case  3: init = init + *f1 * *f2; ++f1; ++f2;
        //case  2: init = init + *f1 * *f2; ++f1; ++f2;
        //case  1: init = init + *f1 * *f2; ++f1; ++f2;
        //case  0: ;
        //}

        // the following implementation crashes (probably when l1==f1)
        //switch ((l1-f1) % 8) {
        //case  0: do { init = init + *f1 * *f2; ++f1; ++f2;
        //case  7:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  6:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  5:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  4:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  3:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  2:      init = init + *f1 * *f2; ++f1; ++f2;
        //case  1:      init = init + *f1 * *f2; ++f1; ++f2;
        //      } while (f1 != l1);
        //}

        return init;
    }

  }
}

#endif
