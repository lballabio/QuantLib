/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

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
/*! \file binomialvanillaengine.hpp
    \brief Plain-option pricer using binomial methods 

    \fullpath
    ql/Pricers/%binomialvanillaengine.hpp
*/

// $Id$

#ifndef quantlib_pricers_binomial_plain_option_h
#define quantlib_pricers_binomial_plain_option_h

#include <ql/numericalmethod.hpp>
#include <ql/Pricers/vanillaoptionengine.hpp>

namespace QuantLib {

    namespace Pricers {

        //! binomial pricing engine for European options
        class BinomialVanillaEngine : public VanillaOptionEngine {
          public:
            enum Type { CoxRossRubinstein, JarrowRudd, LeisenReimer };

            BinomialVanillaEngine(Type type, Size steps)
            : type_(type), steps_(steps) {}
            void calculate() const;
          private:
            Type type_;
            Size steps_;
        };

        class DiscretizedVanillaOption : public DiscretizedAsset {
          public:
            DiscretizedVanillaOption(
                const Handle<NumericalMethod>& method, 
                const VanillaOptionParameters& params)
            : DiscretizedAsset(method), parameters_(params) {}

            void reset(Size size);

            void adjustValues();

            void addTimes(std::list<Time>& times) const {
                times.push_back(parameters_.residualTime);
            }

          private:
            void applySpecificCondition();
            VanillaOptionParameters parameters_;
        };


    }

}


#endif
