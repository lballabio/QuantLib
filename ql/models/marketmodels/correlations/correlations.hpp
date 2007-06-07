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

/*! \file correlations.hpp
    \brief Market Model correlation matrix
*/

#ifndef quantlib_correlations_hpp
#define quantlib_correlations_hpp

#include <ql/math/matrix.hpp>
#include <ql/utilities/disposable.hpp>

namespace QuantLib {

    //! Time homogeneous functional form with:
    //! - L = long term correlation
    //! - beta = exponential decay of correlation between far away forward rates
    Disposable<Matrix> exponentialCorrelations(
                                        const std::vector<Rate>& rateTimes,
                                        Real longTermCorr,
                                        Real beta);

    //! Time homogeneous functional form with:
    //! - L = long term correlation
    //! - beta = exponential decay of correlation between far away forward rates
    //! - gamma = exponent for time to go  
    //! - time = time dependence
    Disposable<Matrix> exponentialCorrelationsTimeDependent(
                                        const std::vector<Rate>& rateTimes,
                                        Real longTermCorr,
                                        Real beta,
                                        Real gamma,
                                        Time time);
}

#endif
