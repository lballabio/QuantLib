/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file binomialtree.cpp
    \brief Binomial tree class

    \fullpath
    ql/Lattices/%binomialtree.cpp
*/

// $Id$

#include "ql/Lattices/binomialtree.hpp"

namespace QuantLib {

    namespace Lattices {


        EqualProbabilitiesBinomialTree::EqualProbabilitiesBinomialTree(
            const Handle<DiffusionProcess>& process,
            Time end, Size steps)
        : BinomialTree(steps+1) {

            x0_ = process->x0();
            dt_ = end/steps;
            double drift = process->drift(0.0, x0_);
            driftPerStep_ = drift * dt_;
        }

        JarrowRudd::JarrowRudd(const Handle<DiffusionProcess>& process,
            Time end, Size steps)
        : EqualProbabilitiesBinomialTree(process, end, steps) {

            up_ = QL_SQRT(process->variance(0.0, x0_, dt_));
        }

        AdditiveEQPBinomialTree::AdditiveEQPBinomialTree(
            const Handle<DiffusionProcess>& process, Time end, Size steps)
        : EqualProbabilitiesBinomialTree(process, end, steps) {

            up_ = - driftPerStep_/2.0 + 0.5 *
                QL_SQRT(4.0 * process->variance(0.0, x0_, dt_) -
                        3.0 * driftPerStep_ * driftPerStep_);
        }



       
        EqualJumpsBinomialTree::EqualJumpsBinomialTree(
            const Handle<DiffusionProcess>& process, Time end, Size steps)
        : BinomialTree(steps+1) {

            x0_ = process->x0();
            dt_ = end/steps;
            double drift = process->drift(0.0, x0_);
            driftPerStep_ = drift * dt_;
        }

        CoxRossRubinstein::CoxRossRubinstein(
            const Handle<DiffusionProcess>& process, Time end, Size steps)
        : EqualJumpsBinomialTree(process, end, steps) {

            dx_ = QL_SQRT(process->variance(0.0, x0_, dt_));
            pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
            QL_REQUIRE(pu_<=1.0,
                "CoxRossRubinstein::CoxRossRubinstein : "
                "negative probability");
            QL_REQUIRE(pu_>=0.0,
                "CoxRossRubinstein::CoxRossRubinstein : "
                "negative probability");
            pd_ = 1.0 - pu_;
        }


        Trigeorgis::Trigeorgis(const Handle<DiffusionProcess>& process,
            Time end, Size steps)
        : EqualJumpsBinomialTree(process, end, steps) {

            dx_ = QL_SQRT(process->variance(0.0, x0_, dt_)+
                          driftPerStep_*driftPerStep_);
            pu_ = 0.5 + 0.5*driftPerStep_/dx_;;
            QL_REQUIRE(pu_<=1.0,
                "CoxRossRubinstein::CoxRossRubinstein : "
                "negative probability");
            QL_REQUIRE(pu_>=0.0,
                "CoxRossRubinstein::CoxRossRubinstein : "
                "negative probability");
            pd_ = 1.0 - pu_;
        }



    }

}

