/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file onefactormodel.hpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/InterestRateModelling/%onefactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_one_factor_model_h
#define quantlib_interest_rate_modelling_one_factor_model_h

#include <ql/InterestRateModelling/model.hpp>
#include <ql/InterestRateModelling/shortrateprocess.hpp>
#include <ql/Lattices/trinomialtree.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class OneFactorModel : public Model {
          public:
            OneFactorModel(
                Size nParameters,
                const RelinkableHandle<TermStructure>& termStructure);
            virtual ~OneFactorModel() {}

            virtual Handle<ShortRateProcess> process() const = 0;

            virtual Handle<Lattices::Tree> tree(const TimeGrid& grid) const {
                return Handle<Lattices::Tree>(
                    new OwnTrinomialTree(process(), grid));
            }

          protected:

            class OwnTrinomialTree : public Lattices::TrinomialTree {
              public:
                OwnTrinomialTree(const Handle<ShortRateProcess>& process,
                                 const TimeGrid& timeGrid) 
                : Lattices::TrinomialTree(process, timeGrid), process_(process) {}

                OwnTrinomialTree(
                    const Handle<ShortRateProcess>& process,
                    const Handle<TermStructureFittingParameter::NumericalImpl>& 
                          theta,
                    const TimeGrid& timeGrid);

                int findCentralNode(Size i, int j, double avg) const {
                    return (int)floor(avg/dx(i+1) + 0.5);
                }

                virtual DiscountFactor discount(Size i, int j) const {
                     Rate r = process_->shortRate(t(i), j*dx(i));
                     return QL_EXP(-r*dt(i));
                }
              protected:
                Handle<ShortRateProcess> process_;
              private:
                class Helper;
            };

          private:
            class StandardConstraint;
        };

    }

}
#endif
