
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

/*! \file blackkarasinski.hpp
    \brief Black-Karasinski model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%blackkarasinski.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_black_karasinski_h
#define quantlib_one_factor_models_black_karasinski_h

#include <ql/InterestRateModelling/onefactormodel.hpp>
#include <ql/InterestRateModelling/timefunction.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class BlackKarasinski : public OneFactorModel {
          public:
            BlackKarasinski(
                const RelinkableHandle<TermStructure>& termStructure,
                double dt);
            virtual ~BlackKarasinski() {}

            Handle<Lattices::Tree> tree(
                const Lattices::TimeGrid& timeGrid) const;

            virtual std::string name() { return "Black-Karasinski"; }

          private:
            double alpha(Time t) const {
                return (*alpha_)(t);
            }

            class Process;
            friend class Process;

            class PrivateTree;
            friend class PrivateTree;

            class PrivateFunction;
            friend class PrivateFunction;

            Handle<TimeFunction> alpha_;

            const double& a_;
            const double& sigma_;
            double dtMax_;
        };
    }
}

#endif
