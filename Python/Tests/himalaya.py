import QuantLib

def initCovariance(corr, vol):
    n = len(vol)
    cov = QuantLib.Matrix(n,n)
    if(n != corr.rows()):
        print "correlation matrix and volatility vector have different size"
    for i in range(n):
        cov[i][i] = vol[i]*vol[i]
        for j in range(i):
            cov[i][j] = corr[i][j]*vol[i]*vol[j]
            cov[j][i] = cov[i][j]
    return cov

def main():
    cor = QuantLib.Matrix(4,4)
    cor[0][0] = 1.00
    cor[1][0] = 0.50; cor[1][1] = 1.00
    cor[2][0] = 0.30; cor[2][1] = 0.20; cor[2][2] = 1.00
    cor[3][0] = 0.10; cor[3][1] = 0.40; cor[3][2] = 0.60; cor[3][3] = 1.00

    volatility = [ 0.3,  0.3,  0.3,  0.3]
    covariance = initCovariance(cor,volatility)
    
    assetValues = [100, 100, 100, 100]
    dividendYields = [0.0, 0.0, 0.0, 0]
    timeIncrements = [0.25, 0.25, 0.25, 0.25]
    riskFreeRate = 0.05
    strike = 100    
    samples = 40000
    seed = 3456789

    him = QuantLib.Himalaya(assetValues, dividendYields, covariance,
                riskFreeRate, strike, timeIncrements, samples, seed)

    print "Himalaya option price and error:", 
    print him.value(), him.errorEstimate()
    print "Stored values:                   7.30213127731 0.0521786123186" 

main()
print 'Press return to end this test'
raw_input()
    