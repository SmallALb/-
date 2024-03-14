# shader的概念

## 介绍

对于图像顶点和片元的处理行为，他有着自己的一套逻辑，

有着自己的针对GPU的编程语言(glsl)   称为着shader色器语言

最主要的shader为 vertexShader（顶点着色器）& FragmentShader（片段着色器）


举例：比如我想画一片地形，我只传入了一片在三维空间的长方形，但是我要的是一片凹凸起伏的地形，

所以我可以对每个传入gpu管线的顶点进行一些数学上的处理，让他凹凸不平

这就需要对vertexShader进行编程。


经典的着色器代码：

```c
#version 450
//vertexshader
layout (location = 0) in vec4 inPos;
layout (location = 2) in vec3 inColor;
layout (binding = 0) uniform UBO
{
mat4 projection;
mat4 model;
mat4 view;
} ubo;
layout (location = 0) out vec3 outColor;
void main()
{
gl_Position = ubo.projection * ubo.view * ubo.model * inPos;
outColor = inColor;
}
```

```c
#version 450
//片段着色
layout(location=0) in vec3 fragColor;
layout(location=0) out vec4 outColor;
void main() {
outColor = vec4(fragColor, 1.0);
}
```
