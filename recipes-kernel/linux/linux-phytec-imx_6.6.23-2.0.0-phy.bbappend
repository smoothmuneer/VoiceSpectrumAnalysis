FILESEXTRAPATHS:prepend := "${THISDIR}/linux-phytec-6.6:"

SRC_URI:append = " file://audio.scc file://audio.cfg file://overlays/imx8mp-phyboard-pollux-inmp441.dtso"

KERNEL_FEATURES:append = " audio.scc"

KERNEL_DEVICETREE_OVERLAYS:append = " overlays/imx8mp-phyboard-pollux-inmp441.dtso"
