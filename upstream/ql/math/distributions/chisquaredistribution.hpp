/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file chisquaredistribution.hpp
    \brief Chi-square (central and non-central) distributions
*/

#ifndef quantlib_chi_square_distribution_hpp
#define quantlib_chi_square_distribution_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    class CumulativeChiSquareDistribution {
      public:
        explicit CumulativeChiSquareDistribution(Real df) : df_(df) {}
        Real operator()(Real x) const;
      private:
        Real df_;
    };

    class NonCentralCumulativeChiSquareDistribution {
      public:
        NonCentralCumulativeChiSquareDistribution(Real df, Real ncp)
        : df_(df), ncp_(ncp) {}
        Real operator()(Real x) const;
      private:
        Real df_, ncp_;
    };

    class NonCentralCumulativeChiSquareSankaranApprox {
      public:
        NonCentralCumulativeChiSquareSankaranApprox(Real df, Real ncp)
        : df_(df), ncp_(ncp) {}
        Real operator()(Real x) const;
      private:
        Real df_, ncp_;
    };

    class InverseNonCentralCumulativeChiSquareDistribution {
      public:
        InverseNonCentralCumulativeChiSquareDistribution(Real df, Real ncp,
                                               Size maxEvaluations=10,
                                               Real accuracy = 1e-8);
        Real operator()(Real x) const;

    private:
        NonCentralCumulativeChiSquareDistribution nonCentralDist_;
        const Real guess_;
        const Size maxEvaluations_;
        const Real accuracy_;
    };

}


#endif
