if test -e ${devtype} ${devnum}:${distro_bootpart} uEnv.txt; then
    echo "Found uEnv.txt, loading environment..."
    load ${devtype} ${devnum}:${distro_bootpart} ${loadaddr} uEnv.txt
    env import -t ${loadaddr} ${filesize}
fi

setenv bootargs console=${console} console=tty1 root=/dev/mmcblk0p2 rootwait panic=10 ${extra}

load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} ${fdtfile}
load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} uImage

echo "Applying ${overlay_prefix} overlays: ${overlays}"

fdt addr ${fdt_addr_r}
fdt resize 65536

for overlay in ${overlays}; do
    if test -e ${devtype} ${devnum}:${distro_bootpart} /devicetree/${overlay_prefix}-${overlay}.dtbo; then
        echo "Loading overlay: ${overlay_prefix}-${overlay}"
        load ${devtype} ${devnum}:${distro_bootpart} ${fdtoverlay_addr_r} /devicetree/${overlay_prefix}-${overlay}.dtbo
        fdt apply ${fdtoverlay_addr_r}
    else
        echo "Warning: overlay /devicetree/${overlay_prefix}-${overlay}.dtbo not found!"
    fi
done

bootm ${kernel_addr_r} - ${fdt_addr_r}
