

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
/*! \file coxingersollrossplus.hpp
    \brief CIR++ model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%coxingersollrossplus.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_cox_ingersoll_ross_plus_h
#define quantlib_one_factor_models_cox_ingersoll_ross_plus_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class CoxIngersollRossPlus : public OneFactorModel {
          public:
            CoxIngersollRossPlus(
                const RelinkableHandle<TermStructure>& termStructure);
            virtual ~CoxIngersollRossPlus() {}

            virtual double phi(Time t) const;

            virtual bool hasDiscountBondFormula() { return true; }
            virtual double discountBond(Time T, Time s, Rate r);

            virtual std::string name() { return "CIR++"; }

          private:
            double A(Time t, Time T) const;
            double B(Time t, Time T) const;
            double C(Time t, Time T) const;

            class Process;
            friend class Process;

            const double& k_;
            const double& theta_;
            const double& sigma_;
        };

    }

}

#endif
