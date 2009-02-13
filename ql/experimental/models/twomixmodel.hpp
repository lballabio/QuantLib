/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file twomixmodel.hpp
    \brief Mixture models with two components
*/

#ifndef quantlib_two_mix_model_hpp
#define quantlib_two_mix_model_hpp

#include <ql/models/model.hpp>

namespace QuantLib {

    //! Two-component mixture model for Normal models.
    /*! References: Brigo and Mercurio, 2006, Interest Rate Models.
                    Kenyon, 2008, Inflation is Normal.

        These use piecewise constant parameters that may not retain
        their meanings between pieces.  I.e. do not assume that sig1
        is the history of sig1.  It is just the value at different
        times.  The reason is that the probability is different
        between sections hence we are in the exponential-scenario
        version of the mixture model (not two scenarios.)
    */
    class TwoBachelierModel : public CalibratedModel {
      public:
        TwoBachelierModel(const Handle<YieldTermStructure>& termStructure,
                          Real labmda, Volatility sig1, Volatility sig2,
                          Volatility  minVol, Volatility maxVol,
                          Real mu, Real minMu, Real maxMu);

        virtual Real lambda(Time t) const {return arguments_[0](t);}
        virtual Volatility sig1(Time t) const {return arguments_[1](t);}
        virtual Volatility sig2(Time t) const {return arguments_[2](t);}
        virtual Real mu(Time t) const {return arguments_[3](t);}

        /*! The nominal term structure is not changing, it just gets
            used for discounting hence it is not a parameter argument.
            N.B. it is not observed because calibration takes too long
            to re-start so.  Users must take care of re-calibration
            explicitly.
        */
        virtual boost::shared_ptr<YieldTermStructure> termStructure() const {
            return termStructure_.currentLink();
        }

      protected:
        const Handle<YieldTermStructure> termStructure_;
        Real lambda_;
        Volatility sig1_, sig2_, minVol_, maxVol_;
        Real mu_, minMu_, maxMu_;
    };


    //! Two-component mixture model for displaced-diffusion models.
    /*! References: Brigo and Mercurio, 2006, Interest Rate Models.
                    Kenyon, 2008, Inflation is Normal.

        These use piecewise constant parameters that may not retain
        their meanings between pieces.  I.e. do not assume that sig1
        is the history of sig1.  It is just the value at different
        times.  The reason is that the probability is different
        between sections hence we are in the exponential-scenario
        version of the mixture model (not two scenarios.)
    */
    class TwoDDModel : public CalibratedModel  {
      public:
        TwoDDModel(const Handle<YieldTermStructure>& termStructure,
                   Real labmda, Volatility sig1, Volatility sig2,
                   Volatility  minVol, Volatility maxVol,
                   Real displ1, Real displ2,
                   Real minDispl, Real maxDispl);

        virtual Real prob(Time t) const {return arguments_[0](t);}
        virtual Volatility sig1(Time t) const {return arguments_[1](t);}
        virtual Volatility sig2(Time t) const {return arguments_[2](t);}
        virtual Real displ1(Time t) const {return arguments_[3](t);}
        virtual Real displ2(Time t) const {return arguments_[4](t);}

        /*! The nominal term structure is not changing, it just gets
            used for discounting hence it is not a parameter argument.
            N.B. it is not observed because calibration takes too long
            to re-start so.  Users must take care of re-calibration
            explicitly.
        */
        virtual boost::shared_ptr<YieldTermStructure> termStructure() const {
            return termStructure_.currentLink();
        }

      protected:
        const Handle<YieldTermStructure> termStructure_;
        Real lambda_;
        Volatility sig1_, sig2_, minVol_, maxVol_;
        Real displ1_, displ2_, minDispl_, maxDispl_;
    };

}

#endif

