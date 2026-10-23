{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
    zon2nix = {
      url = "github:jcollie/zon2nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };
  outputs =
    {
      self,
      nixpkgs,
      utils,
      zon2nix,
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
            postPatch ? "",
          }:
          pkgs.stdenv.mkDerivation ({
            pname = "bits";
            version = "0.1.0";

            inherit src;

            doCheck = true;

            inherit nativeBuildInputs postPatch;

            buildInputs = with pkgs; [
              libpng
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
            hardeningDisable = [ "fortify" ];
            inherit shellHook;
          };
      in
      {
        packages = {
          bits-zig = mkBitsPackage {
            nativeBuildInputs = with pkgs; [
              zig_0_17.hook
            ];
            postPatch = ''
              cp -rsL --no-preserve=mode ${pkgs.callPackage ./build.zig.zon.nix { }} zig-pkg
            '';
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
              zon2nix.packages.${system}.zon2nix
            ];
            shellHook = ''
              cp -frsL --no-preserve=mode ${pkgs.callPackage ./build.zig.zon.nix { }} zig-pkg
              export ZIG_GLOBAL_CACHE_DIR=$(mktemp -d)
            '';
          };

          bits-meson-shell = mkBitsShell self.packages.${system}.bits-meson { };

          default = self.devShells.${system}.bits-meson-shell;
        };
      }
    );
}
