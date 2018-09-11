#!/usr/bin/env bats

PASS_DIR=~/programming/jailhouse/dev_env/passthrough_dir
DISK=~/programming/jailhouse/dev_env/dev_disk.qcow2

run_intel() {
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

	ssh root@localhost -T -p 5555 ls &> /dev/null
	RET=$?

	while [ "$RET" != "0" ]; do
		ssh root@localhost -T -p 5555 ls &> /dev/null
		RET=$?
		sleep 5
	done

	ssh root@localhost -T -p 5555 <<-HEREDOC
	mount -t 9p passthrough_dir dev
	HEREDOC
}

run_amd() {
	/home/oem/Desktop/qemu/build/x86_64-softmmu/qemu-system-x86_64 \
		-machine q35,kernel_irqchip=split -m 1G \
		-smp 4 \
		-cpu Opteron_G5,+svm,+npt \
		-hda ${DISK} \
		-netdev user,id=net,hostfwd=tcp::5555-:22 -device e1000e,addr=2.0,netdev=net \
		-fsdev local,id=pass_thr,path=${PASS_DIR},security_model=passthrough \
		-device virtio-9p-pci,fsdev=pass_thr,addr=1f.7,mount_tag=passthrough_dir \
		-device amd-iommu \
		-daemonize -pidfile pid

	ssh root@localhost -T -p 5555 ls &> /dev/null
	RET=$?

	while [ "$RET" != "0" ]; do
		ssh root@localhost -T -p 5555 ls &> /dev/null
		RET=$?
		sleep 5
	done

	ssh root@localhost -T -p 5555 <<-HEREDOC
	mount -t 9p passthrough_dir dev
	HEREDOC
}

teardown() {
	# Shutdown, prepare for next instance
	ssh root@localhost -T -p 5555 <<-HEREDOC
	shutdown now
	HEREDOC

	while [ "$(ps aux | grep $(sudo cat pid) | grep -v grep)" != "" ]; do
		sleep 5
	done

	sudo rm -f pid
}

test_tmp_rm_tmp() {
	scp -P 5555 root@localhost:tmp .
	ssh root@localhost -T -p 5555 "rm -f tmp"
	RES=$(cat tmp)
	rm -f tmp
	[ "$RES" == 0 ]
}

test_hw_check() {
	ssh root@localhost -T -p 5555 <<-HEREDOC
	coverage run -p --include="*jailhouse*" jailhouse/tools/jailhouse-hardware-check
	echo $? > tmp
	HEREDOC

	test_tmp_rm_tmp
}

test_config_create() {
	ssh root@localhost -T -p 5555 <<-HEREDOC
	coverage run -p --include="*jailhouse*" jailhouse/tools/jailhouse-config-create sys.c
	echo $? > tmp
	HEREDOC

	test_tmp_rm_tmp
}

@test "pre-testing setup" {

	run_intel

	# Some setup stuff 
	ssh root@localhost -T -p 5555 <<-HEREDOC
	mount -t 9p passthrough_dir dev
	rm -f .coverage*
	cd jailhouse
	make && make install
	HEREDOC
}

@test "intel" {

	run_intel

	test_hw_check
	[ "$?" == "0" ]
	test_config_create
	[ "$?" == "0" ]
}

@test "amd" {
	skip

	run_amd

	test_hw_check
	[ "$?" == "0" ]
	test_config_create
	[ "$?" == "0" ]
}

@test "post-testing teardown" {

	run_intel

	# Pull coverage information, generate docs, shutdown machine
	scp -P 5555 root@localhost:.coverage.* .
	coverage combine .coverage.*
	sed -i "s/\/root\/dev/\/home\/oem\/programming\/jailhouse\/dev_env\/passthrough_dir/g" .coverage
	coverage html -i
}
