/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file tenoroptionletvts.hpp
    \brief caplet volatility term structure based on volatility transformation
*/

#ifndef quantlib_tenoroptionletvts_hpp
#define quantlib_tenoroptionletvts_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <utility>


namespace QuantLib {

    class TenorOptionletVTS : public OptionletVolatilityStructure {

      public:
        class CorrelationStructure; // declaration below

      protected:
        class TenorOptionletSmileSection : public SmileSection {
          protected:
            ext::shared_ptr<CorrelationStructure> correlation_;
            std::vector<ext::shared_ptr<SmileSection> > baseSmileSection_;
            std::vector<Time> startTimeBase_; // for correlation parametrisation
            std::vector<Real> fraRateBase_;
            Real fraRateTarg_;
            std::vector<Real> v_;
            // implement transformation formula
            Volatility volatilityImpl(Rate strike) const override;

          public:
            // constructor includes actual transformation details
            TenorOptionletSmileSection(const TenorOptionletVTS& volTS, Time optionTime);

            // further SmileSection interface methods
            Real minStrike() const override {
                return baseSmileSection_[0]->minStrike() + fraRateTarg_ - fraRateBase_[0];
            }
            Real maxStrike() const override {
                return baseSmileSection_[0]->maxStrike() + fraRateTarg_ - fraRateBase_[0];
            }
            Real atmLevel() const override { return fraRateTarg_; }
        };

        Handle<OptionletVolatilityStructure> baseVTS_;
        ext::shared_ptr<IborIndex> baseIndex_;
        ext::shared_ptr<IborIndex> targIndex_;
        ext::shared_ptr<CorrelationStructure> correlation_;

      public:
        // functor interface for parametric correlation
        class CorrelationStructure {
          public:
            // return the correlation between two FRA rates starting at start1 and start2
            virtual Real operator()(const Time& start1, const Time& start2) const = 0;
            virtual ~CorrelationStructure() = default;
            ;
        };

        // very basic choice for correlation structure
        class TwoParameterCorrelation : public CorrelationStructure {
          protected:
            ext::shared_ptr<Interpolation> rhoInf_;
            ext::shared_ptr<Interpolation> beta_;

          public:
            TwoParameterCorrelation(ext::shared_ptr<Interpolation> rhoInf,
                                    ext::shared_ptr<Interpolation> beta)
            : rhoInf_(std::move(rhoInf)), beta_(std::move(beta)) {}
            Real operator()(const Time& start1, const Time& start2) const override {
                Real rhoInf = (*rhoInf_)(start1);
                Real beta = (*beta_)(start1);
                Real rho = rhoInf + (1.0 - rhoInf) * exp(-beta * fabs(start2 - start1));
                return rho;
            }
        };

        // constructor
        TenorOptionletVTS(const Handle<OptionletVolatilityStructure>& baseVTS,
                          ext::shared_ptr<IborIndex> baseIndex,
                          ext::shared_ptr<IborIndex> targIndex,
                          ext::shared_ptr<CorrelationStructure> correlation);

        // Termstructure interface

        //! the latest date for which the curve can return values
        Date maxDate() const override { return baseVTS_->maxDate(); }

        // VolatilityTermstructure interface

        //! implements the actual smile calculation in derived classes
        ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime) const override {
            return ext::shared_ptr<SmileSection>(new TenorOptionletSmileSection(*this, optionTime));
        }
        //! implements the actual volatility calculation in derived classes
        Volatility volatilityImpl(Time optionTime, Rate strike) const override {
            return smileSection(optionTime)->volatility(strike);
        }


        //! the minimum strike for which the term structure can return vols
        Rate minStrike() const override { return baseVTS_->minStrike(); }
        //! the maximum strike for which the term structure can return vols
        Rate maxStrike() const override { return baseVTS_->maxStrike(); }

        // the methodology is designed for normal volatilities
        VolatilityType volatilityType() const override { return Normal; }
    };

    typedef TenorOptionletVTS::CorrelationStructure TenorOptionletVTSCorrelationStructure;

}

#endif
