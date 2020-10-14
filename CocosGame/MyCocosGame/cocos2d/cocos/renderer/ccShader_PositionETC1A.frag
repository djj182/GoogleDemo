"                                                           \n\
#ifdef GL_ES												\n\
precision lowp float;										\n\
#endif														\n\
varying vec4 v_fragmentColor;								\n\
varying vec2 v_texCoord;									\n\
void main()                                                 \n\
{                                                           \n\
	vec4 v4Colour = texture2D(CC_Texture0, v_texCoord);     \n\
    v4Colour.a = texture2D(CC_Texture1, v_texCoord).r; \n\
//    v4Colour.xyz = v4Colour.xyz * v4Colour.a;               \n\
    gl_FragColor = v4Colour * v_fragmentColor;              \n\
}                                                           \n\
";
