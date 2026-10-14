FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://common.cfg \
    file://goodix.cfg \
    file://r61851.cfg \
"

KERNEL_DTC_FLAGS:append = " -@"
