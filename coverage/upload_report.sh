#! /bin/sh
set -ex

lcov --list coverage/coverage.info # debug before upload
coveralls-lcov coverage/coverage.info