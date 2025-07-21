/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/methods/lattices/binomialtree.hpp>
#include <ql/math/distributions/binomialdistribution.hpp>
#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    JarrowRudd::JarrowRudd(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree<JarrowRudd>(process, end, steps) {
        // drift removed
        up_ = process->stdDeviation(0.0, x0_, dt_);
    }


    CoxRossRubinstein::CoxRossRubinstein(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualJumpsBinomialTree<CoxRossRubinstein>(process, end, steps) {

        dx_ = process->stdDeviation(0.0, x0_, dt_);
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    AdditiveEQPBinomialTree::AdditiveEQPBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree<AdditiveEQPBinomialTree>(process,
                                                              end, steps) {
        up_ = - 0.5 * driftPerStep_ + 0.5 *
            std::sqrt(4.0*process->variance(0.0, x0_, dt_)-
                      3.0*driftPerStep_*driftPerStep_);
    }


    Trigeorgis::Trigeorgis(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualJumpsBinomialTree<Trigeorgis>(process, end, steps) {

        dx_ = std::sqrt(process->variance(0.0, x0_, dt_)+
                        driftPerStep_*driftPerStep_);
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Tian::Tian(const ext::shared_ptr<StochasticProcess1D>& process,
               Time end, Size steps, Real)
    : BinomialTree<Tian>(process, end, steps) {

        Real q = std::exp(process->variance(0.0, x0_, dt_));
        Real r = std::exp(driftPerStep_)*std::sqrt(q);

        up_ = 0.5 * r * q * (q + 1 + std::sqrt(q * q + 2 * q - 3));
        down_ = 0.5 * r * q * (q + 1 - std::sqrt(q * q + 2 * q - 3));

        pu_ = (r - down_) / (up_ - down_);
        pd_ = 1.0 - pu_;

        // doesn't work
        //     treeCentering_ = (up_+down_)/2.0;
        //     up_ = up_-treeCentering_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    LeisenReimer::LeisenReimer(const ext::shared_ptr<StochasticProcess1D>& process,
                               Time end,
                               Size steps,
                               Real strike)
    : BinomialTree<LeisenReimer>(process, end, ((steps % 2) != 0U ? steps : (steps + 1))) {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        Size oddSteps = ((steps % 2) != 0U ? steps : (steps + 1));
        Real variance = process->variance(0.0, x0_, end);
        Real ermqdt = std::exp(driftPerStep_ + 0.5*variance/oddSteps);
        Real d2 = (std::log(x0_/strike) + driftPerStep_*oddSteps ) /
                                                          std::sqrt(variance);
        pu_ = PeizerPrattMethod2Inversion(d2, oddSteps);
        pd_ = 1.0 - pu_;
        Real pdash = PeizerPrattMethod2Inversion(d2+std::sqrt(variance),
                                                 oddSteps);
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);
    }

    Real Joshi4::computeUpProb(Real k, Real dj) const {
        Real alpha = dj/(std::sqrt(8.0));
        Real alpha2 = alpha*alpha;
        Real alpha3 = alpha*alpha2;
        Real alpha5 = alpha3*alpha2;
        Real alpha7 = alpha5*alpha2;
        Real beta = -0.375*alpha-alpha3;
        Real gamma = (5.0/6.0)*alpha5 + (13.0/12.0)*alpha3
            +(25.0/128.0)*alpha;
        Real delta = -0.1025 *alpha- 0.9285 *alpha3
            -1.43 *alpha5 -0.5 *alpha7;
        Real p =0.5;
        Real rootk = std::sqrt(k);
        p+= alpha/rootk;
        p+= beta /(k*rootk);
        p+= gamma/(k*k*rootk);
        // delete next line to get results for j three tree
        p+= delta/(k*k*k*rootk);
        return p;
    }

    Joshi4::Joshi4(const ext::shared_ptr<StochasticProcess1D>& process,
                   Time end,
                   Size steps,
                   Real strike)
    : BinomialTree<Joshi4>(process, end, (steps % 2) != 0U ? steps : (steps + 1)) {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        Size oddSteps = (steps % 2) != 0U ? steps : (steps + 1);
        Real variance = process->variance(0.0, x0_, end);
        Real ermqdt = std::exp(driftPerStep_ + 0.5*variance/oddSteps);
        Real d2 = (std::log(x0_/strike) + driftPerStep_*oddSteps ) /
                                                          std::sqrt(variance);
        pu_ = computeUpProb((oddSteps-1.0)/2.0,d2 );
        pd_ = 1.0 - pu_;
        Real pdash = computeUpProb((oddSteps-1.0)/2.0,d2+std::sqrt(variance));
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);
    }
}
