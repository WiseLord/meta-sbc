SUMMARY = "Builds minimal image"

inherit core-image

IMAGE_INSTALL = "packagegroup-core-boot"
IMAGE_LINGUAS = " "

IMAGE_FSTYPES = "wic.gz"

SUNXI_BOOT_SPACE = "120"
WKS_FILES = "sbc-sdcard-image.wks.in"

IMAGE_BOOT_FILES:append = " devicetree/*.dtbo;devicetree/"
IMAGE_BOOT_FILES:append = " sbc-initramfs-minimal-${MACHINE}.rootfs.cpio.gz.u-boot;uramfs"

IMAGE_PREPROCESS_COMMAND:append = " create_uenv_txt"

create_uenv_txt() {
    install -d ${DEPLOY_DIR_IMAGE}
    cat << EOF > ${DEPLOY_DIR_IMAGE}/uEnv.txt
overlay_prefix=sun8i-h3
overlays=eth0 gt911 r61851
ramdisk=uramfs
extra=fbcon=rotate:2
EOF
}

IMAGE_BOOT_FILES:append = " uEnv.txt"
