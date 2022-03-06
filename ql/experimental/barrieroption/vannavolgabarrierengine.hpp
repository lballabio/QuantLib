/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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

/*! \file vannavolgabarrierengine.hpp
    \brief Vanna/Volga barrier option engine
*/

#ifndef quantlib_vanna_volga_barrier_engine_hpp
#define quantlib_vanna_volga_barrier_engine_hpp

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/instruments/dividendbarrieroption.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>

namespace QuantLib {

    //! Vanna Volga barrier option engine

    /*!
        \ingroup barrierengines
    */
    class VannaVolgaBarrierEngine
        : public GenericEngine<DividendBarrierOption::arguments,
                               DividendBarrierOption::results> {
      public:

        // Constructor
        VannaVolgaBarrierEngine(Handle<DeltaVolQuote> atmVol,
                                Handle<DeltaVolQuote> vol25Put,
                                Handle<DeltaVolQuote> vol25Call,
                                Handle<Quote> spotFX,
                                Handle<YieldTermStructure> domesticTS,
                                Handle<YieldTermStructure> foreignTS,
                                bool adaptVanDelta = false,
                                Real bsPriceWithSmile = 0.0);

        void calculate() const override;

      private:
        const Handle<DeltaVolQuote> atmVol_;
        const Handle<DeltaVolQuote> vol25Put_;
        const Handle<DeltaVolQuote> vol25Call_;
        const Time T_;
        const Handle<Quote> spotFX_;
        const Handle<YieldTermStructure> domesticTS_;
        const Handle<YieldTermStructure> foreignTS_;
        const bool adaptVanDelta_;
        const Real bsPriceWithSmile_;

    };


}

#endif /*quantlib_fd_black_scholes_barrier_engine_hpp*/


#ifndef id_007db98bc58e4aadaf1d9c98537fd065
#define id_007db98bc58e4aadaf1d9c98537fd065
inline bool test_007db98bc58e4aadaf1d9c98537fd065(const int* i) {
    return i != nullptr;
}
#endif
