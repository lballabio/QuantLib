
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

            virtual bool hasDiscountBondFormula() { return true; }
            virtual double discountBond(Time T, Time s, Rate r);

            virtual bool hasDiscountBondOptionFormula() { return true; }
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
        };

    }

}

#endif
