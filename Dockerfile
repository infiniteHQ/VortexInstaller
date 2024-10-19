FROM ubuntu:24.04

# Mettre à jour les paquets
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install software-properties-common -y \
 && add-apt-repository ppa:graphics-drivers \
 && apt-get install nvidia-driver-440 -y \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Installer les paquets nécessaires
RUN apt-get install -y xorg nano xterm libx11-dev git libglfw3-dev make gcc libxi-dev libxcursor-dev libspdlog-dev vulkan-tools wget libglfw3 libvulkan-dev cmake libxinerama-dev unzip nvidia-driver-470 nlohmann-json3-dev

# Télécharger et installer VortexMaker
RUN mkdir -p ~/Downloads && \
    cd ~/Downloads && \
    wget --progress=dot:giga https://infinite.si/_b/VortexMaker.zip && \
    unzip VortexMaker.zip && \
    cd VortexMaker && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install

WORKDIR /root

ENV DISPLAY=:0
ENV XDG_RUNTIME_DIR=/run/user/1000


CMD ["/bin/sh", "-c", "cd /root/vx/ && vortex -g"]
#VOLUME build/vx:/root/vx Add with command in /root/vx. Handle it directly whatever origin of this project.
VOLUME /run/user/
VOLUME /tmp/.X11-unix


# BUILD : sudo docker build --no-cache -t vortex .