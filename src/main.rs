pub mod dx;
pub mod data;

fn main() {
    dx::init_d3d();
    
    let vertex_shader = dx::create_vertex_shader("shaders/shaders.hlsl", "vs_main");
    let input_layout = dx::create_input_layout(&[
        ("POS", dx::DxgiFormat::R32G32B32Float),
        ("NOR", dx::DxgiFormat::R32G32B32Float),
        ("TEX", dx::DxgiFormat::R32G32Float),
        ("COL", dx::DxgiFormat::R32G32B32Float),
    ], &vertex_shader);
    let pixel_shader = dx::create_pixel_shader("shaders/shaders.hlsl", "ps_main");
    
    let rasterizer_state = dx::create_rasterizer_state();
    let sampler_state = dx::create_sampler_state();
    let depth_stencil_state = dx::create_depth_stencil_state();
    let constant_buffer = dx::create_constant_buffer();
    
    let vertex_buffer = dx::create_vertex_buffer(&data::VERTEX_DATA);
    let index_buffer = dx::create_index_buffer(&data::INDEX_DATA);

    let texture_view = dx::create_texture_view(&data::TEXTURE_DATA, data::TEXTURE_WIDTH, data::TEXTURE_HEIGHT);

    let background_color: [f32; 4] = [0.025, 0.025, 0.025, 1.0];
    let stride = 11 * std::mem::size_of::<f32>() as u32;

    let viewport = dx::Viewport {
        top_left_x: 0.0,
        top_left_y: 0.0,
        width: dx::wnd_width() as f32,
        height: dx::wnd_height() as f32,
        min_depth: 0.0,
        max_depth: 1.0,
    };

    let w = dx::wnd_width() as f32 / dx::wnd_height() as f32;
    let h: f32 = 1.0;
    let n: f32 = 1.0;
    let f: f32 = 9.0;

    let mut model_rotation: [f32; 3] = [0.0, 0.0, 0.0];
    let model_scale: [f32; 3] = [1.0, 1.0, 1.0];
    let model_translation: [f32; 3] = [0.0, 0.0, 4.0];

    loop {
        let end = dx::message_loop();

        if end {
            break;
        }

        let transform = dx::get_transform(&model_rotation, &model_scale, &model_translation);
        let projection: [f32; 16] = [ 2.0 * n / w, 0.0, 0.0, 0.0, 0.0, 2.0 * n / h, 0.0, 0.0, 0.0, 0.0, f / (f - n), 1.0, 0.0, 0.0, n * f / (n - f), 0.0 ];
        let light: [f32; 3] = [1.0, -1.0, 1.0];

        model_rotation[0] += 0.005;
        model_rotation[1] += 0.009;
        model_rotation[2] += 0.001;

        let mut const_buffer_data: Vec<f32> = Vec::new();

        const_buffer_data.extend_from_slice(&transform);
        const_buffer_data.extend_from_slice(&projection);
        const_buffer_data.extend_from_slice(&light);

        dx::map_constant_buffer(&constant_buffer, &const_buffer_data);

        dx::clear_view(&background_color);
        dx::ia_set_buffers(&input_layout, &vertex_buffer, stride, &index_buffer);
        dx::set_vertex_shader(&vertex_shader, &constant_buffer);
        dx::set_rasterizer(&viewport, &rasterizer_state);
        dx::set_pixel_shader(&pixel_shader, &texture_view, &sampler_state);
        dx::set_render_target(&depth_stencil_state);

        dx::draw_indexed(data::INDEX_DATA.len() as u32);

        dx::present();
    }
}
