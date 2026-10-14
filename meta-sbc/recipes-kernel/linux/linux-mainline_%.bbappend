FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://goodix.cfg \
"

KERNEL_DTC_FLAGS:append = " -@"
