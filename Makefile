.DEFAULT_GOAL := help
.PHONY: help install test test-verbose lint format typecheck precommit clean ci

help: ## Show this help message
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}'

install: ## Install package and dev dependencies
	pip install -e ".[dev]"
	pre-commit install

test: ## Run all tests
	python -m pytest tests/

test-verbose: ## Run tests with verbose output
	python -m pytest tests/ -v

lint: ## Run linter (ruff check + format check)
	ruff check .
	ruff format --check .

format: ## Auto-format code (ruff format)
	ruff format .
	ruff check --fix .

typecheck: ## Run type checker (pyright)
	pyright quantlib/

precommit: ## Run all pre-commit hooks
	pre-commit run --all-files

clean: ## Remove build artifacts and caches
	rm -rf build/ dist/ *.egg-info .pytest_cache .ruff_cache __pycache__
	find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true

ci: ## Run full CI pipeline locally (lint + typecheck + test)
	$(MAKE) lint
	$(MAKE) typecheck
	$(MAKE) test
