#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;
uniform float     blink_alpha;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    vec4 pixel = sf_v_color;
    pixel.a    = blink_alpha;

    gl_FragColor = pixel;
}
