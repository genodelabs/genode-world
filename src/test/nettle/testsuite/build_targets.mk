TARGET   := tests

tests : aes-test arcfour-test arctwo-test base16-test base64-test bcrypt-test blowfish-test buffer-test camellia-test cast128-test cbc-test \
 ccm-test cfb-test chacha-poly1305-test chacha-test cmac-test cnd-memcpy-test ctr-test curve25519-dh-test curve448-dh-test cxx-test des3-test des-test 

mkfile_path := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

%-test:
	echo $@
	mkdir $(mkfile_path)$@
	sed 's/@@test_exe@@/$@/g' $(mkfile_path)nettle_test.template > $(mkfile_path)$@/target.mk
# 
