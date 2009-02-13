/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/models/twomixmodel.hpp>

namespace QuantLib {

    TwoBachelierModel::TwoBachelierModel(
                              const Handle<YieldTermStructure>& termStructure,
                              Real lambda, Volatility sig1, Volatility sig2,
                              Volatility  minVol, Volatility maxVol,
                              Real mu, Real minMu, Real maxMu)
    : CalibratedModel(4), termStructure_(termStructure),
      lambda_(lambda), sig1_(sig1), sig2_(sig2),
      minVol_(minVol), maxVol_(maxVol),
      mu_(mu), minMu_(minMu), maxMu_(maxMu) {

        arguments_[0] = ConstantParameter(lambda_,
                                          BoundaryConstraint(0.0, 1.0));
        arguments_[1] = ConstantParameter(sig1_,
                                          BoundaryConstraint(minVol_, maxVol_));
        arguments_[2] = ConstantParameter(sig2_,
                                          BoundaryConstraint(minVol_, maxVol_));
        arguments_[3] = ConstantParameter(mu_,
                                          BoundaryConstraint(minMu_, maxMu_));
    }


    TwoDDModel::TwoDDModel(const Handle<YieldTermStructure>& termStructure,
                           Real lambda, Volatility sig1, Volatility sig2,
                           Volatility  minVol, Volatility maxVol,
                           Real displ1, Real displ2,
                           Real minDispl, Real maxDispl)
    : CalibratedModel(5), termStructure_(termStructure),
      lambda_(lambda), sig1_(sig1), sig2_(sig2),
      minVol_(minVol), maxVol_(maxVol),
      displ1_(displ1), displ2_(displ2),
      minDispl_(minDispl), maxDispl_(maxDispl) {

        arguments_[0] = ConstantParameter(lambda_,
                                          BoundaryConstraint(0.0, 1.0));
        arguments_[1] = ConstantParameter(sig1_,
                                          BoundaryConstraint(minVol_, maxVol_));
        arguments_[2] = ConstantParameter(sig2_,
                                          BoundaryConstraint(minVol_, maxVol_));
        arguments_[3] = ConstantParameter(displ1_,
                                          BoundaryConstraint(minDispl_,
                                                             maxDispl_));
        arguments_[4] = ConstantParameter(displ2_,
                                          BoundaryConstraint(minDispl_,
                                                             maxDispl_));
    }

}

