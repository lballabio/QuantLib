
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file brownianbridge.hpp
    \brief Browian bridge

    \fullpath
    ql/MonteCarlo/%brownianbridge.hpp

*/

// $Id$

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
// which was taken from "Monte Carlo Methods in Finance", by Peter Jäckel.
// Only minor modifications were applied to integrate it in QuantLib
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#ifndef   quantlib_brownian_bridge_h
#define   quantlib_brownian_bridge_h

#include <ql/qldefines.hpp>
#include <ql/errors.hpp>
#include <vector>

namespace QuantLib {

    namespace MonteCarlo {

        //! Builds Wiener process paths of equal time steps of delta t = 1.
        class BrownianBridge {
        public:
            BrownianBridge(unsigned long numberOfSteps);
            void buildPath(std::vector<double>& theWienerProcessPath,
                           const std::vector<double>& gaussianVariates);
        private:
            unsigned long numberOfSteps_;
            std::vector<unsigned long> leftIndex_, rightIndex_, bridgeIndex_;
            std::vector<double> leftWeight_, rightWeight_, sigma_;
        };
    }
}

#endif // quantlib_brownian_bridge_h
