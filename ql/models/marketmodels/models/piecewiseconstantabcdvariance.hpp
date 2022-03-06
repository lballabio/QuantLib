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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_piecewise_const_abcd_variance_hpp
#define quantlib_piecewise_const_abcd_variance_hpp

#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib {

    class PiecewiseConstantAbcdVariance : public PiecewiseConstantVariance {
      public:
        PiecewiseConstantAbcdVariance(Real a, Real b, Real c, Real d,
                                      Size resetIndex,
                                      const std::vector<Time>& rateTimes);
        const std::vector<Real>& variances() const override;
        const std::vector<Real>& volatilities() const override;
        const std::vector<Time>& rateTimes() const override;
        void getABCD(Real& a, Real& b, Real& c, Real& d) const;
      private:
        std::vector<Real> variances_;
        std::vector<Real> volatilities_;
        std::vector<Time> rateTimes_;
        Real a_;
        Real b_;
        Real c_;
        Real d_;

    };

}

#endif


#ifndef id_eb1f01b0b318ffa835bc5d50230d183b
#define id_eb1f01b0b318ffa835bc5d50230d183b
inline bool test_eb1f01b0b318ffa835bc5d50230d183b(const int* i) {
    return i != nullptr;
}
#endif
