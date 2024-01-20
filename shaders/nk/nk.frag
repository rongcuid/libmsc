#version 460

#extension GL_EXT_nonuniform_qualifier:require

layout(set=0,binding=0)uniform sampler2D textures[];
layout(set=1,binding=0)uniform PerDrawData{
    int textureId;
};

layout(location=0)in vec2 inUV;

layout(location=0)out vec4 outColor;

void main(){
    outColor=texture(textures[textureId],inUV);
}