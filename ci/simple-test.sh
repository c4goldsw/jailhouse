#!/usr/bin/env bats

PATH_TO_JAILHOUSE=~

# ================================
# TESTS
# ================================

# BATS executes each of these in the order they're instantiated in.

# Although BATS has a setup() and teardown() function for each individual test,
# it lacks such setup / teardown functions that run before / after all tests.
# Thus, this "test" is used as a setup function.
@test "pre-testing setup" {
	rm -f .coverage*
}

# The only legitimate test here - both instances of [ "$?" == "0" ] must
# evaluate to 0 for the test to succeed (the test fails whenever such an 
# expression evaluates to a non-zero value)
@test "intel" {

	coverage run -p --include="*jailhouse*" ${PATH_TO_JAILHOUSE}/jailhouse/tools/jailhouse-hardware-check
	[ "$?" == "0" ]
	coverage run -p --include="*jailhouse*" ${PATH_TO_JAILHOUSE}/jailhouse/tools/jailhouse-config-create sys.c
	[ "$?" == "0" ]
}

# Counterpart to @test "pre-testing setup"
@test "post-testing teardown" {
	# Generate pretty coverage docs
	coverage combine .coverage.*
}
