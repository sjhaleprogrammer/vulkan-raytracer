{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
    #c libraries
    pkgs.glfw 
    pkgs.glew
    pkgs.gtk4
    pkgs.pkg-config

    #c compiler
    pkgs.gcc
   
    #build tools
    pkgs.meson
    pkgs.ninja
    
    #debugger
    pkgs.gdb
    
  ];

  
}