
/*
 * Copyright (C) 2001
 * QuantLib Group
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

/*! \file ratehelper.h

    $Source$
    $Log$
    Revision 1.1  2001/03/07 10:34:25  nando
    added ratehelper.cpp and ratehelper.h.
    Borland, Linux and Visual C++ updated
    Also added some missing files to Visual C++


*/

#include "ratehelper.h"

namespace QuantLib {

    double DepositRate::value() const {
        Time t = dayCounter_->yearFraction(termStructure_->settlementDate(),
            maturity_);
        Rate impliedRate = (1.0/termStructure_->discount(maturity_)-1.0)/t;
        return rate_-impliedRate;
    }

    double DepositRate::guess() const {
        Time t = dayCounter_->yearFraction(termStructure_->settlementDate(),
            maturity_);
        return 1.0/(1.0+rate_ * t);
    }

    double ForwardRate::value() const {
        return 0.0;
    }

    double ForwardRate::guess() const {
        return 0.0;
    }

    double SwapRate::value() const {
        return 0.0;
    }

    double SwapRate::guess() const {
        return 0.0;
    }

}
