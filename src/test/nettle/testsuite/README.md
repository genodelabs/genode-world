# Testsuite for nettle

This suite builds the tests included with the nettle library.

The make files for the tests are generated and can be (re-)create with

    make -f build_targets.mk

The run script for the tests starts one test after the other and are limited to a maximum run time of 2 seconds. This limit is needed as the dynamic rom mechanism currently doesn't provide support for changing configurations depending on another input.

The build_target.mk and the testsuite.run files contain all existing tests, the tests not possible to run on genode are commented out and have the information about the (supposed) reason.
