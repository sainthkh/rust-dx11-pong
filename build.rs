extern crate cmake;

fn main() {
    let dst = cmake::build("lib");

    println!("cargo:rustc-link-search=native={}/lib", dst.display());
    
    println!("cargo:rustc-link-lib=static=user32");
    println!("cargo:rustc-link-lib=static=d3d11");
    println!("cargo:rustc-link-lib=static=d3dcompiler");
    println!("cargo:rustc-link-lib=static=dx11");
}
