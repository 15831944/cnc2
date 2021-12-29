sudo raspi-config

sudo apt update
sudo apt upgrade

sudo apt install rpi-update
sudo rpi-update
reboot

sudo apt dist-upgrade

sudo apt install gcc g++
sudo apt install clang clang++
sudo apt install cmake
sudo apt install xterm
sudo apt install git-core
sudo apt install subversion
sudo apt install vim
sudo apt install gtkterm

sudo usermod -a -G dialout <username>
sudo usermod -a -G dialout pi

sudo apt-get install build-essential
sudo apt install qt5-default
sudo apt install qtdeclarative5-dev
sudo apt install qtcreator

sudo apt-get install qt5-doc
sudo apt-get install qt5-doc-html qtbase5-doc-html
sudo apt-get install qtbase5-examples

Qtcreator > Help > about plugins > untick Remote Linux > reboot
Qtcreator > Tools > Options > Build and Run" > add compilers /usr/bin/gcc

sudo apt install libqt5serialport5
sudo apt install libqt5serialport5-dev
sudo apt install libgl-dev

sudo apt install g++8
sudo apt install gcc-8

sudo apt install clang++-7 clang-7 lldb-7 lld-7
sudo apt install clang++-6.0 clang-6.0 lldb-6.0 lld-6.0

sudo apt install libqwt5*
sudo apt-get install libqt5svg5-dev
sudo apt-get install libqt5svg5*

//
sudo apt install libfontconfig1-dev libdbus-1-dev libfreetype6-dev libudev-dev libicu-dev libsqlite3-dev
sudo apt install libxslt1-dev libssl-dev libasound2-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev
sudo apt install libgstreamer-plugins-base0.10-dev gstreamer-tools gstreamer0.10-plugins-good gstreamer0.10-plugins-bad
sudo apt install libraspberrypi-dev libpulse-dev libx11-dev libglib2.0-dev libcups2-dev freetds-dev libsqlite0-dev libpq-dev
sudo apt install libiodbc2-dev libmysqlclient-dev firebird-dev libpng12-dev libjpeg9-dev libgst-dev libxext-dev libxcb1 libxcb1-dev
sudo apt install libx11-xcb1 libx11-xcb-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-image0 libxcb-image0-dev libxcb-shm0
sudo apt install libxcb-shm0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-sync1 libxcb-sync-dev libxcb-render-util0
sudo apt install libxcb-render-util0-dev libxcb-xfixes0-dev libxrender-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-glx0-dev
sudo apt install libxi-dev libdrm-dev libssl-dev libxcb-xinerama0 libxcb-xinerama0-dev

# LLVM
sudo apt install libllvm-8-ocaml-dev libllvm8 llvm-8 llvm-8-dev llvm-8-doc llvm-8-examples llvm-8-runtime
# Clang and co
sudo apt install clang-8 clang-tools-8 clang-8-doc libclang-common-8-dev libclang-8-dev libclang1-8 clang-format-8 python-clang-8
# libfuzzer
sudo apt install libfuzzer-8-dev
# lldb
sudo apt install lldb-8
# lld (linker)
sudo apt install lld-8
# libc++
sudo apt install libc++-8-dev libc++abi-8-dev
# OpenMP
sudo apt install libomp-8-dev

kdiff3
krename
p7zip-full
kate
krusader

In directory /etc/ld.so.conf.d
Make your own file myqwt.conf with path /usr/local/qwt-6.1.5/lib
sudo ldconfig

linuxdeployqt
export PATH=/..qt/bin/:$PATH

# sudo apt install default-jre
# sudo apt install openjfx
# sudo apt-get update
# sudo apt-get install oracle-java11-installer-local
# sudo apt-get install oracle-java11-set-default-local

sudo cp jdk-8u261-linux-x64.tar.gz /usr/java
sudo tar xvzf jdk-8u261-linux-x64.tar.gz
sudo gedit /etc/profile

export JAVA_HOME=/usr/java/jdk1.8.0.261
export PATH $JAVA_HOME/bin:$PATH


# sudo apt-get libusb*
sudo apt install stlink-tools

# ByteBlaster
# https://www.intel.com/content/www/us/en/programmable/support/support-resources/download/drivers/dri-usb_b-lnx.html
lsusb
create file /etc/udev/rules.d/52-usbblaster.rules
SUBSYSTEMS=="usb", ATTRS{idVendor}=="09fb", ATTRS{idProduct}="6010", MODE=="0666"

wget https://packages.microsoft.com/config/ubuntu/21.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb

sudo apt-get update; \
  sudo apt-get install -y apt-transport-https && \
  sudo apt-get update && \
  sudo apt-get install -y dotnet-sdk-5.0

sudo apt-get update; \
  sudo apt-get install -y apt-transport-https && \
  sudo apt-get update && \
  sudo apt-get install -y aspnetcore-runtime-5.0

sudo apt-get install -y dotnet-runtime-5.0