//
//
//														alphafinder.cpp
//
//

#include <ql/models/marketmodels/models/alphafinder.hpp>

namespace QuantLib
{
template<class T, double (T::*Value)(double) >
double Bisection(double target, 
                 double low, 
                 double high,
                 double tolerance,
                 T& theObject) 
{

    double x=0.5*(low+high);
    double y=(theObject.*Value)(x);
    
    do
    {
       
        if (y < target)
            low = x;

        if (y > target)
            high = x;

        x = 0.5*(low+high);

        y = (theObject.*Value)(x);
    
    }
    while 
        ( (fabs(y-target) > tolerance) );

    return x;
}


	class quadratic
	{
	public:
		quadratic (Real a, Real b, Real c);
		Real turningPoint() const;
		Real valueAtTurningPoint() const;
		Real operator()(Real x) const;
		Real discriminant() const;
		bool roots(Real& x, Real &y) const; // return false if roots not real, and give turning point instead
		
	private:
		Real a_;
		Real b_;
		Real c_;

	};

quadratic::quadratic(Real a, Real b, Real c) : a_(a), b_(b), c_(c)
{
}

Real quadratic::turningPoint() const
{
		return -b_/(2.0*a_);
}

Real quadratic::valueAtTurningPoint() const
{
	return (*this)(turningPoint());
}

Real quadratic::operator()(Real x) const
{
	return x*(x*a_+b_)+c_;
}

Real quadratic::discriminant() const
{
	return b_*b_-4*a_*c_;
}

bool quadratic::roots(Real& x, Real &y) const// return false if roots not real, and give turning point instead
{
	Real d = discriminant();
	if (d<0)
	{
		x = y = turningPoint();
		return false;
	}
	d = sqrt(d);
	x = (-b_ -  d)/(2*a_);
	y = (-b_ + d)/(2*a_);
	return true;

}
alphafinder::alphafinder(boost::shared_ptr<alphaform> parametricform)
: parametricform_(parametricform)
{
}


	Real alphafinder::computeLinearPart(
		Real alpha
		)
	{
		Real cov =0.0;
		parametricform_->setAlpha(alpha);

		for (Integer i=0; i < stepindex_+1; ++i)
		{
			Real vol1 = ratetwohomogeneousvols_[i]*(*parametricform_)(i);
			cov += vol1*rateonevols_[i]*correlations_[i];
		}

		cov *= 2*w0_*w1_;

		return cov;
	}


	Real alphafinder::computeQuadraticPart(
			Real alpha
		)
	{
		Real var =0.0;
		parametricform_->setAlpha(alpha);

		for (Integer i=0; i < stepindex_+1; ++i)
		{
			Real vol = ratetwohomogeneousvols_[i]*(*parametricform_)(i);
			var+= vol*vol;
		}

		var *= w1_*w1_;

		return var;
	}

void alphafinder::finalPart( Real alphaFound,
						Integer stepindex,
						const std::vector<Volatility>& ratetwohomogeneousvols,
						Real quadraticPart,
						Real linearPart,
						Real constantPart,
						Real& alpha,
						Real& a,
						Real& b,
						std::vector<Volatility>& ratetwovols)
{
		    alpha = alphaFound;
			quadratic q2(quadraticPart, linearPart, constantPart );
			parametricform_->setAlpha(alpha);
			Real y; // dummy
			q2.roots(a,y);

			Real totalVar=0.0;
			Real varSoFar=0.0;
			for (Integer i =0; i < stepindex+1; ++i)
			{
				totalVar += ratetwohomogeneousvols[i] * ratetwohomogeneousvols[i];
				ratetwovols[i] =  ratetwohomogeneousvols[i] * (*parametricform_)(i)*a;
				varSoFar += ratetwovols[i]* ratetwovols[i];
			}
			totalVar += ratetwohomogeneousvols[stepindex+1] * ratetwohomogeneousvols[stepindex+1];

			Real VarToFind = totalVar-varSoFar;
			Real requiredSd = sqrt(VarToFind);
			b = requiredSd / (ratetwohomogeneousvols[stepindex+1] * (*parametricform_)(stepindex));
			ratetwovols[stepindex+1] = requiredSd;
			return;
		
}
Real alphafinder::valueAtTurningPoint(Real alpha)
{
	linearPart_ = computeLinearPart(
			alpha
			);

    quadraticPart_ = computeQuadraticPart(
			alpha
			);

	quadratic q(quadraticPart_, linearPart_, constantPart_);
	Real valueAtTP =q.valueAtTurningPoint();
	return valueAtTP;

}

Real alphafinder::minusValueAtTurningPoint(Real alpha)
{
	return - valueAtTurningPoint(alpha);
}
	
void alphafinder::solve( Real alpha0,
		Integer stepindex, // index of caplet we are trying to find
		const std::vector<Volatility>& rateonevols,
		const std::vector<Volatility>& ratetwohomogeneousvols,
		const std::vector<Real>& correlations,
		Real w0,
		Real w1,
		Real targetVariance,
    	Real tolerance,
    	Real alphaMax,
	 	Real alphaMin,
		Integer steps,
		Real& alpha,
		Real& a,
		Real& b,
		std::vector<Volatility>& ratetwovols
		)
{
	stepindex_=stepindex;
	rateonevols_=rateonevols;
	ratetwohomogeneousvols_=ratetwohomogeneousvols;
	correlations_=correlations;
	w0_=w0;
	w1_=w1;

		// constant part will not depend on alpha

	constantPart_ =0.0;
	for (Integer i=0; i < stepindex+1; ++i)
			constantPart_+=rateonevols[i]*rateonevols[i];

	constantPart_ *= w0*w0;

		// compute linear part with initial alpha
	Real valueAtTP = valueAtTurningPoint(alpha0);

	if (valueAtTP <= targetVariance)
	{
		finalPart(  alpha0,
			stepindex,
			ratetwohomogeneousvols,
			quadraticPart_,
			linearPart_,
			constantPart_-targetVariance,
			alpha,
			a,
			b,
			ratetwovols);
		return;
	}

	// we now have to solve

	Real bottomValue = valueAtTurningPoint(alphaMin);
	Real bottomAlpha = alphaMin;
	Real topValue = valueAtTurningPoint(alphaMax);
	Real topAlpha = alphaMax;
	Real bilimit = alpha0;

	if (bottomValue > targetVariance && topValue > targetVariance)
	{// see if if ok at some intermediate point by stepping through
		Integer i=1; 
		while ( i < steps && topValue> targetVariance)
		{
			topAlpha = alpha0 + (alphaMax-alpha0)*(i+0.0)/(steps+0.0);
			topValue=valueAtTurningPoint(topAlpha);
			++i;
		}

		if (topValue <= targetVariance)
			bilimit = alpha0 + (topAlpha-alpha0)*(i-2.0)/(steps+0.0);

	}

	if (bottomValue > targetVariance && topValue > targetVariance)
	{ // see if if ok at some intermediate point by stepping through
		Integer i=1;  
		while ( i < steps && topValue> targetVariance)
		{
			bottomAlpha = alpha0 + (alphaMin-alpha0)*(i+0.0)/(steps+0.0);
			bottomValue=valueAtTurningPoint(bottomAlpha);
			++i;
		}

		if (bottomValue <= targetVariance)
			bilimit = alpha0 +(bottomAlpha-alpha0)*(i-2.0)/(steps+0.0);

	}

	if (bottomValue > targetVariance && topValue > targetVariance)
		throw("method failed");

	if (bottomValue <= targetVariance) // then find root of increasing function (or as if increasing function)
	{
		alpha = Bisection<alphafinder,&alphafinder::valueAtTurningPoint>(targetVariance,
																															bottomAlpha,
																															bilimit,
																															tolerance,
																															*this);		
	}
	else // find root of decreasing function (or as if decreasing function)
	{
		alpha = Bisection<alphafinder,&alphafinder::minusValueAtTurningPoint>(targetVariance,
																															bilimit,
																															topAlpha,
																															tolerance,
																															*this);		
	}


	finalPart(  alpha,
			stepindex,
			ratetwohomogeneousvols,
			quadraticPart_,
			linearPart_,
			constantPart_-targetVariance,
			alpha,
			a,
			b,
			ratetwovols);

	return;

	}
}
