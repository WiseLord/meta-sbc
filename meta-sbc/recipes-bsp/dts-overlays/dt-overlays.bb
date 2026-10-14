SUMMARY = "Devicetree overlays"
DESCRIPTION = "Handle custom devicetree overlays"

inherit devicetree

SRC_URI = " \
    file://sun8i-h3-eth0.dts \
"

COMPATIBLE_MACHINE  = "^(bananapi-m2-zero)$"
