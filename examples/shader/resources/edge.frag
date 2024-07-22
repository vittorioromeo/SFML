#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;
uniform float     edge_threshold;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    const float offset = 1.0 / 512.0;

    vec2 offx = vec2(offset, 0.0);
    vec2 offy = vec2(0.0, offset);

    // clang-format off
    vec4 hEdge = texture2D(sf_u_texture, sf_v_texCoord.xy - offy)        * -2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offy)        *  2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx - offy) * -1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx + offy) *  1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx - offy) * -1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx + offy) *  1.0;
    // clang-format on

    // clang-format off
    vec4 vEdge = texture2D(sf_u_texture, sf_v_texCoord.xy - offx)        *  2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx)        * -2.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx - offy) *  1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy - offx + offy) * -1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx - offy) *  1.0 +
                 texture2D(sf_u_texture, sf_v_texCoord.xy + offx + offy) * -1.0;
    // clang-format on

    vec3 result = sqrt(hEdge.rgb * hEdge.rgb + vEdge.rgb * vEdge.rgb);

    float edge = length(result);

    vec4 pixel = sf_v_color * texture2D(sf_u_texture, sf_v_texCoord.xy);

    if (edge > (edge_threshold * 8.0))
        pixel.rgb = vec3(0.0, 0.0, 0.0);
    else
        pixel.a = edge_threshold;

    gl_FragColor = pixel;
}
