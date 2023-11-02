use std::{ffi::{OsStr, CString}, os::windows::prelude::OsStrExt};

use libc::{c_void, c_char};

extern crate libc;

#[derive(Copy, Clone)]
pub enum DxgiFormat {
    R32G32B32Float = 6,
    R32G32Float = 16,
}

#[repr(C)]
pub struct Viewport {
    pub top_left_x: f32,
    pub top_left_y: f32,
    pub width: f32,
    pub height: f32,
    pub min_depth: f32,
    pub max_depth: f32,
}

#[repr(C)]
pub struct VertexShaderHandle {
    blob: *mut c_void,
    shader: *mut c_void,
}

#[repr(C)]
pub struct InputLayoutHandle {
    layout: *mut c_void,
}

#[repr(C)]
struct InputElementDescInternal {
    semantic_name: *const c_char,
    format: u32,
}

#[repr(C)]
pub struct PixelShaderHandle {
    blob: *mut c_void,
    shader: *mut c_void,
}

#[repr(C)]
pub struct RasterizerStateHandle {
    state: *mut c_void,
}

#[repr(C)]
pub struct SamplerStateHandle {
    state: *mut c_void,
}

#[repr(C)]
pub struct DepthStencilStateHandle {
    state: *mut c_void,
}

#[repr(C)]
pub struct BufferHandle {
    buffer: *mut c_void,
}

#[repr(C)]
pub struct TextureViewHandle {
    view: *mut c_void,
}

#[link(name = "dx11", kind = "static")]
extern "C" {
    fn dx_test() -> i32;
    fn dx_set_pfn(cb: extern fn()) -> ();
    fn dx_init_d3d() -> ();
    fn dx_wnd_width() -> i32;
    fn dx_wnd_height() -> i32;
    fn dx_create_vertex_shader(path: *const libc::wchar_t, entry_point: *const libc::c_char) -> VertexShaderHandle;
    fn dx_create_input_layout(input_element_desc: *const InputElementDescInternal, num_elements: u32, vertex_shader: *const VertexShaderHandle) -> InputLayoutHandle;
    fn dx_create_pixel_shader(path: *const libc::wchar_t, entry_point: *const libc::c_char) -> PixelShaderHandle;
    fn dx_create_rasterizer_state() -> RasterizerStateHandle;
    fn dx_create_sampler_state() -> SamplerStateHandle;
    fn dx_create_depth_stencil_state() -> DepthStencilStateHandle;
    fn dx_create_constant_buffer() -> BufferHandle;
    fn dx_create_vertex_buffer(data: *const c_void, size: u32) -> BufferHandle;
    fn dx_create_index_buffer(data: *const c_void, size: u32) -> BufferHandle;
    fn dx_create_texture_view(data: *const c_void, width: u32, height: u32) -> TextureViewHandle;
    fn dx_map_constant_buffer(buffer: *const BufferHandle, data: *const c_void, size: u32) -> ();
    fn dx_clear_view(background_color: *const f32) -> ();
    fn dx_ia_set_buffers(input_layout: *const InputLayoutHandle, vertex_buffer: *const BufferHandle, stride: u32, index_buffer: *const BufferHandle) -> ();
    fn dx_set_vertex_shader(vertex_shader: *const VertexShaderHandle, constant_buffer: *const BufferHandle) -> ();
    fn dx_set_rasterizer(viewport: *const Viewport, rasterizer_state: *const RasterizerStateHandle) -> ();
    fn dx_set_pixel_shader(pixel_shader: *const PixelShaderHandle, texture_view: *const TextureViewHandle, sampler_state: *const SamplerStateHandle) -> ();
    fn dx_set_render_target(depth_stencil_state_handle: *const DepthStencilStateHandle) -> ();
    fn dx_draw_indexed(index_count: u32) -> ();
    fn dx_present() -> ();
    fn dx_message_loop() -> bool;
    fn dx_get_transform(model_rotation: *const f32, model_scale: *const f32, model_translation: *const f32, data: *mut *mut f32) -> ();
}

pub fn test() {
    unsafe {
        dx_test();
    }
}

pub fn set_pfn(cb: extern fn()) {
    unsafe {
        dx_set_pfn(cb);
    }
}

pub fn to_utf16(s: &str) -> Vec<u16> {
    let mut v: Vec<u16> = OsStr::new(s).encode_wide().collect();

    v.push(0);

    v
}

pub fn init_d3d() {
    unsafe {
        dx_init_d3d();
    }
}

pub fn wnd_width() -> i32 {
    unsafe {
        dx_wnd_width()
    }
}

pub fn wnd_height() -> i32 {
    unsafe {
        dx_wnd_height()
    }
}

pub fn create_vertex_shader(path: &str, entry_point: &str) -> VertexShaderHandle {
    let shader_path: Vec<u16> = to_utf16(path);
    let c_entry_point = CString::new(entry_point).unwrap();

    unsafe {
        dx_create_vertex_shader(shader_path.as_ptr() as *const libc::wchar_t, c_entry_point.as_ptr() as *const libc::c_char)
    }
}

pub fn create_input_layout(input_element_desc: &[(&str, DxgiFormat)], vertex_shader: &VertexShaderHandle) -> InputLayoutHandle {
    let mut input_element_desc_internal: Vec<InputElementDescInternal> = Vec::new();

    for desc in input_element_desc {
        let name = CString::new(desc.0).unwrap();

        input_element_desc_internal.push(InputElementDescInternal {
            semantic_name: name.as_ptr() as *const c_char,
            format: desc.1 as u32,
        });
    }

    unsafe {
        dx_create_input_layout(input_element_desc_internal.as_ptr(), input_element_desc_internal.len() as u32, vertex_shader as *const _)
    }
}

pub fn create_pixel_shader(path: &str, entry_point: &str) -> PixelShaderHandle {
    let shader_path: Vec<u16> = to_utf16(path);
    let entry_point = CString::new(entry_point).unwrap();

    unsafe {
        dx_create_pixel_shader(shader_path.as_ptr() as *const libc::wchar_t, entry_point.as_ptr() as *const libc::c_char)
    }
}

pub fn create_rasterizer_state() -> RasterizerStateHandle {
    unsafe {
        dx_create_rasterizer_state()
    }
}

pub fn create_sampler_state() -> SamplerStateHandle {
    unsafe {
        dx_create_sampler_state()
    }
}

pub fn create_depth_stencil_state() -> DepthStencilStateHandle {
    unsafe {
        dx_create_depth_stencil_state()
    }
}

pub fn create_constant_buffer() -> BufferHandle {
    unsafe {
        dx_create_constant_buffer()
    }
}

pub fn create_vertex_buffer(data: &[f32]) -> BufferHandle {
    unsafe {
        dx_create_vertex_buffer(data.as_ptr() as *const c_void, (data.len() * std::mem::size_of::<f32>()) as u32)
    }
}

pub fn create_index_buffer(data: &[u32]) -> BufferHandle {
    unsafe {
        dx_create_index_buffer(data.as_ptr() as *const c_void, (data.len() * std::mem::size_of::<u32>()) as u32)
    }
}

pub fn create_texture_view(data: &[u32], width: u32, height: u32) -> TextureViewHandle {
    unsafe {
        dx_create_texture_view(data.as_ptr() as *const c_void, width, height)
    }
}

pub fn map_constant_buffer(buffer: &BufferHandle, data: &[f32]) {
    unsafe {
        dx_map_constant_buffer(buffer as *const _, data.as_ptr() as *const c_void, (data.len() * std::mem::size_of::<f32>()) as u32)
    }
}

pub fn clear_view(background_color: &[f32]) {
    unsafe {
        dx_clear_view(background_color.as_ptr())
    }
}

pub fn ia_set_buffers(input_layout: &InputLayoutHandle, vertex_buffer: &BufferHandle, stride: u32, index_buffer: &BufferHandle) {
    unsafe {
        dx_ia_set_buffers(input_layout as *const _, vertex_buffer as *const _, stride, index_buffer as *const _)
    }
}

pub fn set_vertex_shader(vertex_shader: &VertexShaderHandle, constant_buffer: &BufferHandle) {
    unsafe {
        dx_set_vertex_shader(vertex_shader as *const _, constant_buffer as *const _)
    }
}

pub fn set_rasterizer(viewport: &Viewport, rasterizer_state: &RasterizerStateHandle) {
    unsafe {
        dx_set_rasterizer(viewport as *const _, rasterizer_state as *const _)
    }
}

pub fn set_pixel_shader(pixel_shader: &PixelShaderHandle, texture_view: &TextureViewHandle, sampler_state: &SamplerStateHandle) {
    unsafe {
        dx_set_pixel_shader(pixel_shader as *const _, texture_view as *const _, sampler_state as *const _)
    }
}

pub fn set_render_target(depth_stencil_state_handle: &DepthStencilStateHandle) {
    unsafe {
        dx_set_render_target(depth_stencil_state_handle as *const _)
    }
}

pub fn draw_indexed(index_count: u32) {
    unsafe {
        dx_draw_indexed(index_count)
    }
}

pub fn present() {
    unsafe {
        dx_present()
    }
}

pub fn message_loop() -> bool {
    unsafe {
        dx_message_loop()
    }
}

pub fn get_transform(model_rotation: &[f32; 3], model_scale: &[f32; 3], model_translation: &[f32; 3]) -> [f32; 16] {
    let mut data: [f32; 16] = [3.0; 16];

    unsafe {
        dx_get_transform(model_rotation.as_ptr(), model_scale.as_ptr(), model_translation.as_ptr(), &mut data.as_mut_ptr());
    }

    println!("{:?}", data);

    data
}
