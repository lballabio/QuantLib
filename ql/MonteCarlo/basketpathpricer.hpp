
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
/*! \file basketpathpricer.hpp
    \brief multipath pricer for European-type basket option

    \fullpath
    ql/MonteCarlo/%basketpathpricer.hpp

*/

// $Id$

#ifndef quantlib_basket_path_pricer_h
#define quantlib_basket_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/multipath.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! multipath pricer for European-type basket option
        /*! The value of the option at expiration is given by the value
            of the underlying which has best performed.
        */
        class BasketPathPricer : public PathPricer<MultiPath> {
          public:
            BasketPathPricer(Option::Type type,
                             const Array& underlying,
                             double strike,
                             DiscountFactor discount,
                             bool useAntitheticVariance);
            double operator()(const MultiPath& multiPath) const;
          private:
            Option::Type type_;
            Array underlying_;
            double strike_;
        };

    }

}


#endif
