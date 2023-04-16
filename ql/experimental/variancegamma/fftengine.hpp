/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

/*! \file fftengine.hpp
    \brief base class for FFT option pricing engines
*/

#ifndef quantlib_fft_engine_hpp
#define quantlib_fft_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/stochasticprocess.hpp>
#include <complex>

namespace QuantLib {

    //! Base class for FFT pricing engines for European vanilla options
    /*! \ingroup vanillaengines
    
        The FFT engine calculates the values of all options with the same expiry at the same time.
        For that reason it is very inefficient to price options individually.  When using this engine
        you should collect all the options you wish to price in a list and call 
        the engine's precalculate method before calling the NPV method of the option.

        References:
        Carr, P. and D. B. Madan (1998),
        "Option Valuation using the fast Fourier transform,"
        Journal of Computational Finance, 2, 61-73.
    */

    class FFTEngine :
        public VanillaOption::engine {
      public:
        FFTEngine(std::shared_ptr<StochasticProcess1D> process, Real logStrikeSpacing);
        void calculate() const override;
        void update() override;

        void precalculate(const std::vector<std::shared_ptr<Instrument> >& optionList);
        virtual std::unique_ptr<FFTEngine> clone() const = 0;
      protected:
        virtual void precalculateExpiry(Date d) = 0;
        virtual std::complex<Real> complexFourierTransform(std::complex<Real> u) const = 0;
        virtual Real discountFactor(Date d) const = 0;
        virtual Real dividendYield(Date d) const = 0;
        void calculateUncached(const std::shared_ptr<StrikedTypePayoff>& payoff,
                               const std::shared_ptr<Exercise>& exercise) const;

        std::shared_ptr<StochasticProcess1D> process_;
        Real lambda_;   // Log strike spacing

      private:
        typedef std::map<std::shared_ptr<StrikedTypePayoff>, Real> PayoffResultMap;
        typedef std::map<Date, PayoffResultMap> ResultMap;
        ResultMap resultMap_;
    };

}


#endif

