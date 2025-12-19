DESCRIPTION = "LVGL-based application built with CMake for framebuffer display"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "alsa-lib lvgl"
RDEPENDS:${PN} = "alsa-lib"

SRC_URI = " \
    file://cmake \
"

OECMAKE_SOURCEPATH = "${WORKDIR}/cmake"
EXTRA_OECMAKE = "-DUSE_SDL_BACKEND=0"

inherit cmake

do_install() {
    # Install the binary from CMake build output
    install -d ${D}${bindir}
    install -m 0755 ${B}/bin/orchid-spectral ${D}${bindir}/orchid-lvgl
}

FILES:${PN} = " \
    ${bindir}/orchid-lvgl \
"
