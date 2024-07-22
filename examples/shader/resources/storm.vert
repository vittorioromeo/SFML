#ifdef GL_ES
precision mediump float;
#endif

uniform vec2  storm_position;
uniform float storm_total_radius;
uniform float storm_inner_radius;

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

    vec2 offset = newPosition.xy - storm_position;

    float len = length(offset);
    if (len < storm_total_radius)
    {
        float push_distance = storm_inner_radius + len / storm_total_radius * (storm_total_radius - storm_inner_radius);
        newPosition.xy      = storm_position + normalize(offset) * push_distance;
    }

    gl_Position   = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(newPosition, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;

    gl_PointSize = 2.0;
}
