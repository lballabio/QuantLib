
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
                           const boost::shared_ptr<DiffusionProcess>& process,
                           Time end, unsigned long steps)
    : Tree(steps+1) {

        x0_ = process->x0();
        dt_ = end/steps;
        driftPerStep_ = process->drift(0.0, x0_) * dt_;
    }


    double EqualJumpsBinomialTree::probability(Size, Size,
                                               Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


    double EqualJumpsBinomialTree::underlying(Size i, Size index) const {
        long j = 2*long(index) - long(i);
        // exploiting equal jump and the x0_ tree centering
        return x0_*QL_EXP(j*dx_);
    }

    double EqualProbabilitiesBinomialTree::underlying(Size i,
                                                      Size index) const {
        long j = 2*long(index) - long(i);
        // exploiting the forward value tree centering
        return x0_*QL_EXP(i*driftPerStep_ + j*up_);
    }


    JarrowRudd::JarrowRudd(const boost::shared_ptr<DiffusionProcess>& process,
                           Time end, unsigned long steps, double)
    : EqualProbabilitiesBinomialTree(process, end, steps) {

        // drift removed
        up_ = QL_SQRT(process->variance(0.0, x0_, dt_));
    }

    AdditiveEQPBinomialTree::AdditiveEQPBinomialTree(
                          const boost::shared_ptr<DiffusionProcess>& process, 
                          Time end, unsigned long steps, double)
    : EqualProbabilitiesBinomialTree(process, end, steps) {

        up_ = - 0.5 * driftPerStep_ + 0.5 *
            QL_SQRT(4.0*process->variance(0.0, x0_, dt_)-
                    3.0*driftPerStep_*driftPerStep_);
    }




    CoxRossRubinstein::CoxRossRubinstein(
                          const boost::shared_ptr<DiffusionProcess>& process, 
                          Time end, unsigned long steps, double)
    : EqualJumpsBinomialTree(process, end, steps) {

        dx_ = QL_SQRT(process->variance(0.0, x0_, dt_));
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Trigeorgis::Trigeorgis(const boost::shared_ptr<DiffusionProcess>& process, 
                           Time end, unsigned long steps, double)
    : EqualJumpsBinomialTree(process, end, steps) {

        dx_ = QL_SQRT(process->variance(0.0, x0_, dt_)+
                      driftPerStep_*driftPerStep_);
        pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
        pd_ = 1.0 - pu_;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }


    Tian::Tian(const boost::shared_ptr<DiffusionProcess>& process, 
               Time end, unsigned long steps, double)
    : BinomialTree(process, end, steps) {

        double q = QL_EXP(process->variance(0.0, x0_, dt_));
        double r = QL_EXP(driftPerStep_)*QL_SQRT(q);

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

    double Tian::underlying(Size i, Size index) const {
        return x0_ * QL_POW(down_, double(long(i)-long(index)))
                   * QL_POW(up_, double(index));
    }

    double Tian::probability(Size, Size, Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


    LeisenReimer::LeisenReimer(
                           const boost::shared_ptr<DiffusionProcess>& process,
                           Time end, unsigned long steps, double strike)
    : BinomialTree(process, end, (steps%2 ? steps : steps+1)) {

        QL_REQUIRE(strike>0.0, "strike must be positive");
        unsigned long oddSteps = (steps%2 ? steps : steps+1);
        double variance = process->variance(0.0, x0_, end);
        double ermqdt = QL_EXP(driftPerStep_ + 0.5*variance/oddSteps);
        double d2 = (QL_LOG(x0_/strike) + driftPerStep_*oddSteps ) /
                                                            QL_SQRT(variance);
        pu_ = PeizerPrattMethod2Inversion(d2, oddSteps);
        pd_ = 1.0 - pu_;
        double pdash = PeizerPrattMethod2Inversion(d2+QL_SQRT(variance),
                                                   oddSteps);
        up_ = ermqdt * pdash / pu_;
        down_ = (ermqdt - pu_ * up_) / (1.0 - pu_);

    }

    double LeisenReimer::underlying(Size i, Size index) const {
        return x0_ * QL_POW(down_, double(long(i)-long(index)))
                   * QL_POW(up_, double(index));
    }

    double LeisenReimer::probability(Size, Size, Size branch) const {
        if (branch == 1)
            return pu_;
        else
            return pd_;
    }


}

