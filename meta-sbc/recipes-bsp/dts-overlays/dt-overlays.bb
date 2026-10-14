SUMMARY = "Devicetree overlays"
DESCRIPTION = "Handle custom devicetree overlays"

inherit devicetree

SRC_URI = " \
    file://sun8i-h3-eth0.dts \
    file://sun8i-h3-gt911.dts \
    file://sun8i-h3-r61851.dts \
"

COMPATIBLE_MACHINE  = "^(bananapi-m2-zero)$"
