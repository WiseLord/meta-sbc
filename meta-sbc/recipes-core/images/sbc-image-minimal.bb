SUMMARY = "Builds minimal image."

require recipes-core/images/core-image-minimal.bb

WKS_FILES = "sbc-sdcard-image.wks.in"

IMAGE_INSTALL:append = " kernel-module-bluetooth"
IMAGE_INSTALL:append = " kernel-module-brcmfmac"
IMAGE_INSTALL:append = " kernel-module-brcmfmac-wcc"
IMAGE_INSTALL:append = " kernel-module-brcmutil"
IMAGE_INSTALL:append = " kernel-module-btbcm"
IMAGE_INSTALL:append = " kernel-module-btintel"
IMAGE_INSTALL:append = " kernel-module-cfg80211"
IMAGE_INSTALL:append = " kernel-module-ecc"
IMAGE_INSTALL:append = " kernel-module-ecdh-generic"
IMAGE_INSTALL:append = " kernel-module-hci-uart"
IMAGE_INSTALL:append = " kernel-module-sha256"

IMAGE_INSTALL:append = " linux-firmware-bcm43430"
IMAGE_INSTALL:append = " wireless-regdb-static"

#IMAGE_INSTALL:append = " kernel-modules"
#IMAGE_INSTALL:append = " linux-firmware"

IMAGE_INSTALL:append = " dt-overlays"

IMAGE_BOOT_FILES:append = " devicetree/*.dtbo;devicetree/"

IMAGE_PREPROCESS_COMMAND:append = " create_uenv_txt"

create_uenv_txt() {
    install -d ${DEPLOY_DIR_IMAGE}
    cat << EOF > ${DEPLOY_DIR_IMAGE}/uEnv.txt
overlays=sun8i-h3-eth0
EOF
}

IMAGE_BOOT_FILES:append = " uEnv.txt"

ROOTFS_POSTPROCESS_COMMAND:append = " create_bpi_firmware_symlinks"

create_bpi_firmware_symlinks() {
    mkdir -p ${IMAGE_ROOTFS}/lib/firmware/brcm/
    ln -sf ../cypress/cyfmac43430-sdio.bin ${IMAGE_ROOTFS}/lib/firmware/brcm/brcmfmac43430-sdio.sinovoip,bpi-m2-zero.bin
}
