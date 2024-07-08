/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bsmprocessesextractor.hpp
    \brief helper class to extract underlying, volatility etc from a vector of processes
*/

#ifndef quantlib_vector_bsm_process_extractor_hpp
#define quantlib_vector_bsm_process_extractor_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <functional>

namespace QuantLib {
    namespace detail {
        class VectorBsmProcessExtractor {
          public:
            VectorBsmProcessExtractor(
                std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p);

            Array getSpot() const;
            Array getBlackVariance(const Date& maturityDate) const;
            Array getDividendYield(const Date& maturityDate) const;
            DiscountFactor getInterestRate(const Date& maturityDate) const;

          private:
            Array extractProcesses(
                const std::function<Real(const ext::shared_ptr<GeneralizedBlackScholesProcess>&)>& f) const;

            const std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes_;
        };
    }
}

#endif
