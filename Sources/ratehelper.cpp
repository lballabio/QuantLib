
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file ratehelper.cpp

    $Source$
    $Log$
    Revision 1.7  2001/04/09 14:13:33  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.6  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.5  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/03/19 18:39:10  nando
    conflict resolved

    Revision 1.3  2001/03/19 17:52:19  nando
    introduces DepositRate2.
    Later this will superseed DepositRate

    Revision 1.2  2001/03/14 14:03:45  lballabio
    Fixed Doxygen documentation and makefiles

    Revision 1.1  2001/03/07 10:34:25  nando
    added ratehelper.cpp and ratehelper.h.
    Borland, Linux and Visual C++ updated
    Also added some missing files to Visual C++


*/

#include "ql/ratehelper.hpp"


namespace QuantLib {

    double DepositRate2::rateError() const {

//        std::cout << termStructure_->discount(maturity_) << " " << timeToMaturity_ <<
//            " " << impliedRate << " " << rate_ << std::endl;

        Rate impliedRate = (1.0/termStructure_->discount(maturity_)-1.0)/timeToMaturity_;
        return rate_-impliedRate;
    }

    double DepositRate2::discountGuess() const {
        return 1.0/(1.0+rate_ * timeToMaturity_);
    }

    double ForwardRate::rateError() const {
        return 0.0;
    }

    double ForwardRate::discountGuess() const {
        return 0.0;
    }

    double SwapRate::rateError() const {
        return 0.0;
    }

    double SwapRate::discountGuess() const {
        return 0.0;
    }

}
