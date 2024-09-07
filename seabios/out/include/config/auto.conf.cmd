deps_config := \
	vgasrc/Kconfig \
	/home/byter/documents/repositories/kvm-new/seabios/src/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
