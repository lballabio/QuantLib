#include <ql/Processes/hullwhiteprocess.hpp>

namespace QuantLib {

    HullWhiteProcess::HullWhiteProcess(const Handle<YieldTermStructure>& h,
                                       Real a,
                                       Real sigma)
    : process_(new OrnsteinUhlenbeckProcess(
                   a, sigma, h->forwardRate(0.0,0.0,Continuous,NoFrequency))),
      h_(h), a_(a), sigma_(sigma) {}

    Real HullWhiteProcess::x0() const {
        return process_->x0();
    }

    Real HullWhiteProcess::drift(Time t, Real x) const {
        return process_->drift(t, x);
    }

    Real HullWhiteProcess::diffusion(Time t, Real x) const{
        return process_->diffusion(t, x);
    }

    Real HullWhiteProcess::expectation(Time t0, Real x0, Time dt) const {
        return process_->expectation(t0, x0, dt)
             + alpha(t0 + dt) - alpha(t0)*std::exp(-a_*dt);
    }

    Real HullWhiteProcess::stdDeviation(Time t0, Real x0, Time dt) const{
        return process_->stdDeviation(t0, x0, dt);
    }

    Real HullWhiteProcess::variance(Time t0, Real x0, Time dt) const{
        return process_->variance(t0, x0, dt);
    }

    Real HullWhiteProcess::alpha(Time t) const {
        Real alfa = (sigma_/a_)*(1 - std::exp(-a_*t));
        alfa *= 0.5*alfa;
        alfa += h_->forwardRate(0.0,0.0,Continuous,NoFrequency);
        return alfa;
    }


    HullWhiteForwardProcess::HullWhiteForwardProcess(
                                          const Handle<YieldTermStructure>& h,
                                          Real a,
                                          Real sigma)
    : process_(new OrnsteinUhlenbeckProcess(
                   a, sigma, h->forwardRate(0.0,0.0,Continuous,NoFrequency))),
      h_(h), a_(a), sigma_(sigma) {}

    Real HullWhiteForwardProcess::x0() const {
        return process_->x0();
    }

    Real HullWhiteForwardProcess::drift(Time t, Real x) const {
        return process_->drift(t, x) - B(t, T_)*sigma_*sigma_;
    }

    Real HullWhiteForwardProcess::diffusion(Time t, Real x) const{
        return process_->diffusion(t, x);
    }

    Real HullWhiteForwardProcess::expectation(Time t0, Real x0,
                                              Time dt) const {
        return process_->expectation(t0, x0, dt)
             + alpha(t0 + dt) - alpha(t0)*std::exp(-a_*dt)
             - M_T(t0, t0+dt, T_);
    }

    Real HullWhiteForwardProcess::stdDeviation(Time t0, Real x0,
                                               Time dt) const {
        return process_->stdDeviation(t0, x0, dt);
    }

    Real HullWhiteForwardProcess::variance(Time t0, Real x0, Time dt) const{
        return process_->variance(t0, x0, dt);
    }

    Real HullWhiteForwardProcess::alpha(Time t) const {
        Real alfa = (sigma_/a_)*(1 - std::exp(-a_*t));
        alfa *= 0.5*alfa;
        alfa += h_->forwardRate(0.0, 0.0, Continuous, NoFrequency);

        return alfa;
    }

    Real HullWhiteForwardProcess::M_T(Real s, Real t, Real T) const {
        Real coeff = (sigma_*sigma_)/(a_*a_);
        Real exp1 = std::exp(-a_*(t-s));
        Real exp2 = std::exp(-a_*(T-t));
        Real exp3 = std::exp(-a_*(T+t-2.0*s));
        Real M = coeff*(1-exp1)-0.5*coeff*(exp2-exp3);
        return M;
    }

    Real HullWhiteForwardProcess::B(Time t, Time T) const {
        Real B = 1/a_ * (1-std::exp(-a_*(T-t)));
        return B;
    }

}

