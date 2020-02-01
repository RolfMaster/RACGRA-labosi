#version 420 core

uniform sampler2D glossmap;
uniform sampler3D spheremap3d;

in VS_OUT
{
    vec3 normal;
    vec3 view;
    vec2 tc;
} fs_in;

out vec4 color;

void main(void)
{
    vec3 reflected = reflect(normalize(fs_in.view), normalize(fs_in.normal));
    float m = 1.0 / (2.0 * sqrt(reflected.x*reflected.x 
                            + reflected.y*reflected.y
                            + pow(reflected.z + 1.0, 2.0)
                            )
                    );
    vec2 st = reflected.xy * m + vec2(0.5);

    vec4 glossmap_sample = texture(glossmap, fs_in.tc * vec2(3.0, 1.0) * 0.5);
    float gloss = length(glossmap_sample);

    vec4 glossy = texture(spheremap3d, vec3(st, 0));
    vec4 clear = texture(spheremap3d, vec3(st, 1));
    
    color = mix(clear, glossy, gloss);
   // color = texture(glossmap, fs_in.tc);
   
}
