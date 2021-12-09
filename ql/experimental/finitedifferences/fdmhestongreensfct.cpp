/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file fdmhestongreenfct.cpp
    \brief Heston Fokker-Planck Green's function
*/

#include <ql/experimental/finitedifferences/fdmhestongreensfct.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/squarerootprocessrndcalculator.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <utility>

namespace QuantLib {

    FdmHestonGreensFct::FdmHestonGreensFct(ext::shared_ptr<FdmMesher> mesher,
                                           ext::shared_ptr<HestonProcess> process,
                                           FdmSquareRootFwdOp::TransformationType trafoType_,
                                           const Real l0)
    : l0_(l0), mesher_(std::move(mesher)), process_(std::move(process)), trafoType_(trafoType_) {}

    Disposable<Array> FdmHestonGreensFct::get(Time t, Algorithm algorithm)
    const {
        const Rate r = process_->riskFreeRate()->forwardRate(0, t, Continuous);
        const Rate q = process_->dividendYield()->forwardRate(0,t, Continuous);

        const Real s0    = process_->s0()->value();
        const Real v0    = process_->v0();
        const Real x0    = std::log(s0) + (r-q-0.5*v0*l0_*l0_)*t;

          const Real rho   = process_->rho();
        const Real theta = process_->theta();
        const Real kappa = process_->kappa();
        const Real sigma = process_->sigma();

        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        const FdmLinearOpIterator endIter = layout->end();

        Array p(mesher_->layout()->size());
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            const Real x = mesher_->location(iter, 0);
            const Real v = (trafoType_ != FdmSquareRootFwdOp::Log)
                ? mesher_->location(iter, 1)
                : std::exp(mesher_->location(iter, 1));

            Real retVal;
            switch (algorithm) {
              case ZeroCorrelation:
              {
                const Real sd_x = l0_*std::sqrt(v0*t);
                  const Real p_x = M_1_SQRTPI*M_SQRT1_2/sd_x
                          * std::exp(-0.5*square<Real>()((x - x0)/sd_x));
                  const Real p_v = SquareRootProcessRNDCalculator(
                      v0, kappa, theta, sigma).pdf(v, t);

                  retVal = p_v*p_x;
              }
              break;
              case SemiAnalytical:
                retVal = process_->pdf(x, v, t, 1e-4);
              break;
              case Gaussian:
              {
                const Real sd_x = l0_*std::sqrt(v0*t);
                const Real sd_v = sigma*std::sqrt(v0*t);
                const Real z0 = v0 + kappa*(theta - v0)*t;
                retVal = 1.0/(M_TWOPI*sd_x*sd_v*std::sqrt(1-rho*rho))
                    *std::exp(-(  square<Real>()((x-x0)/sd_x)
                                + square<Real>()((v-z0)/sd_v)
                                - 2*rho*(x-x0)*(v-z0)/(sd_x*sd_v))
                              /(2*(1-rho*rho)) );
              }
              break;
              default:
                QL_FAIL("unknown algorithm");
            }

            switch (trafoType_) {
              case FdmSquareRootFwdOp::Log:
                retVal*=v;
                break;
              case FdmSquareRootFwdOp::Plain:
                break;
              case FdmSquareRootFwdOp::Power:
                retVal*=std::pow(v, 1.0 - 2*kappa*theta/(sigma*sigma));
                break;
              default:
                QL_FAIL("unknown transformation type");
            }

            p[iter.index()] = retVal;
        }

        return p;
    }
}
