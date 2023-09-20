#with import (fetchTarball channel:nixos-19.03) {};
with import <nixpkgs> { };

stdenv.mkDerivation rec{
  name = "lwNBD";

  nativeBuildInputs = [ pkg-config doxygen ];

  buildInputs =
    [
      python3Packages.docopt
      autoreconfHook
      ronn
      libuv
      #wolfssl
    ] ++ checkInputs;

  checkInputs = [
    #todo : filecompare
    eclipses.eclipse-cpp
    libnbd
    clang-tools
    checkmake
    #curl
    gdb
    gdbgui
    iaito
    iperf2
    wrk
    bridge-utils
    hdparm
    shellcheck
   # libguestfs-with-appliance
  ];
}
