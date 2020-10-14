
const char* ccPositionTextureColor_constrast_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision lowp float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform mat4 CC_filterMat;
void main()
{
    vec4 value = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = CC_filterMat*value;
}
);

