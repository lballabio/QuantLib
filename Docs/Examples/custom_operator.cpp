
// Simple definition of the discretized Black-Scholes operator.
// A more suitable one is available in the library as BSMOperator.

class BlackScholesOperator : public TridiagonalOperator {
  public:
    BlackScholesOperator(
        double sigma, double nu,    // parameters of the
        Rate r,                     // Black-Scholes equation
        int points,                 // number of discretized points
        double h)                   // grid spacing
    : TridiagonalOperator(
        // build the operator by adding basic ones
        - (sigma*sigma/2.0) * DPlusDMinus(points,h)
        - nu * DZero(points,h)
        + r * Identity<Array>(points)
    ) {}
};

// instantiate the operator with the given parameters
TridiagonalOperator L = BlackScholesOperator(sigma, nu, r, points, h);

