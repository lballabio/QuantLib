/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file blackswaption.cpp
    \brief European swaption calculated using Black formula

    \fullpath
    ql/Pricers/%blackswaption.cpp
*/

// $Id$

#include "ql/Pricers/blackswaption.hpp"

namespace QuantLib {

    namespace Pricers {

        void BlackSwaption::calculate() const {
            Time start = arguments_.floatingResetTimes[0];
            double w;
            if (arguments_.payFixed)
                w = 1.0;
            else 
                w = -1.0;
            results_.value =  arguments_.fixedBPS * 
                BlackModel::formula(arguments_.fairRate,
                                    arguments_.fixedRate, 
                                    model_->volatility()*QL_SQRT(start), w);
        }

    }

}
