/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009 Jose Aparicio

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file randomdefaultmodel.hpp
    \brief Random default-time scenarios for a pool of credit names
*/

#ifndef quantlib_random_default_model_hpp
#define quantlib_random_default_model_hpp

#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/onefactorcopula.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>

namespace QuantLib {

    //! Base class for random default models
    /*! Provides sequences of random default times for each name in the pool. */
    class RandomDefaultModel : public Observer, public Observable {
    public:
      RandomDefaultModel(const ext::shared_ptr<Pool>& pool,
                         const std::vector<DefaultProbKey>& defaultKeys)
      : pool_(pool), defaultKeys_(defaultKeys) {
          // assuming none defaulted this is true.
          QL_REQUIRE(defaultKeys.size() == pool->size(), "Incompatible pool and keys sizes.");
        }
        ~RandomDefaultModel() override = default;
        void update() override { notifyObservers(); }
        /*!
          Generate a sequence of random default times, one for each name in the
          pool, and store the result in the Pool using method setTime(name).
          tmax denotes the maximum relevant time- default times > tmax are not
          computed but set to tmax + 1 instead to save coputation time.
         */
        virtual void nextSequence(Real tmax = QL_MAX_REAL) = 0;
        virtual void reset() = 0;
    protected:
        ext::shared_ptr<Pool> pool_;
        std::vector<DefaultProbKey> defaultKeys_;
    };

    /*!
      Random default times using a one-factor Gaussian copula.
    */
    class GaussianRandomDefaultModel : public RandomDefaultModel {
    public:
      GaussianRandomDefaultModel(const ext::shared_ptr<Pool>& pool,
                                 const std::vector<DefaultProbKey>& defaultKeys,
                                 const Handle<OneFactorCopula>& copula,
                                 Real accuracy,
                                 long seed);
      void nextSequence(Real tmax = QL_MAX_REAL) override;
      void reset() override;

    private:
        Handle<OneFactorCopula> copula_;
        Real accuracy_;
        long seed_;
        PseudoRandom::rsg_type rsg_;
    };

}

#endif
