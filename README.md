# meta-jannik

Custom Yocto layer for spectral analyzer application with systemd autostart.

## Requirements

- PHYTEC BSP-Yocto (Scarthgap)
- Base layer dependencies: `phytec`, `openembedded-layer`

## Contents

- `recipes-apps/jannik-spectral/` - Simple spectral analyzer application with systemd service
- `recipes-images/images/jannik-image.bb` - Custom image based on phytec-headless-image

## Setup

1. Download and extract the PHYTEC BSP-Yocto
2. Clone this layer into `sources/meta-jannik`
3. Add to `build/conf/bblayers.conf`:
   ```
   BBLAYERS += "/workdir/sources/meta-jannik"
   ```
4. Build the image:
   ```bash
   bitbake jannik-image
   ```

## Application

The `jannik-spectral` application is a simple C program that logs periodically. It's automatically started via systemd and will restart if it crashes.
