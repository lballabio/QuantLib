
/*
  Copyright (C) 2002 Andre Louw.

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it under the
  terms of the QuantLib license.  You should have received a copy of the
  license along with this program; if not, please email ferdinando@ametrano.net
  The license is also available online at http://quantlib.org/html/license.html

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file compoundforward.cpp
  \brief compounded forward term structure

  \fullpath
  ql/TermStructures/%compoundforward.cpp
*/

// $Id$

#include <ql/TermStructures/compoundforward.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib
{
   using Math::LinearInterpolation;
   using Math::LogLinearInterpolation;

   namespace TermStructures
   {

      CompoundForward::CompoundForward(const std::vector < Date > &dates,
                                       const std::vector < Rate > &forwards,
                                       const Currency currency,
                                       const DayCounter & dayCounter,
                                       const Date & todaysDate,
                                       const Calendar & calendar,
                                       const int settlementDays,
                                       const RollingConvention roll,
                                       const int compoundFrequency)
	 :currency_(currency), dayCounter_(dayCounter),
	  todaysDate_(todaysDate), calendar_(calendar),
	  settlementDays_(settlementDays), roll_(roll),
	  compoundFrequency_(compoundFrequency),
	  needsBootstrap_(true), inputDates_(dates),
	  dates_(dates), forwards_(forwards)
      {

         QL_REQUIRE(dates_.size() > 0, "No input Dates given");
         QL_REQUIRE(forwards_.size() > 0, "No input rates given");
         QL_REQUIRE(dates_.size() == forwards_.size(),
                    "Inconsistent number of Dates/Forward Rates");
         settlementDate_ = calendar.advance(todaysDate_,
                                            settlementDays_, Days);
         discounts_ = std::vector < DiscountFactor > ();

         validateInputs();
      }

      CompoundForward::CompoundForward(const std::vector < std::string >
                                       &identifiers,
                                       const std::vector < Rate > &forwards,
                                       const Currency currency,
                                       const DayCounter & dayCounter,
                                       const Date & todaysDate,
                                       const Calendar & calendar,
                                       const int settlementDays,
                                       const RollingConvention roll,
                                       const int compoundFrequency)
	 :currency_(currency), dayCounter_(dayCounter),
	  todaysDate_(todaysDate), calendar_(calendar),
	  settlementDays_(settlementDays), roll_(roll),
	  compoundFrequency_(compoundFrequency),
	  needsBootstrap_(true),
	  forwards_(forwards)
      {

         QL_REQUIRE(identifiers.size() > 0, "No input Identifiers given");
         QL_REQUIRE(forwards_.size() > 0, "No input rates given");
         QL_REQUIRE(identifiers.size() == forwards_.size(),
                    "Inconsistent number of Identifiers/Forward Rates");
         settlementDate_ = calendar.advance(todaysDate_,
                                            settlementDays_, Days);
         discounts_ = std::vector < DiscountFactor > ();

         for(Size i = 0; i < identifiers.size(); i++)
            dates_.push_back(calendar.advance(settlementDate_,
                                              Period(identifiers.at(i)),
                                              roll));
         inputDates_ = dates_;

         validateInputs();
      }

      void CompoundForward::bootstrap() const
      {
         // prevent recursively calling bootstrap() when the
         // term structure methods are called by the rate helpers
         needsBootstrap_ = false;
         try
         {
            Date compoundDate = calendar_.advance(settlementDate_,
                                                  compoundFrequency_,
                                                  Months, roll_);

            for(Size i = 0, ci = 0; i < dates_.size(); i++)
            {
               DiscountFactor df;
               Time t;

               Date rateDate = dates_[i];
               Rate fwd = forwards_[i];
               if (compoundDate >= rateDate)
               {
                  t = dayCounter_.yearFraction(settlementDate_,
                                               rateDate,
                                               settlementDate_, rateDate);
                  df = 1.0 / (1.0 + fwd * t);
                  ci = i;
               }
               else
               {
                  Size a;
                  Date aDate, pDate;
                  double tempD = 0.0, prev;

                  QL_REQUIRE(discounts_.size() > 0,
                             "Needs forward on at least "
                             "compounding start");
                  for(a = discounts_.size() - 1; a > ci; a--)
                  {
                     prev = discounts_.at(a);
                     aDate = dates_.at(a);
                     pDate = dates_.at(a - 1);
                     t = dayCounter_.yearFraction(pDate, aDate, pDate, aDate);
                     tempD += fwd * prev * t;
                  }
                  prev = discounts_.at(a);
                  aDate = dates_.at(a);
                  t = dayCounter_.yearFraction(settlementDate_, aDate,
                                               settlementDate_, aDate);
                  tempD += fwd * prev * t;

                  aDate = dates_.at(discounts_.size());
                  pDate = dates_.at(discounts_.size() - 1);
                  t = dayCounter_.yearFraction(pDate, aDate, pDate, aDate);
                  df = (1.0 - tempD) / (1.0 + fwd * t);
               }
               discounts_.push_back(df);
            }
            dfinterp_ = Handle < DfInterpolation >
               (new DfInterpolation(times_.begin(),
				    times_.end(),
                                    discounts_.begin()));
         }
         catch(...)
         {
         }
      }

      void CompoundForward::validateInputs() const
      {
         for(Size i = 0; i < dates_.size(); i++)
            times_.push_back(dayCounter_.yearFraction(settlementDate_,
                                                      dates_[i]));

         fwdinterp_ = Handle < FwdInterpolation >
            (new FwdInterpolation(times_.begin(),
				  times_.end(),
				  forwards_.begin()));

         Date compoundDate = calendar_.advance(settlementDate_,
                                               compoundFrequency_,
                                               Months,
                                               roll_);

         for(Size i = 0, ci = 1; i < dates_.size(); i++)
         {
            Date rateDate;

               rateDate = dates_.at(i);
            // Passed compounding?
            if (compoundDate < rateDate)
            {
               Date tmpDate;

                  tmpDate = calendar_.advance(settlementDate_,
                                              compoundFrequency_ *
                                              (++ci), Months, roll_);
               // Missed any forwards?
               while(tmpDate < rateDate)
               {
                  Time t = dayCounter_.yearFraction(settlementDate_,
                                                    tmpDate);
                  Rate r = (*fwdinterp_) (t);

                     dates_.insert(dates_.begin() + i, tmpDate);
                     forwards_.insert(forwards_.begin() + i, r);
                     i++;
                     tmpDate = calendar_.advance(settlementDate_,
                                                 compoundFrequency_ *
                                                 (++ci), Months, roll_);
               }
            }
         }
      }

      DiscountFactor CompoundForward::discountImpl(Time t, bool extrapolate) const
      {
         if(needsBootstrap_)
            bootstrap();
         if(t == 0.0)
            return 1.0;
         int n = referenceNode(t, extrapolate);
         if (t == times_[n])
               return discounts_[n];
            return (*dfinterp_) (t);
      }

      Rate CompoundForward::zeroYieldImpl(Time t, bool extrapolate) const
      {
         if(needsBootstrap_)
            bootstrap();
         return DiscountStructure::zeroYieldImpl(t, extrapolate);
      }

      Rate CompoundForward::forwardImpl(Time t, bool extrapolate) const
      {
         if(needsBootstrap_)
            bootstrap();
         return DiscountStructure::forwardImpl(t, extrapolate);
      }

      int CompoundForward::referenceNode(Time t, bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "CompoundForward: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
         if(t >= times_.back())
            return times_.size() - 1;
         std::vector < Time >::const_iterator i = times_.begin(),
            j = times_.end(), k;
         while(j - i > 1)
         {
            k = i + (j - i) / 2;
            if(t <= *k)
               j = k;
            else
               i = k;
         }
         return (j - times_.begin());
      }
   }
}
