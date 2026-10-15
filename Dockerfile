FROM archlinux:latest

RUN pacman -Syu --noconfirm --needed \
        base-devel git cmake ninja python alsa-lib cmake hidapi ibus jack libdecor libthai fribidi libgl libpulse libusb libx11 libxcursor libxext libxfixes libxi libxinerama libxkbcommon libxrandr libxrender libxss libxtst mesa ninja pipewire sndio vulkan-driver vulkan-headers wayland wayland-protocols \
    && pacman -Scc --noconfirm \
    && git config --system --add safe.directory '*'

WORKDIR /src

ENV BUILD_DIR=build \
	BUILD_TYPE=Release \
	X86_VERSION=* \
	CONFIGURE_FLAGS="" \
	BUILD_FLAGS="" \
    ENABLE_STEAMWORKS=OFF \
    STEAMWORKS_APP_ID=480 \
    HOME=/tmp

ENTRYPOINT ["/bin/bash", "-c", "set -e; scripts/build.py --build-dir=\"$BUILD_DIR\" --build-type=\"$BUILD_TYPE\" \
			--x86-version=$X86_VERSION --configure-flags=\"$CONFIGURE_FLAGS\" --build-flags=\"$BUILD_FLAGS\"", "--"]
