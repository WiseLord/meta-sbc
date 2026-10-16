SUMMARY = "Builds minimal initramfs"

require recipes-core/images/core-image-minimal.bb

#IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
IMAGE_FSTYPES = "cpio.gz.u-boot"

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

IMAGE_INSTALL:append = " kernel-module-r61581"

#IMAGE_INSTALL:append = " kernel-modules"
#IMAGE_INSTALL:append = " linux-firmware"

IMAGE_INSTALL:append = " i2c-tools"
IMAGE_INSTALL:append = " wpa-supplicant"

IMAGE_INSTALL:append = " dt-overlays"
IMAGE_INSTALL:append = " sbc-init"

IMAGE_INSTALL:append = " lvgl-demo-fb"

ROOTFS_POSTPROCESS_COMMAND:append = " create_bpi_firmware_symlinks"

create_bpi_firmware_symlinks() {
    mkdir -p ${IMAGE_ROOTFS}/lib/firmware/brcm/
    ln -sf ../cypress/cyfmac43430-sdio.bin ${IMAGE_ROOTFS}/lib/firmware/brcm/brcmfmac43430-sdio.sinovoip,bpi-m2-zero.bin
}

ROOTFS_POSTPROCESS_COMMAND:append = " remove_boot_dir_from_initramfs"

remove_boot_dir_from_initramfs() {
    rm -rf ${IMAGE_ROOTFS}/boot
    mkdir -p ${IMAGE_ROOTFS}/boot
}