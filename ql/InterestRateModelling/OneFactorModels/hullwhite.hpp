
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

/*! \file hullwhite.hpp
    \brief Hull & White (HW) model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%hullwhite.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_hull_white_h
#define quantlib_one_factor_models_hull_white_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class HullWhite : public OneFactorModel {
          public:
            HullWhite(const RelinkableHandle<TermStructure>& termStructure);
            virtual ~HullWhite() {}

            virtual double alpha(Time t) const;

            virtual double discountBond(Time T,
                                        Time s,
                                        Rate r);

            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity);

            virtual std::string name() { return "Hull & White"; }

          private:
            inline double B(Time t) const {
                if (a_ == 0.0)
                    return t;
                else
                    return (1.0 - QL_EXP(-a_*t))/a_;
            }
            double lnA(Time T, Time s) const;
            class Process;
            friend class Process;

            const double& a_;
            const double& sigma_;
            std::vector<double> theta_;
            double dt_;
        };

    }

}

#endif
