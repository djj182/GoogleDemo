"                                                                   \n\
#ifdef GL_ES                                                        \n\
precision lowp float;                                               \n\
#endif                                                              \n\
varying vec4 v_fragmentColor;                                       \n\
varying vec2 v_texCoord;                                            \n\
uniform sampler2D CC_Texture_Alpha;                                 \n\
void main()                                                         \n\
{                                                                   \n\
	vec4 v4Colour = texture2D(CC_Texture0, v_texCoord);             \n\
    v4Colour.a = texture2D(CC_Texture_Alpha, v_texCoord).r;         \n\
    v4Colour.xyz = v4Colour.xyz * v4Colour.a;                       \n\
    float gray = dot(v4Colour.rgb, vec3(0.212, 0.715, 0.072));      \n\
    gl_FragColor = vec4(gray, gray, gray, v4Colour.a) * v_fragmentColor;     \n\
}                                                                   \n\
";
