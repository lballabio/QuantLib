/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/experimental/credit/randomdefaultmodel.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <utility>

using namespace std;

namespace QuantLib {

    namespace {

        // Utility for the numerical solver
        class Root {
          public:
            Root(Handle<DefaultProbabilityTermStructure> dts, Real pd)
            : dts_(std::move(dts)), pd_(pd) {}
            Real operator()(Real t) const {
                QL_REQUIRE(t >= 0.0, "GaussianRandomDefaultModel: internal error, t < 0 ("
                                         << t << ") during root searching.");
                return dts_->defaultProbability(t, true) - pd_;
            }
          private:
            const Handle<DefaultProbabilityTermStructure> dts_;
            Real pd_;
        };

    }

    GaussianRandomDefaultModel::GaussianRandomDefaultModel(
        const std::shared_ptr<Pool>& pool,
        const std::vector<DefaultProbKey>& defaultKeys,
        const Handle<OneFactorCopula>& copula,
        Real accuracy,
        long seed)
    : RandomDefaultModel(pool, defaultKeys), copula_(copula), accuracy_(accuracy), seed_(seed),
      rsg_(PseudoRandom::make_sequence_generator(pool->size() + 1, seed)) {
        registerWith(copula);
    }

    void GaussianRandomDefaultModel::reset() {
        Size dim = pool_->size() + 1;
        rsg_ = PseudoRandom::make_sequence_generator(dim, seed_);
    }

    void GaussianRandomDefaultModel::nextSequence(Real tmax) {
        const std::vector<Real>& values = rsg_.nextSequence().value;
        Real a = sqrt(copula_->correlation());
        for (Size j = 0; j < pool_->size(); j++) {
            const string name = pool_->names()[j];
            const Handle<DefaultProbabilityTermStructure>&
                dts = pool_->get(name).defaultProbability(defaultKeys_[j]);

            Real y = a * values[0] + sqrt(1-a*a) * values[j+1];
            Real p = CumulativeNormalDistribution()(y);

            if (dts->defaultProbability(tmax) < p)
                pool_->setTime(name, tmax + 1);
            else {
                // we know there is a zero of f(t) = dts->defaultProbability(t) - p in [0, tmax]
                try {
                    // try bracketing the root and find it with Brent
                    Brent brent;
                    brent.setLowerBound(0.0);
                    brent.setUpperBound(tmax);
                    pool_->setTime(name, brent.solve(Root(dts, p), accuracy_, tmax / 2.0, 1.0));
                } catch (...) {
                    // if Brent fails, use Bisection, this is guaranteed to find the root
                    pool_->setTime(
                        name, Bisection().solve(Root(dts, p), accuracy_, tmax / 2.0, 0.0, tmax));
                }
            }
        }
    }

}

