/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_generic_longstaff_schwartz_hpp
#define quantlib_generic_longstaff_schwartz_hpp

#include <ql/methods/montecarlo/nodedata.hpp>

namespace QuantLib {

    /*! Estimates the value of early-exercise rights using the
        Longstaff-Schwartz least-squares regression method.

        Returns the biased estimate of the option value obtained
        by regressing continuation values against basis functions
        along each simulated path.

        \param simulationData collected node data for all paths and
               exercise dates. It must contain n+1 elements
               for n exercise dates:
               - simulationData[0][j] holds the cash flows
                 accumulated up to the first exercise date on the
                 j-th path (only the \c cumulatedCashFlows
                 field is used);
               - simulationData[i+1][j] holds the data at
                 the i-th exercise date on the j-th
                 path.

        \param basisCoefficients output parameter. After the call, it
               contains the regression coefficients for each exercise
               date; its size must equal the number of exercise dates
               \c n.
    */
    Real genericLongstaffSchwartzRegression(
        std::vector<std::vector<NodeData> >& simulationData,
        std::vector<std::vector<Real> >& basisCoefficients);

}


#endif

