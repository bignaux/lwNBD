#with import (fetchTarball channel:nixos-19.03) {};
with import <nixpkgs> {}; # nix-shell -I nixpkgs=/home/genesis/devel/nixpkgs

stdenv.mkDerivation rec{
  name = "lwNBD";

  nativeBuildInputs = [  pkgconfig doxygen ];

  buildInputs =
	[
	  python3Packages.docopt
	  eclipses.eclipse-cpp
	  autoreconfHook
	  ronn
	] ++ checkInputs;

  checkInputs = [
      #todo : filecompare
      libnbd
      clang-tools
		  gdb
		  gdbgui # r -c nixos-logo-b\&w.png -o nixos.h
		  clang-tools
		];
}
