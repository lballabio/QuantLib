
EXTRA_DIST=TestUnit.py \
	american_option.py barrier_option.py binary_option.py \
	date.py distributions.py european_option.py \
	european_with_dividends.py everest_option.py \
	finite_difference_european.py get_covariance.py \
	himalaya_option.py implied_volatility.py \
	montecarlo_pricers.py pagoda_option.py \
	plain_basket_option.py random_generators.py \
	risk_statistics.py statistics.py term_structures.py

python-test:
	for i in *.py ; do eval python $$i -b || exit 1 ; done

