{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    pkgs.glfw 
    pkgs.glew
    pkgs.gtk4
    pkgs.pkg-config


    pkgs.gcc
    pkgs.meson
    pkgs.ninja
  ];

  
}