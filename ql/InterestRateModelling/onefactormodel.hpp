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
                Size nParams,
                const RelinkableHandle<TermStructure>& termStructure);
            virtual ~OneFactorModel() {}

            const Handle<ShortRateProcess>& process() const {
                return process_;
            }

            virtual Handle<Lattices::Tree> tree(
                const Lattices::TimeGrid& timeGrid) const;

          protected:
            Handle<ShortRateProcess> process_;

          private:
            class StandardConstraint;
            class PrivateTree;
        };

    }

}
#endif
