#ifdef GL_ES
precision mediump float;
#endif

uniform float wave_phase;
uniform vec2  wave_amplitude;

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_textureMatrix;
uniform mat4 sf_u_modelViewMatrix;

attribute vec4 sf_a_color;
attribute vec2 sf_a_position;
attribute vec2 sf_a_texCoord;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    vec2 newPosition = sf_a_position;

    newPosition.x += cos(newPosition.y * 0.02 + wave_phase * 3.8) * wave_amplitude.x +
                     sin(newPosition.y * 0.02 + wave_phase * 6.3) * wave_amplitude.x * 0.3;

    newPosition.y += sin(newPosition.x * 0.02 + wave_phase * 2.4) * wave_amplitude.y +
                     cos(newPosition.x * 0.02 + wave_phase * 5.2) * wave_amplitude.y * 0.3;

    gl_Position   = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(newPosition, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
}
