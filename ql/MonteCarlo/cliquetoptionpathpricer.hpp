
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cliquetoptionpathpricer.hpp
    \brief path pricer for cliquet options
*/

#ifndef quantlib_montecarlo_cliquetoption_path_pricer_h
#define quantlib_montecarlo_cliquetoption_path_pricer_h

#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>

namespace QuantLib {

    //! %path pricer for cliquet options
    class CliquetOptionPathPricer : public PathPricer<Path> {
      public:
        CliquetOptionPathPricer(
                           Option::Type type,
                           double underlying,
                           double moneyness,
                           double accruedCoupon,
                           double lastFixing,
                           double localCap,
                           double localFloor,
                           double globalCap,
                           double globalFloor,
                           bool redemptionOnly,
                           const RelinkableHandle<TermStructure>& riskFreeTS);
        double operator()(const Path& path) const;
      private:
        Option::Type type_;
        double underlying_, moneyness_, accruedCoupon_;
        double lastFixing_, localCap_, localFloor_, globalCap_, globalFloor_;
        bool redemptionOnly_;
        RelinkableHandle<TermStructure> riskFreeTS_;
    };



    //! %path pricer for cliquet options
    /*! %path pricer for cliquet options
      \deprecated use CliquetOptionPathPricer instead
    */
    class CliquetOptionPathPricer_old : public PathPricer_old<Path> {
      public:
        CliquetOptionPathPricer_old(
                                Option::Type type,
                                double underlying,
                                double moneyness,
                                double accruedCoupon,
                                double lastFixing,
                                double localCap,
                                double localFloor,
                                double globalCap,
                                double globalFloor,
                                const std::vector<DiscountFactor>& discounts,
                                bool redemptionOnly,
                                bool useAntitheticVariance);
        double operator()(const Path& path) const;
      private:
        Option::Type type_;
        double underlying_, moneyness_, accruedCoupon_;
        double lastFixing_, localCap_, localFloor_, globalCap_, globalFloor_;
        std::vector<DiscountFactor> discounts_;
        bool redemptionOnly_;
    };

}


#endif
