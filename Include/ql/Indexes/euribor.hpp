
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

/*! \file euribor.hpp
    \brief %Euribor index

    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_euribor_hpp
#define quantlib_euribor_hpp

#include "ql/qldefines.hpp"
#include "ql/Indexes/xibor.hpp"
#include "ql/Currencies/eur.hpp"
#include "ql/Calendars/target.hpp"
#include "ql/DayCounters/actual360.hpp"

namespace QuantLib {

    namespace Indexes {
        
        //! Euribor index
        class Euribor : public Xibor {
          public:
            Handle<Currency> currency() const {
                return Handle<Currency>(new Currencies::EUR);
            }
            Handle<Calendar> calendar() const {
                return Handle<Calendar>(new Calendars::TARGET);
            }
            bool modifiedFollowing() const { return true; }
            Handle<DayCounter> dayCounter() const {
                return Handle<DayCounter>(new DayCounters::Actual360);
            }
            std::string name() const { return "Euribor"; }
        };
    
    }

}


#endif
