/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file randomdefaultmodel.hpp
  \brief Provide random default time scenarios for a pool of credit names
*/

#ifndef quantlib_randomdefaultmodel_hpp
#define quantlib_randomdefaultmodel_hpp


#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/onefactorcopula.hpp>

using namespace std;
using namespace QuantLib;

namespace QuantLib {
    /*!
      Base class for random default models
      Provide sequences of random default times for each name in the pool.
    */
    class RandomDefaultModel {
    public:
        RandomDefaultModel(boost::shared_ptr<Pool> pool)
            : pool_(pool) {}
        virtual ~RandomDefaultModel() {}
        /*!
          Generate a sequence of random default times, one for each name in the
          pool, and store the result in the Pool using method setTime(name).
          tmax denotes the maximum relevant time- default times > tmax are not
          computed but set to tmax + 1 instead to save coputation time. 
         */
        virtual void nextSequence(Real tmax = QL_MAX_REAL) = 0;
    protected:
        boost::shared_ptr<Pool> pool_;
    };

    // Utility for the numerical solver
    class Root {
    public:
        Root(const Handle<DefaultProbabilityTermStructure> dts, Real pd) 
            : dts_(dts), pd_(pd) {}
        Real operator()(Real t) const {
            QL_REQUIRE (t >= 0.0, "t < 0");
            return dts_->defaultProbability(t, true) - pd_;
        }
    private:
        const Handle<DefaultProbabilityTermStructure> dts_;
        Real pd_;
    };

    /*!
      Random default times using a one-factor Gaussian copula.
    */
    class GaussianRandomDefaultModel : public RandomDefaultModel {
    public:
        GaussianRandomDefaultModel(boost::shared_ptr<Pool> pool, 
                                   Handle<OneFactorCopula> copula,
                                   Real accuracy, long seed);
        void nextSequence(Real tmax = QL_MAX_REAL);
    private:
        Handle<OneFactorCopula> copula_;
        Real accuracy_;
        long seed_;
        PseudoRandom::rsg_type rsg_;
    };
    
}

#endif
