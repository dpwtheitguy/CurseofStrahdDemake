#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

if [[ $(dpkg --print-architecture) != amd64 ]]; then
    echo "This bootstrap pins Ubuntu 24.04 amd64 packages. See README for system tools." >&2
    exit 1
fi

if [[ -d .toolchain/noble-amd64 ]]; then
    exit 0
fi

mkdir -p .toolchain
scratch=$(mktemp -d .toolchain/staging.XXXXXX)
trap 'rm -rf "$scratch"' EXIT
(
    cd "$scratch"
    apt-get download \
        gcc-arm-none-eabi=15:13.2.rel1-2 \
        binutils-arm-none-eabi=2.42-1ubuntu1+23 \
        libretro-mgba=0.10.2+dfsg-1.1build3 libinih1=55-1ubuntu2
    for package in ./*.deb; do
        dpkg-deb -x "$package" root
    done
)
mv -T "$scratch/root" .toolchain/noble-amd64
