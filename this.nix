
{ pkgs ? import <nixpkgs> {} }:

let
  libX11 = pkgs.xorg.libX11;
in

pkgs.mkShell {
  buildInputs = with pkgs; [

    # tools
    linuxPackages_latest.perf
    pkg-config
    valgrind
    clang-tools
    clang
    gdb

    # libs
    xorg.libX11
    fontconfig
    freetype
    cairo
    inih
  ];

  shellHook = ''
    export CFLAGS="-I${libX11.dev}/include"
    export LDFLAGS="-L${libX11}/lib"
  '';
}

