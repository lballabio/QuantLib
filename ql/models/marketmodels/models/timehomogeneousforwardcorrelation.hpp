/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_time_homogeneous_forward_correlation_hpp
#define quantlib_time_homogeneous_forward_correlation_hpp

#include <ql/models/marketmodels/models/timedependantcorrelationstructure.hpp>
#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class TimeHomogeneousForwardCorrelation : public TimeDependantCorrelationStructure {
      public:
        TimeHomogeneousForwardCorrelation(
                        const Matrix& fwdCorrelation,
                        const std::vector<Time>& rateTimes,
                        Size numberOfFactors);
        const std::vector<Time>& times() const;
        const Matrix& pseudoRoot(Size i) const;
        Size numberOfFactors() const;
        Size numberOfRates() const;
    private:
        Matrix fwdCorrelation_;
        std::vector<Time> rateTimes_, times_;
        Size numberOfFactors_, numberOfRates_;
        std::vector<Matrix> pseudoRoots_;
    };

}

#endif
