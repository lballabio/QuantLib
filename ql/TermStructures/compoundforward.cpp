#include <ql/TermStructures/compoundforward.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib
{
   namespace TermStructures
   {
      CompoundForward::CompoundForward(
	 const std::vector<Date>& dates,
	 const std::vector<Rate>& forwards,
	 const Currency currency,
	 const DayCounter& dayCounter,
	 const Date& todaysDate,
	 const Calendar& calendar,
	 const int settlementDays,
	 const RollingConvention roll,
	 const int compoundFrequency)
	 : currency_(currency), dayCounter_(dayCounter),
	   todaysDate_(todaysDate),
	   calendar_(calendar),settlementDays_(settlementDays),
	   roll_(roll), compoundFrequency_(compoundFrequency),
	   needsBootstrap_(true),dates_(dates), forwards_(forwards)
      {
	 QL_REQUIRE(dates_.size()>0,"No input Dates given");
	 QL_REQUIRE(forwards_.size()>0,"No input rates given");
	 QL_REQUIRE(dates_.size()==forwards_.size(),
		    "Inconsistent number of Dates/Forward Rates");
	 settlementDate_ = calendar.advance(todaysDate_,settlementDays_,Days);

	 discounts_ = std::vector<DiscountFactor>();
	 zeroYields_ = std::vector<Rate>();
   
	 for (Size i=0; i<dates_.size(); i++)
	    times_.push_back(dayCounter_.yearFraction(settlementDate_,
						      dates_[i]));
	 
 	 fwdinterp_ = Handle<FwdInterpolation>
	    (new FwdInterpolation(times_.begin(), times_.end(),
				  forwards_.begin(), true));

	 dfinterp_ = Handle<DfInterpolation>
	    (new DfInterpolation(times_.begin(), times_.end(),
				 discounts_.begin(), true));
	 
	 validateInputs();
      }

      CompoundForward::CompoundForward(
	 const std::vector<std::string>& identifiers,
	 const std::vector<Rate>& forwards,
	 const Currency currency,
	 const DayCounter& dayCounter,
	 const Date& todaysDate,
	 const Calendar& calendar,
	 const int settlementDays,
	 const RollingConvention roll,
	 const int compoundFrequency)
	 : currency_(currency), dayCounter_(dayCounter),
	   todaysDate_(todaysDate),
	   calendar_(calendar),settlementDays_(settlementDays),
	   roll_(roll), compoundFrequency_(compoundFrequency),
	   needsBootstrap_(true),forwards_(forwards)
      {
	 QL_REQUIRE(identifiers.size()>0,"No input Identifiers given");
	 QL_REQUIRE(forwards_.size()>0,"No input rates given");
	 QL_REQUIRE(identifiers.size()==forwards_.size(),
		    "Inconsistent number of Identifiers/Forward Rates");
	 settlementDate_ = calendar.advance(todaysDate_,settlementDays_,Days);
	 for (Size i=0; i<identifiers.size(); i++)
	 {
	    std::string identifier;
	    Date aDate;
	    
	    identifier = identifiers.at(i);
	    aDate = calendar.advance(settlementDate_,
					Period(identifier), roll);
	    dates_.push_back(aDate);
	 }
	 discounts_ = std::vector<DiscountFactor>();
	 zeroYields_ = std::vector<Rate>();
   
	 for (Size i=0; i<dates_.size(); i++)
	    times_[i] = dayCounter_.yearFraction(settlementDate_, dates_[i]);
	 
 	 fwdinterp_ = Handle<FwdInterpolation>
	    (new FwdInterpolation(times_.begin(), times_.end(),
				  forwards_.begin(), true));

	 dfinterp_ = Handle<DfInterpolation>
	    (new DfInterpolation(times_.begin(), times_.end(),
				 discounts_.begin(), true));
	 
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
						  compoundFrequency_, Months,
						  roll_);
	    
	    for (Size i=0,ci=1; i<dates_.size(); i++)
	    {
	       DiscountFactor df;
	       Time t;

	       Date rateDate = dates_.at(i);
	       Rate fwd = forwards_.at(i);
	       if (compoundDate >= rateDate)
	       {
		  t = dayCounter_.yearFraction(settlementDate_,rateDate,
					       settlementDate_,rateDate);
		  df = 1.0/(1.0+fwd*t);
		  std::cout << rateDate
			    << ", fwd => "
			    << DoubleFormatter::toString(fwd,8)
			    << ", df => "
			    << DoubleFormatter::toString(df,8) << std::endl;
		  ci = i;
	       }
	       else
	       {
		  Size a, currCnt;
		  double tempD = 0.0, prev = 0.0;
		  Date aDate, pDate;

		  currCnt = discounts_.size();
		  for (a = currCnt-1; a>ci; a--)
		  {
		     prev = discounts_.at(a);
		     aDate = dates_.at(a);
		     pDate = dates_.at(a-1);
		     t = dayCounter_.yearFraction(pDate,aDate,
						  pDate,aDate);
		     tempD += fwd*prev*t;
		  }
		  if (a >= 0)
		  {
		     prev = discounts_.at(a);
		     aDate = dates_.at(a);
		     t = dayCounter_.yearFraction(settlementDate_,aDate,
						  settlementDate_,aDate);
		     tempD += fwd*prev*t;
		     
		     aDate = dates_.at(currCnt);
		     pDate = dates_.at(currCnt-1);
		     t = dayCounter_.yearFraction(pDate,aDate,
						  pDate,aDate);
		  }
		  else
		     t = dayCounter_.yearFraction(settlementDate_,rateDate,
						  settlementDate_,rateDate);
		  df = (1.0-tempD)/(1.0+fwd*t);
		  std::cout << rateDate
			    << ", fwd => "
			    << DoubleFormatter::toString(fwd,8)
			    << ", df => "
			    << DoubleFormatter::toString(df,8)
			    << ", tempD => "
			    << DoubleFormatter::toString(tempD,8)
			    << std::endl;
	       }
	       discounts_.push_back(df);
	       zeroYields_.push_back(-QL_LOG(df) / times_[i]);
	    }
	 }
	 catch (...)
	 {
	 }
      }

      void CompoundForward::validateInputs() const
      {
	 Date compoundDate = calendar_.advance(settlementDate_,
					       compoundFrequency_,
					       Months,
					       roll_);

	 for (Size i=0,ci=1; i<dates_.size(); i++)
	 {
	    Date rateDate;

	    rateDate = dates_.at(i);
	    std::cout << "Checking " << rateDate << "  ..." << std::endl;
	    // Passed compounding?
	    if (compoundDate < rateDate)
	    {
	       Date tmpDate;

	       tmpDate = calendar_.advance(settlementDate_,
					   compoundFrequency_*(++ci),
					   Months,
					   roll_);
	       // Missed any forwards?
	       while (tmpDate < rateDate)
	       {
		  std::cout << "Found " << tmpDate << std::endl;
		  Time t = dayCounter_.yearFraction(settlementDate_, tmpDate);
		  Rate r = (*fwdinterp_)(t);

		  dates_.insert(dates_.begin()+i,tmpDate);
		  forwards_.insert(forwards_.begin()+i,r);
		  i++;
		  tmpDate = calendar_.advance(settlementDate_,
					      compoundFrequency_*(++ci),
					      Months,
					      roll_);
	       }
	    }
	 }
      }

      Rate CompoundForward::zeroYieldImpl(Time t,
					  bool extrapolate) const
      {
	 if (needsBootstrap_) 
	    bootstrap();
	 if (t == 0.0)
	 {
	    return zeroYields_[0];
	 }
	 else
	 {
	    int n = referenceNode(t, extrapolate);
	    if (t == times_[n])
	    {
	       return zeroYields_[n];
	    }
	    else
	    {
	       Time tn = times_[n-1];
	       return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
	    }
	 }
	 QL_DUMMY_RETURN(Rate());
      }

      DiscountFactor CompoundForward::discountImpl(Time t,
						   bool extrapolate) const
      {
	 if (needsBootstrap_) 
	    bootstrap();
	 if (t == 0.0)
	 {
	    return 1.0;
	 }
	 else
	 {
	    int n = referenceNode(t, extrapolate);
	    if (t == times_[n])
	    {
	       return discounts_[n];
	    }
	    else
	    {
	       return (*dfinterp_)(t);
	    }
	 }
	 QL_DUMMY_RETURN(DiscountFactor());
      }

      Rate CompoundForward::forwardImpl(Time t,
					bool extrapolate) const
      {
	 if (needsBootstrap_) 
	    bootstrap();
	 if (t == 0.0)
	 {
	    return forwards_[0];
	 }
	 else
	 {
	    int n = referenceNode(t, extrapolate);
	    if (t == times_[n])
	    {
	       return forwards_[n];
	    }
	    else
	    {
	       return (*fwdinterp_)(t);
	    }
	 }
	 QL_DUMMY_RETURN(Rate());
      }

      int CompoundForward::referenceNode(Time t, bool extrapolate) const
      {
	 QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "CompoundForward: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
	 if (t>=times_.back())
	    return times_.size()-1;
	 std::vector<Time>::const_iterator i=times_.begin(),
	    j=times_.end(), k;
	 while (j-i > 1)
	 {
	    k = i+(j-i)/2;
	    if (t <= *k)
	       j = k;
	    else
	       i = k;
	 }
	 return (j-times_.begin());
      }
   }
}
