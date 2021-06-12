ifeq ($(CONTRIB_DIR),)
INC_DIR += $(call select_from_repositories,include/nspr)
INC_DIR += $(call select_from_repositories,include/nss)
else
INC_DIR += $(call select_from_ports,nss3)/include/nspr
INC_DIR += $(call select_from_ports,nss3)/include/nss
INC_DIR += $(call select_from_repositories,include/nss)
endif
