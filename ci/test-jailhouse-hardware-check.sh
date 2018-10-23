#!/usr/bin/env bats

PASS_DIR=~/programming/jailhouse/dev_env/passthrough_dir
DISK=~/programming/jailhouse/dev_env/dev_disk.qcow2


# ================================
# BATS specific helpers
# ================================

# This is a function called by BATS at the end of every @test.
setup() {
	sudo /home/oem/Desktop/qemu/build/x86_64-softmmu/qemu-system-x86_64 \
		-machine q35,kernel_irqchip=split -m 1G -enable-kvm \
		-smp 4 -device intel-iommu,intremap=on,x-buggy-eim=on \
		-cpu kvm64,-kvm_pv_eoi,-kvm_steal_time,-kvm_asyncpf,-kvmclock,+vmx \
		-hda ${DISK} \
		-netdev user,id=net,hostfwd=tcp::5555-:22 -device e1000e,addr=2.0,netdev=net \
		-device intel-hda,addr=1b.0 -device hda-duplex \
		-fsdev local,id=pass_thr,path=${PASS_DIR},security_model=passthrough \
		-device virtio-9p-pci,fsdev=pass_thr,addr=1f.7,mount_tag=passthrough_dir \
		-daemonize -pidfile pid

	ssh root@localhost -T -p 5555 <<-HEREDOC
	mount -t 9p passthrough_dir dev
	HEREDOC
}

# This is a function called by BATS at the end of every @test.
teardown() {
	# Shutdown, prepare for next instance
	ssh root@localhost -T -p 5555 <<-HEREDOC
	shutdown now
	HEREDOC

	# Wait for QEMU to shutdown before proceeding, to prevent an early startup
	# of QEMU occuring once the next test begins (which may fail)
	while [ "$(ps aux | grep $(sudo cat pid) | grep -v grep)" != "" ]; do
		sleep 5
	done

	sudo rm -f pid
}

# ================================
# BATS-independent helpers
# ================================

run_python_script_with_coverage() {
	# Run the command in the QEMU instance and save the return value in tmp
	# (on the instance).  Note that the python script (stored in $1) is being 
	# executed through the python coverage program `coverage`
	ssh root@localhost -T -p 5555 <<-HEREDOC
	coverage run -p --include="*jailhouse*" $1
	echo $? > tmp
	HEREDOC

	# Get the result stored in tmp via scp, remove tmp on the instance
	scp -P 5555 root@localhost:tmp .
	ssh root@localhost -T -p 5555 "rm -f tmp"

	# Load value of tmp (on the host) into variable, remove tmp on the host,
	# return result	
	RES=$(cat tmp)
	rm -f tmp
	[ "$RES" == 0 ]
}

# ================================
# TESTS
# ================================

# BATS executes each of these in the order they're instantiated in.

# Although BATS has a setup() and teardown() function for each individual test,
# it lacks such setup / teardown functions that run before / after all tests.
# Thus, this "test" is used as a setup function.
@test "pre-testing setup" {

	# Some setup stuff 
	ssh root@localhost -T -p 5555 <<-HEREDOC
	mount -t 9p passthrough_dir dev
	rm -f .coverage*
	cd jailhouse
	make && make install
	HEREDOC
}

# The only legitimate test here - both instances of [ "$?" == "0" ] must
# evaluate to 0 for the test to succeed (the test fails whenever such an 
# expression evaluates to a non-zero value)
@test "intel" {

	run_python_script_with_coverage jailhouse/tools/jailhouse-hardware-check
	[ "$?" == "0" ]
	run_python_script_with_coverage jailhouse/tools/jailhouse-config-create sys.c
	[ "$?" == "0" ]
}

# Counterpart to @test "pre-testing setup"
@test "post-testing teardown" {

	# Pull coverage information
	scp -P 5555 root@localhost:.coverage.* .

	# Generate pretty coverage docs
	coverage combine .coverage.*
	sed -i "s/\/root\/dev/\/home\/oem\/programming\/jailhouse\/dev_env\/passthrough_dir/g" .coverage
	coverage html -i
}
