# Custom image with autostart spectral analyzer
SUMMARY = "Custom image with jannik-spectral analyzer and systemd autostart"
LICENSE = "MIT"

# Include the base phytec-headless-image
require recipes-images/images/phytec-headless-image.bb

# Add the spectral analyzer application to the image
IMAGE_INSTALL:append = " jannik-spectral"