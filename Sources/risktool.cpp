
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file risktool.cpp
    \brief risk tools
    
    $Source$
    $Log$
    Revision 1.2  2001/01/15 13:55:16  lballabio
    Fixed typo in documentation

    Revision 1.1  2001/01/12 17:30:04  nando
    added RiskTool.
    It offres VAR, shortfall, average shortfall methods.
    It still needs test

*/

#include "risktool.h"

namespace QuantLib {

    namespace RiskTool {

        RiskTool::RiskTool(double target) {
            reset(target);
        }
        
        void RiskTool::reset(double target) {
            Math::Statistics::reset();
            target_ = target;
            shortfallCounter_ = 0.0;
            averageShortfall_ = 0.0;
        }
    
        void RiskTool::reset() {
            Math::Statistics::reset();
            target_ = Null<double>();
            shortfallCounter_ = Null<double>();
            averageShortfall_ = Null<double>();
        }
    }
}

