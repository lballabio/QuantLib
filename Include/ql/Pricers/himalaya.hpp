
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
/*! \file himalaya.hpp

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_himalaya_h
#define quantlib_himalaya_h

#include "ql/rate.hpp"
#include "ql/date.hpp"
#include "ql/MonteCarlo/multifactorpricer.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Himalayan-type option pricer.
        /*! The payoff of a Himalaya option is computed in the following way:
            Given a basket of N assets, and N time periods, at end of
            each period the option who performed the best is added to the
            average and then discarded from the basket. At the end of the
            N periods the option pays the max between the strike and the
            average of the best performers.
        */
        class Himalaya : public MultiFactorPricer {
        public:
            Himalaya(const Array &underlying,
                const Array &dividendYield,
                const Math::Matrix &covariance,
                Rate riskFreeRate, double strike,
                const std::vector<Time> &timeDelays,
                long samples, long seed = 0);
        };

    }

}


#endif
