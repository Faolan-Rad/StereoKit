#pragma once

#include "../stereokit.h"

namespace sk {

bool defaults_init    ();
void defaults_shutdown();

extern tex_t        sk_default_tex;
extern tex_t        sk_default_tex_black;
extern tex_t        sk_default_tex_gray;
extern tex_t        sk_default_tex_flat;
extern tex_t        sk_default_tex_rough;
extern tex_t        sk_default_cubemap;
extern mesh_t       sk_default_quad;
extern mesh_t       sk_default_screen_quad;
extern mesh_t       sk_default_sphere;
extern mesh_t       sk_default_cube;
extern shader_t     sk_default_shader;
extern shader_t     sk_default_shader_pbr;
extern shader_t     sk_default_shader_pbr_clip;
extern shader_t     sk_default_shader_unlit;
extern shader_t     sk_default_shader_unlit_clip;
extern shader_t     sk_default_shader_font;
extern shader_t     sk_default_shader_equirect;
extern shader_t     sk_default_shader_ui;
extern shader_t     sk_default_shader_ui_box;
extern shader_t     sk_default_shader_ui_quadrant;
extern shader_t     sk_default_shader_sky;
extern shader_t     sk_default_shader_lines;
extern shader_t     sk_default_shader_blit_linear;
extern material_t   sk_default_material;
extern material_t   sk_default_material_pbr;
extern material_t   sk_default_material_pbr_clip;
extern material_t   sk_default_material_unlit;
extern material_t   sk_default_material_unlit_clip;
extern material_t   sk_default_material_equirect;
extern material_t   sk_default_material_font;
extern material_t   sk_default_material_ui;
extern material_t   sk_default_material_ui_box;
extern material_t   sk_default_material_ui_quadrant;
extern material_t   sk_default_material_blit_linear;
extern font_t       sk_default_font;
extern text_style_t sk_default_text_style;
extern sound_t      sk_default_click;
extern sound_t      sk_default_unclick;
extern sound_t      sk_default_grab;
extern sound_t      sk_default_ungrab;

} // namespace sk