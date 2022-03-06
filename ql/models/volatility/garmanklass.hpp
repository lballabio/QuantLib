/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file garmanklass.hpp
    \brief Volatility estimators using high low data
*/

#ifndef quantlib_garman_klass_hpp
#define quantlib_garman_klass_hpp

#include <ql/volatilitymodel.hpp>
#include <ql/prices.hpp>

namespace QuantLib {

    //! Garman-Klass volatility model
    /*! This class implements a concrete volatility model based on
        high low formulas using the method of Garman and Klass in
        their paper "On the Estimation of the Security Price from
        Historical Data" at
        http://www.fea.com/resources/pdf/a_estimation_of_security_price.pdf

        Volatilities are assumed to be expressed on an annual basis.
    */
    class GarmanKlassAbstract :
        public LocalVolatilityEstimator<IntervalPrice> {
    protected:
        Real yearFraction_;
        virtual Real calculatePoint(const IntervalPrice &p) = 0;
    public:
        explicit GarmanKlassAbstract(Real y) :
        yearFraction_(y) {}
        TimeSeries<Volatility> calculate(const TimeSeries<IntervalPrice>& quoteSeries) override {
            TimeSeries<Volatility> retval;
            TimeSeries<IntervalPrice>::const_iterator prev, next, cur, start;
            start = quoteSeries.begin();
            for (cur = start; cur != quoteSeries.end(); ++cur) {
                retval[cur->first] =
                    std::sqrt(std::fabs(calculatePoint(cur->second))/
                              yearFraction_);
            }
            return retval;
        }
    };

    class GarmanKlassSimpleSigma :
        public GarmanKlassAbstract {
    public:
        GarmanKlassSimpleSigma(Real y) :
            GarmanKlassAbstract(y) {};
    protected:
      Real calculatePoint(const IntervalPrice& p) override {
          Real c = std::log(p.close() / p.open());
          return c * c;
      }
    };

    /* This template factors out common functionality found in
       classes which rely on the difference between the previous day's
       close price and today's open price. */
    template <class T>
    class GarmanKlassOpenClose : public T {
    protected:
        Real f_;
        Real a_;
    public:
        GarmanKlassOpenClose(Real y, Real marketOpenFraction,
                             Real a) :
        T(y), f_(marketOpenFraction), a_(a) {};
        TimeSeries<Volatility> calculate(const TimeSeries<IntervalPrice>& quoteSeries) override {
            TimeSeries<Volatility> retval;
            TimeSeries<IntervalPrice>::const_iterator prev, next, cur, start;
            start = quoteSeries.begin();
            ++start;
            for (cur = start; cur != quoteSeries.end(); ++cur) {
                prev = cur; --prev;
                Real c0 = std::log(prev->second.close());
                Real o1 = std::log(cur->second.open());
                Real sigma2 =
                    a_ * (o1 - c0) * (o1 - c0) / f_ +
                    (1-a_) * T::calculatePoint(cur->second) /
                    (1-f_);

                retval[cur->first] = std::sqrt(sigma2/T::yearFraction_);
            }
            return retval;
        }
    };


    class GarmanKlassSigma1 :
        public GarmanKlassOpenClose<GarmanKlassSimpleSigma> {
    public:
        GarmanKlassSigma1(Real y, Real marketOpenFraction) :
            GarmanKlassOpenClose<GarmanKlassSimpleSigma>(y,
                                                         marketOpenFraction,
                                                         0.5) {};
    };


    class ParkinsonSigma :
        public GarmanKlassAbstract {
    public:
        ParkinsonSigma(Real y) :
            GarmanKlassAbstract(y) {};
    protected:
      Real calculatePoint(const IntervalPrice& p) override {
          Real u = std::log(p.high() / p.open());
          Real d = std::log(p.low() / p.open());
          return (u - d) * (u - d) / 4.0 / std::log(2.0);
      }
    };


    class GarmanKlassSigma3 :
        public GarmanKlassOpenClose<ParkinsonSigma> {
    public:
        GarmanKlassSigma3(Real y, Real marketOpenFraction) :
            GarmanKlassOpenClose<ParkinsonSigma>(y,
                                                 marketOpenFraction,
                                                 0.17) {};
    };



    class GarmanKlassSigma4 :
        public GarmanKlassAbstract {
    public:
        GarmanKlassSigma4(Real y) :
            GarmanKlassAbstract(y) {};
    protected:
      Real calculatePoint(const IntervalPrice& p) override {
          Real u = std::log(p.high() / p.open());
          Real d = std::log(p.low() / p.open());
          Real c = std::log(p.close() / p.open());
          return 0.511 * (u - d) * (u - d) - 0.019 * (c * (u + d) - 2 * u * d) - 0.383 * c * c;
      }
    };

    class GarmanKlassSigma5 :
        public GarmanKlassAbstract {
    public:
        GarmanKlassSigma5(Real y) :
            GarmanKlassAbstract(y) {};
    protected:
      Real calculatePoint(const IntervalPrice& p) override {
          Real u = std::log(p.high() / p.open());
          Real d = std::log(p.low() / p.open());
          Real c = std::log(p.close() / p.open());
          return 0.5 * (u - d) * (u - d) - (2.0 * std::log(2.0) - 1.0) * c * c;
      }
    };

    class GarmanKlassSigma6 :
        public GarmanKlassOpenClose<GarmanKlassSigma4> {
    public:
        GarmanKlassSigma6(Real y, Real marketOpenFraction) :
        GarmanKlassOpenClose<GarmanKlassSigma4>(y,
                                                marketOpenFraction,
                                                0.012) {};
    };
}


#endif


#ifndef id_17df0bb82192f17ed97cc58042803fa5
#define id_17df0bb82192f17ed97cc58042803fa5
inline bool test_17df0bb82192f17ed97cc58042803fa5(const int* i) {
    return i != nullptr;
}
#endif
