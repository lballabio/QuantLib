
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Decillion Pty(Ltd)

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

/*! \file basispointsensitivity.hpp
    \brief basis point sensitivity calculator
*/

#ifndef quantlib_bps_calculator_hpp
#define quantlib_bps_calculator_hpp

#include <ql/termstructure.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/timebasket.hpp>

namespace QuantLib {

    //! basis point sensitivity (BPS) calculator
    /*! Instances of this class accumulate the BPS of each
        cash flow they visit, returning the sum through their
        result() method.
    */
    class BPSCalculator : public AcyclicVisitor,
                          public Visitor<CashFlow>,
                          public Visitor<Coupon> {
      public:
        BPSCalculator(const RelinkableHandle<TermStructure>& ts) 
        : termStructure_(ts), result_(0.0) {}
        //! \name Visitor interface
        //@{
        virtual void visit(Coupon&);
        virtual void visit(CashFlow&);
        //@}
        double result() const { return result_; }
      private:
        RelinkableHandle<TermStructure> termStructure_;
        double result_;
    };

    //! Collective basis-point sensitivity of a cash-flow sequence
    double BasisPointSensitivity(
                             const std::vector<boost::shared_ptr<CashFlow> >&,
                             const RelinkableHandle<TermStructure>&);


    /*! \bug This class must still be checked. It is not guaranteed
             to yield the right results.
    */
    class BPSBasketCalculator : public AcyclicVisitor,
                                public Visitor<CashFlow>,
                                public Visitor<Coupon>,
                                public Visitor<FixedRateCoupon> {
      public:
        BPSBasketCalculator(const RelinkableHandle<TermStructure>& ts,
                            int basis)
        : termStructure_(ts), basis_(basis) {}
        //! \name Visitor interface
        //@{
        double sensfactor(const Date& date) const;
        virtual void visit(Coupon&);
        virtual void visit(FixedRateCoupon&);
        virtual void visit(CashFlow&);
        //@}
        const TimeBasket& result() const { return result_; }
      private:
        RelinkableHandle<TermStructure> termStructure_;
        int basis_;
        TimeBasket result_;
    };

    /*! \bug This function must still be checked. It is not guaranteed
             to yield the right results.
    */
    TimeBasket BasisPointSensitivityBasket(
                             const std::vector<boost::shared_ptr<CashFlow> >&,
                             const RelinkableHandle<TermStructure>&,
                             int basis);

}


#endif
