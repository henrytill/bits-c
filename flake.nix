{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    let
      src = builtins.path {
        path = ./.;
        name = "bits-src";
      };
    in
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        mkBitsPackage =
          {
            nativeBuildInputs ? [ ],
          }:
          pkgs.stdenv.mkDerivation ({
            pname = "bits";
            version = "0.1.0";

            inherit src;

            doCheck = true;

            inherit nativeBuildInputs;

            buildInputs = with pkgs; [
              openssl
            ];
          });

        mkBitsShell =
          package:
          {
            packages ? [ ],
            shellHook ? "",
          }:
          pkgs.mkShell {
            inputsFrom = [ package ];
            packages =
              with pkgs;
              [
                clang-tools
              ]
              ++ packages;
            inherit shellHook;
          };
      in
      {
        packages = {
          bits-zig = mkBitsPackage {
            nativeBuildInputs = with pkgs; [
              zig_0_15.hook
            ];
          };

          bits-meson = mkBitsPackage {
            nativeBuildInputs = with pkgs; [
              meson
              ninja
              pkg-config
            ];
          };

          default = self.packages.${system}.bits-meson;
        };

        devShells = {
          bits-zig-shell = mkBitsShell self.packages.${system}.bits-zig {
            packages = with pkgs; [
              zls
            ];
            shellHook = ''
              export ZIG_GLOBAL_CACHE_DIR=$(mktemp -d)
            '';
          };

          bits-meson-shell = mkBitsShell self.packages.${system}.bits-meson { };

          default = self.devShells.${system}.bits-meson-shell;
        };
      }
    );
}
