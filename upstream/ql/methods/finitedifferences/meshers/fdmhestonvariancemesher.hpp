/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2019 Klaus Spanderen

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

/*! \file fdmhestonvariancemesher.hpp
    \brief One-dimensional grid mesher for the variance part of the Heston model
*/

#ifndef quantlib_fdm_heston_variance_mesher_hpp
#define quantlib_fdm_heston_variance_mesher_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>

namespace QuantLib {

    class FdmHestonVarianceMesher : public Fdm1dMesher {
      public:
        FdmHestonVarianceMesher(
            Size size,
            const ext::shared_ptr<HestonProcess> & process,
            Time maturity, Size tAvgSteps = 10, Real epsilon = 0.0001,
            Real mixingFactor = 1.0);

        Real volaEstimate() const { return volaEstimate_; }

      private:
        Real volaEstimate_;
    };


    class LocalVolTermStructure;

    class FdmHestonLocalVolatilityVarianceMesher : public Fdm1dMesher {
      public:
        FdmHestonLocalVolatilityVarianceMesher(
            Size size,
            const ext::shared_ptr<HestonProcess>& process,
            const ext::shared_ptr<LocalVolTermStructure>& leverageFct,
            Time maturity, Size tAvgSteps = 10, Real epsilon = 0.0001,
            Real mixingFactor = 1.0);

        Real volaEstimate() const { return volaEstimate_; }

      private:
        Real volaEstimate_;
    };
}

#endif
