
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

/*! \file singleassetpathpricer.hpp
    \brief generic single asset %path pricer

    \fullpath
    ql/MonteCarlo/%singleassetpathpricer.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_single_asset_path_pricer_h
#define quantlib_montecarlo_single_asset_path_pricer_h

#include "ql/MonteCarlo/pathpricer.hpp"
#include "ql/option.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! generic %path pricer for single asset
        /*! Concrete SingleAssetPathPricers can implement
            antithetic variance control technique
            since for a single factor the antithetic path
            can be deduced from the original path.
            This does not apply to MultiPath.

            Concrete SingleAssetPathPricers that will not
            implement antithetic variance control technique
            should throw an exception 
            if <tt>antitheticVariance==true</tt>.
        */
        class SingleAssetPathPricer : public PathPricer {
          public:
            SingleAssetPathPricer(Option::Type type,
                                  double underlying,
                                  double strike,
                                  double discount,
                                  bool antitheticVariance);
          protected:
            Option::Type type_;
            double underlying_, strike_, discount_;
            bool antitheticVariance_;
        };

    }

}


#endif
