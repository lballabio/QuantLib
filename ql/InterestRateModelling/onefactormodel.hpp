
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
#include <ql/InterestRateModelling/trinomialtree.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class OneFactorModel : public Model {
          public:
            OneFactorModel(
                Size nParams,
                const RelinkableHandle<TermStructure>& termStructure)
            : Model(nParams, OneFactor, termStructure) {}
            virtual ~OneFactorModel() {}
            virtual double minStateVariable() const { return -QL_MAX_DOUBLE;}
            virtual double maxStateVariable() const { return QL_MAX_DOUBLE;}

            const Handle<ShortRateProcess>& process() const {
                return process_;
            }

            virtual Handle<Lattices::Tree> tree(
                const Lattices::TimeGrid& timeGrid) const {
                return Handle<Lattices::Tree>(
                    new TrinomialTree(process(), timeGrid));
            }

            //!Used by Jamshidian's decomposition pricing
            virtual double discountBond(Time now, Time maturity, Rate r) {
                return Null<double>();
            }

          protected:
            Handle<ShortRateProcess> process_;

          private:
            class FitFunction;
            friend class FitFunction;
        };

    }

}
#endif
