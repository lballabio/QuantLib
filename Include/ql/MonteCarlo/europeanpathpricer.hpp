
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

/*! \file europeanpathpricer.hpp
    \brief path pricer for European options

    $Id$
*/

// $Source$
// $Log$
// Revision 1.10  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/08/06 16:49:17  nando
// 1) BSMFunction now is VolatilityFunction
// 2) Introduced ExercisePayoff (to be reworked later)
//
// Revision 1.8  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.7  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.6  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_montecarlo_european_path_pricer_h
#define quantlib_montecarlo_european_path_pricer_h

#include "ql/MonteCarlo/pathpricer.hpp"
#include "ql/options.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        //! path pricer for European options
        class EuropeanPathPricer : public PathPricer {
          public:
            EuropeanPathPricer():PathPricer(){}
            EuropeanPathPricer(Option::Type type, double underlying,
                double strike, double discount);
            double value(const Path &path) const;
          protected:
            mutable Option::Type type_;
            mutable double underlying_, strike_, discount_;
        };

    }

}


#endif
