

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
/*! \file blackkarasinski.hpp
    \brief Black-Karasinski model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%blackkarasinski.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_black_karasinski_h
#define quantlib_one_factor_models_black_karasinski_h

#include <ql/InterestRateModelling/timefunction.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class BlackKarasinski : public OneFactorModel {
          public:
            BlackKarasinski(
                const RelinkableHandle<TermStructure>& termStructure);
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
        };
    }
}

#endif
