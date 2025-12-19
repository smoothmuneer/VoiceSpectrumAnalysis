DESCRIPTION = "Simple spectral analyzer application with systemd autostart"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://jannik-spectral.service \
"

inherit systemd

SYSTEMD_SERVICE:${PN} = "jannik-spectral.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    # Install systemd service file
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/jannik-spectral.service ${D}${systemd_system_unitdir}/jannik-spectral.service
}

FILES:${PN} += " \
    ${systemd_system_unitdir}/jannik-spectral.service \
"
