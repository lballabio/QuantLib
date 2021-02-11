/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 François du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file expcorrelations.hpp
    \brief exponential correlation matrix
*/

#ifndef quantlib_exp_correlations_hpp
#define quantlib_exp_correlations_hpp

#include <ql/math/matrix.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>

namespace QuantLib {

    /*! Exponential correlation
        L = long term correlation
        beta = exponential decay of correlation between far away forward rates
        gamma = exponent for time to go
        t = time dependence
    */
    Disposable<Matrix> exponentialCorrelations(
                                    const std::vector<Time>& rateTimes,
                                    Real longTermCorr = 0.5,
                                    Real beta = 0.2,
                                    Real gamma = 1.0,
                                    Time t = 0.0);

    class ExponentialForwardCorrelation :
        public PiecewiseConstantCorrelation {
      public:
        ExponentialForwardCorrelation(const std::vector<Time>& rateTimes,
                                      Real longTermCorr = 0.5,
                                      Real beta = 0.2,
                                      Real gamma = 1.0,
                                      std::vector<Time> times = std::vector<Time>());
        const std::vector<Time>& times() const override;
        const std::vector<Time>& rateTimes() const override;
        const std::vector<Matrix>& correlations() const override;
        Size numberOfRates() const override;

      private:
        Size numberOfRates_;
        Real longTermCorr_, beta_, gamma_;
        std::vector<Time> rateTimes_, times_;
        std::vector<Matrix> correlations_;
    };
}

#endif
