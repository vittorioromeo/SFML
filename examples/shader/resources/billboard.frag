#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    gl_FragColor = texture2D(sf_u_texture, sf_v_texCoord.st);
}
