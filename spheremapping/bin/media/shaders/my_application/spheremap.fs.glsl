#version 420 core

uniform sampler2D spheremap;

in VS_OUT{
    vec3 normal;
    vec3 view;
} fs_in;

out vec4 color;

void main(void){    
    vec3 reflected = reflect(normalize(fs_in.view), normalize(fs_in.normal));
    float m = 1.0 / (2.0 * sqrt(reflected.x*reflected.x 
                            + reflected.y*reflected.y
                            + pow(reflected.z + 1.0, 2.0)
                            )
                    );
    vec2 st = reflected.xy * m + vec2(0.5);
    color = texture(spheremap, st);
}