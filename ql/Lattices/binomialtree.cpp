
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Math/binomialdistribution.hpp>

namespace QuantLib {

    BinomialTree::BinomialTree(
                           const boost::shared_ptr<StochasticProcess>& process,
                           Time end, Size steps)
    : Tree(steps+1) {

        x0_ = process->x0();
        dt_ = end/steps;
        driftPerStep_ = process->drift(0.0, x0_) * dt_;
    }


    Real EqualJumpsBinomialTree::probability(Size, Size,
                                             Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


    Real EqualJumpsBinomialTree::underlying(Size i, Size index) const {
        BigInteger j = 2*BigInteger(index) - BigInteger(i);
        // exploiting equal jump and the x0_ tree centering
        return x0_*QL_EXP(j*dx_);
    }

    Real EqualProbabilitiesBinomialTree::underlying(Size i,
                                                    Size index) const {
        BigInteger j = 2*BigInteger(index) - BigInteger(i);
        // exploiting the forward value tree centering
        return x0_*QL_EXP(i*driftPerStep_ + j*up_);
    }


    JarrowRudd::JarrowRudd(const boost::shared_ptr<StochasticProcess>& process,
                           Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree(process, end, steps) {

        // drift removed
        up_ = QL_SQRT(process->variance(0.0, x0_, dt_));
    }

    AdditiveEQPBinomialTree::AdditiveEQPBinomialTree(
                          const boost::shared_ptr<StochasticProcess>& process, 
                          Time end, Size steps, Real)
    : EqualProbabilitiesBinomialTree(process, end, steps) {

        up_ = - 0.5 * driftPerStep_ + 0.5 *
            QL_SQRT(4.0*process->variance(0.0, x0_, dt_)-
                    3.0*driftPerStep_*driftPerStep_);
    }




    CoxRossRubinstein::CoxRossRubinstein(
                          const boost::shared_ptr<StochasticProcess>& process, 
                          Time end, Size steps, Real)
    : EqualJumpsBinomialTree(process, end, steps) {

        dx_ = QL_SQRT(process->variance(0.0, x0_, dt_));
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Trigeorgis::Trigeorgis(
                         const boost::shared_ptr<StochasticProcess>& process, 
                         Time end, Size steps, Real)
    : EqualJumpsBinomialTree(process, end, steps) {

        dx_ = QL_SQRT(process->variance(0.0, x0_, dt_)+
                      driftPerStep_*driftPerStep_);
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Tian::Tian(const boost::shared_ptr<StochasticProcess>& process, 
               Time end, Size steps, Real)
    : BinomialTree(process, end, steps) {

        Real q = QL_EXP(process->variance(0.0, x0_, dt_));
        Real r = QL_EXP(driftPerStep_)*QL_SQRT(q);

        up_ = 0.5 * r * q * (q + 1 + QL_SQRT(q * q + 2 * q - 3));
        down_ = 0.5 * r * q * (q + 1 - QL_SQRT(q * q + 2 * q - 3));

        pu_ = (r - down_) / (up_ - down_);
        pd_ = 1.0 - pu_;

        // doesn't work
        //     treeCentering_ = (up_+down_)/2.0;
        //     up_ = up_-treeCentering_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }

    Real Tian::underlying(Size i, Size index) const {
        return x0_ * QL_POW(down_, Real(BigInteger(i)-BigInteger(index)))
                   * QL_POW(up_, Real(index));
    }

    Real Tian::probability(Size, Size, Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


    LeisenReimer::LeisenReimer(
                           const boost::shared_ptr<StochasticProcess>& process,
                           Time end, Size steps, Real strike)
    : BinomialTree(process, end, (steps%2 ? steps : steps+1)) {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        Size oddSteps = (steps%2 ? steps : steps+1);
        Real variance = process->variance(0.0, x0_, end);
        Real ermqdt = QL_EXP(driftPerStep_ + 0.5*variance/oddSteps);
        Real d2 = (QL_LOG(x0_/strike) + driftPerStep_*oddSteps ) /
                                                            QL_SQRT(variance);
        pu_ = PeizerPrattMethod2Inversion(d2, oddSteps);
        pd_ = 1.0 - pu_;
        Real pdash = PeizerPrattMethod2Inversion(d2+QL_SQRT(variance),
                                                 oddSteps);
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);

    }

    Real LeisenReimer::underlying(Size i, Size index) const {
        return x0_ * QL_POW(down_, Real(BigInteger(i)-BigInteger(index)))
                   * QL_POW(up_, Real(index));
    }

    Real LeisenReimer::probability(Size, Size, Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


}

