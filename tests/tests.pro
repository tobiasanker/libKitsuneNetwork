TEMPLATE = subdirs
CONFIG += ordered
QT -= qt core gui
CONFIG += c++14

SUBDIRS = \
    unit_tests \
    benchmark_tests

tests.depends = src
