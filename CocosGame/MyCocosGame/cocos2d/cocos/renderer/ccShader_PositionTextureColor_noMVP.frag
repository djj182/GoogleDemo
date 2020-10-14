/*
 * cocos2d for iPhone: http://www.cocos2d-iphone.org
 *
 * Copyright (c) 2011 Ricardo Quesada
 * Copyright (c) 2012 Zynga Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

const char* ccPositionTextureColor_noMVP_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision lowp float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
}
);

const char* ccPositionTextureColor_text_noMVP_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision lowp float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
const float smoothing = 1.0/10.0;
                                                               
//float invert_value_ifless(in float value)
//{
//    float sign_value = sign(value);
//    float sign_value_squared = sign_value*sign_value;
//    return sign_value_squared / ( value + sign_value_squared - 1.0);
//}
                                                               
void main()
{
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
    float value = texColor.a;
    float sign_value = sign(value);
    float sign_value_squared = sign_value*sign_value;
    float invertAlpha = sign_value_squared / ( value + sign_value_squared - 1.0);
    vec3 srcTexColor = texColor.rgb * invertAlpha;
    
    float refineColor = smoothstep(0.1, 0.9, texColor.a);
    vec4 refineTexColor = vec4(srcTexColor.rgb * refineColor, refineColor);
    
    gl_FragColor = vec4(v_fragmentColor * refineTexColor);
}
);
