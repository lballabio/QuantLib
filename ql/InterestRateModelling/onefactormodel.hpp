
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
                unsigned nParams,
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
