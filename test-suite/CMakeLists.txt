# cpp files, this list is maintained manually

set(QuantLib-Test_SRC
    americanoption.cpp
    amortizingbond.cpp
    andreasenhugevolatilityinterpl.cpp
    array.cpp
    asianoptions.cpp
    assetswap.cpp
    autocovariances.cpp
    barrieroption.cpp
    basismodels.cpp
    basketoption.cpp
    batesmodel.cpp
    bermudanswaption.cpp
    binaryoption.cpp
    blackdeltacalculator.cpp
    blackformula.cpp
    bonds.cpp
    brownianbridge.cpp
    businessdayconventions.cpp
    calendars.cpp
    capfloor.cpp
    capflooredcoupon.cpp
    cashflows.cpp
    catbonds.cpp
    cdo.cpp
    cdsoption.cpp
    chooseroption.cpp
    cliquetoption.cpp
    cms.cpp
    cmsspread.cpp
    commodityunitofmeasure.cpp
    compiledboostversion.cpp
    compoundoption.cpp
    convertiblebonds.cpp
    covariance.cpp
    creditdefaultswap.cpp
    creditriskplus.cpp
    curvestates.cpp
    dates.cpp
    daycounters.cpp
    defaultprobabilitycurves.cpp
    digitalcoupon.cpp
    digitaloption.cpp
    distributions.cpp
    dividendoption.cpp
    doublebarrieroption.cpp
    doublebinaryoption.cpp
    europeanoption.cpp
    everestoption.cpp
    exchangerate.cpp
    extendedtrees.cpp
    extensibleoptions.cpp
    fastfouriertransform.cpp
    fdcev.cpp
    fdheston.cpp
    fdcir.cpp
    fdmlinearop.cpp
    fdsabr.cpp
    fittedbonddiscountcurve.cpp
    forwardoption.cpp
    forwardrateagreement.cpp
    functions.cpp
    garch.cpp
    gaussianquadratures.cpp
    gjrgarchmodel.cpp
    gsr.cpp
    hestonmodel.cpp
    hestonslvmodel.cpp
    himalayaoption.cpp
    hybridhestonhullwhiteprocess.cpp
    indexes.cpp
    inflation.cpp
    inflationcapfloor.cpp
    inflationcapflooredcoupon.cpp
    inflationcpibond.cpp
    inflationcpicapfloor.cpp
    inflationcpiswap.cpp
    inflationvolatility.cpp
    instruments.cpp
    integrals.cpp
    interestrates.cpp
    interpolations.cpp
    jumpdiffusion.cpp
    lazyobject.cpp
    libormarketmodel.cpp
    libormarketmodelprocess.cpp
    linearleastsquaresregression.cpp
    lookbackoptions.cpp
    lowdiscrepancysequences.cpp
    main.cpp
    margrabeoption.cpp
    marketmodel.cpp
    marketmodel_cms.cpp
    marketmodel_smm.cpp
    marketmodel_smmcapletalphacalibration.cpp
    marketmodel_smmcapletcalibration.cpp
    marketmodel_smmcaplethomocalibration.cpp
    markovfunctional.cpp
    matrices.cpp
    mclongstaffschwartzengine.cpp
    mersennetwister.cpp
    money.cpp
    noarbsabr.cpp
    normalclvmodel.cpp
    nthorderderivativeop.cpp
    nthtodefault.cpp
    numericaldifferentiation.cpp
    observable.cpp
    ode.cpp
    operators.cpp
    optimizers.cpp
    optionletstripper.cpp
    overnightindexedswap.cpp
    pagodaoption.cpp
    partialtimebarrieroption.cpp
    pathgenerator.cpp
    period.cpp
    piecewiseyieldcurve.cpp
    piecewisezerospreadedtermstructure.cpp
    quantlibtestsuite.cpp
    quantooption.cpp
    quotes.cpp
    rangeaccrual.cpp
    riskneutraldensitycalculator.cpp
    riskstats.cpp
    rngtraits.cpp
    rounding.cpp
    sampledcurve.cpp
    schedule.cpp
    shortratemodels.cpp
    sofrfutures.cpp
    solvers.cpp
    spreadoption.cpp
    squarerootclvmodel.cpp
    stats.cpp
    swap.cpp
    swapforwardmappings.cpp
    swaption.cpp
    swaptionvolatilitycube.cpp
    swaptionvolatilitymatrix.cpp
    swingoption.cpp
    termstructures.cpp
    timegrid.cpp
    timeseries.cpp
    tqreigendecomposition.cpp
    tracing.cpp
    transformedgrid.cpp
    twoassetbarrieroption.cpp
    twoassetcorrelationoption.cpp
    utilities.cpp
    variancegamma.cpp
    varianceoption.cpp
    varianceswaps.cpp
    volatilitymodels.cpp
    vpp.cpp
    zabr.cpp
)

set(BENCHMARK_FILES "main.cpp" "quantlibbenchmark.cpp" "americanoption.cpp" "asianoptions.cpp" "barrieroption.cpp"
       "basketoption.cpp" "batesmodel.cpp" "convertiblebonds.cpp" "digitaloption.cpp" "dividendoption.cpp"
       "europeanoption.cpp" "fdheston.cpp" "hestonmodel.cpp" "interpolations.cpp" "jumpdiffusion.cpp"
       "marketmodel_smm.cpp" "marketmodel_cms.cpp" "lowdiscrepancysequences.cpp" "quantooption.cpp" "riskstats.cpp"
       "shortratemodels.cpp" "utilities.cpp" "utilities.hpp" "swaptionvolstructuresutilities.hpp")

if (USE_BOOST_DYNAMIC_LIBRARIES)
    add_definitions(-DBOOST_TEST_DYN_LINK)
endif()

find_package (Boost REQUIRED COMPONENTS unit_test_framework timer system OPTIONAL_COMPONENTS chrono)

set (TEST quantlib-test-suite)
add_executable (${TEST} ${QuantLib-Test_SRC})
target_link_libraries (${TEST} ${QL_LINK_LIBRARY} ${Boost_LIBRARIES})
set_property(TARGET ${TEST} PROPERTY PROJECT_LABEL "testsuite")

set (BENCHMARK quantlib-benchmark)
add_executable (${BENCHMARK} ${BENCHMARK_FILES})
target_link_libraries (${BENCHMARK} ${QL_LINK_LIBRARY} ${Boost_LIBRARIES})
set_property(TARGET ${BENCHMARK} PROPERTY PROJECT_LABEL "benchmark")

add_test (${TEST} ${TEST})
