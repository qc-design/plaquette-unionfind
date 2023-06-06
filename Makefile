PYTHON3 := $(shell which python3 2>/dev/null)

PYTHON := python3
COVERAGE := --cov=plaquette_unionfind --cov-report term-missing --cov-report=html:coverage_html_report
TESTRUNNER := -m pytest tests --tb=short

.PHONY: format format-cpp format-python clean test-builtin test-cpp
format: format-cpp format-python

format-cpp:
ifdef check
	./bin/format --check --cfversion $(if $(version:-=),$(version),0) ./plaquette_unionfind/src
else
	./bin/format --cfversion $(if $(version:-=),$(version),0) ./plaquette_unionfind/src ./plaquette_unionfind/src/tests
endif

format-python:
ifdef check
	black -l 100 ./tests ./plaquette_unionfind/ --check
else
	black -l 100 ./tests ./plaquette_unionfind/
endif

test-python:
	$(PYTHON) -I $(TESTRUNNER)

test-cpp:
	rm -rf ./BuildTests
	cmake . -BBuildTests -DPLAQUETTE_UNIONFIND_BUILD_TESTS=On
	cmake --build ./BuildTests
	./BuildTests/plaquette_unionfind/src/tests/test_runner

clean: clean-build clean-pyc clean-test
	rm -rf tmp
	rm -rf *.dat
	rm -f *~
	rm -f *#*
	for d in $(find . -name "*~"); do rm $d; done
	rm *.so

clean-build: ## remove build artifacts
	rm -fr build/
	rm -rf BuildTests Build
	rm -fr dist/
	rm -fr .eggs/
	find . -name '*.egg-info' -exec rm -fr {} +
	find . -name '*.egg' -exec rm -f {} +

clean-pyc: ## remove Python file artifacts
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f {} +
	find . -name '__pycache__' -exec rm -fr {} +

clean-test: ## remove test and coverage artifacts
	rm -fr .tox/
	rm -f .coverage
	rm -fr htmlcov/
	rm -fr .pytest_cache

docs: ## generate Sphinx HTML documentation, including API docs
	$(MAKE) -C doc clean
	$(MAKE) -C doc html
