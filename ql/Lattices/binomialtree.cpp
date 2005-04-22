/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Math/binomialdistribution.hpp>

namespace QuantLib {

    JarrowRudd::JarrowRudd(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree<JarrowRudd>(process, end, steps) {
        // drift removed
        up_ = std::sqrt(process->variance(0.0, x0_, dt_));
    }


    CoxRossRubinstein::CoxRossRubinstein(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualJumpsBinomialTree<CoxRossRubinstein>(process, end, steps) {

        dx_ = std::sqrt(process->variance(0.0, x0_, dt_));
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    AdditiveEQPBinomialTree::AdditiveEQPBinomialTree(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree<AdditiveEQPBinomialTree>(process,
                                                              end, steps) {
        up_ = - 0.5 * driftPerStep_ + 0.5 *
            std::sqrt(4.0*process->variance(0.0, x0_, dt_)-
                      3.0*driftPerStep_*driftPerStep_);
    }


    Trigeorgis::Trigeorgis(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real)
    : EqualJumpsBinomialTree<Trigeorgis>(process, end, steps) {

        dx_ = std::sqrt(process->variance(0.0, x0_, dt_)+
                        driftPerStep_*driftPerStep_);
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Tian::Tian(const boost::shared_ptr<StochasticProcess1D>& process,
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


    LeisenReimer::LeisenReimer(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        Time end, Size steps, Real strike)
    : BinomialTree<LeisenReimer>(process, end, (steps%2 ? steps : steps+1)) {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        Size oddSteps = (steps%2 ? steps : steps+1);
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

}

