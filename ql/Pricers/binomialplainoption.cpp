
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file binomialplainoption.cpp
    \brief analytic pricing engine for European options

    \fullpath
    ql/Pricers/%binomialplainoption.cpp
*/

// $Id$

#include <ql/Pricers/binomialplainoption.hpp>
#include <ql/Lattices/bsmlattice.hpp>

namespace QuantLib {

    namespace Pricers {

        void BinomialPlainOption::calculate() const {
            double v = parameters_.volatility;
            Time t = parameters_.residualTime;
            Rate r = parameters_.riskFreeRate;
            double s0 = parameters_.underlying;

            Handle<Lattices::Tree> tree;
            switch(type_) {
                case CoxRossRubinstein: 
                    tree = Handle<Lattices::Tree>(
                        new Lattices::CoxRossRubinstein(v, r, s0, t, steps_));
                    break;
                case JarrowRudd:
                    tree = Handle<Lattices::Tree>(
                        new Lattices::JarrowRudd(v, r, s0, t, steps_));
                    break;
            }

            Handle<Lattices::Lattice> lattice(
                new Lattices::BlackScholesLattice(tree, r, t, steps_));

            Handle<DiscretizedAsset> option(
            new DiscretizedPlainOption(lattice, parameters_));

            lattice->initialize(option, t);
            lattice->rollback(option, 0.0);
            results_.value = lattice->presentValue(option);
        }

        void DiscretizedPlainOption::reset(Size size) {
            values_ = Array(size, 0.0);
            adjustValues();
        }

        void DiscretizedPlainOption::adjustValues() {
            if (isOnTime(parameters_.residualTime)) {
                applySpecificCondition();
            }
        }

        void DiscretizedPlainOption::applySpecificCondition() {
            Handle<Lattices::BlackScholesLattice> lattice(method());
            QL_REQUIRE(!lattice.isNull(), 
                       "Must use a BlackScholes lattice");
            Handle<Lattices::Tree> tree(lattice->tree());
            Size i = tree->nColumns() - 1;
            double strike = parameters_.strike;

            Array prices(values_.size(), 0.0);
            Size j;
            switch (parameters_.type) {
              case Option::Call:
                for (j=0; j<values_.size(); j++)
                    prices[j] = QL_MAX(tree->underlying(i, j) - strike, 0.0);
                break;
              case Option::Put:
                for (j=0; j<values_.size(); j++)
                    prices[j] = QL_MAX(strike - tree->underlying(i, j), 0.0);
                break;
              case Option::Straddle:
                for (j=0; j<values_.size(); j++)
                    prices[j] = QL_FABS(tree->underlying(i, j) - strike);
                break;
              default:
                throw IllegalArgumentError("invalid option type");
            }
            for (j=0; j<values_.size(); j++) {
                values_[j] = QL_MAX(values_[j], prices[j]);
            }
        }

    }

}

