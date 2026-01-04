#with import (fetchTarball channel:nixos-19.03) {};
with import <nixpkgs> { };

stdenv.mkDerivation rec{
  name = "lwNBD";

  nativeBuildInputs = [ pkg-config doxygen ];

  buildInputs =
    [
      python3Packages.docopt
      python3Packages.libnbd
      autoreconfHook
      ronn
      libuv
      kconfig-frontends
      #wolfssl
      SDL2_ttf
      SDL2
      bin2c
    ] ++ checkInputs;

  checkInputs = [
    #todo : filecompare
    eclipses.eclipse-cpp
    libsForQt5.kdevelop
	libnbd
    clang-tools
    python3Packages.distutils
    checkmake
    #curl
    #gcov
    gdb
    #gdbgui
    iaito
    iperf2
    ncurses
    wrk
    bridge-utils
    hdparm
    shellcheck
   # libguestfs-with-appliance
    valgrind
  ];
}
