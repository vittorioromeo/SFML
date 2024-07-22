#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;
uniform float     blur_radius;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    vec2 offx = vec2(blur_radius, 0.0);
    vec2 offy = vec2(0.0, blur_radius);

    // clang-format off
    vec4 pixel = texture2D(sf_u_texture, sf_v_texCoord.xy)               * 4.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx)        * 2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx)        * 2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offy)        * 2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offy)        * 2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx - offy) * 1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx + offy) * 1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx - offy) * 1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx + offy) * 1.0;
    // clang-format on

    gl_FragColor = sf_v_color * (pixel / 16.0);
}
