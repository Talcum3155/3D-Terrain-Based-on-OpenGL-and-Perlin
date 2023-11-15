# 3D-Terrain-Based-on-OpenGL-and-Perlin
### 选题名称：基于OpenGL与Perlin噪声生成3D仿真地形

#### 选题研究：
1.	Perlin噪声算法： 了解Perlin噪声算法的原理和应用，该算法用于生成地形的随机性和自然性
2.	OpenGL图形渲染： 学习如何使用OpenGL进行3D图形渲染，包括顶点和片段着色器的编写、纹理映射、光照等技术
3.	地形生成算法： 探讨各种地形生成算法以及如何将这些算法与Perlin噪声结合生成多样化的地形
4.	实时性与性能优化： 研究如何在实时环境下生成大规模、逼真的地形，包括Level of Detail（LOD）技术、渲染优化等
     
#### 开发工具：C++ 20、openGL 4.6

## Shader类

使用模板方法的设计模式，只需重写`create_compile_shader_delegate`函数就可以添加新的shader进去，不过在构造函数之中需要调整一下新shader的位置

### 参考资料：

#### Technical Support
1. https://learnopengl-cn.github.io/intro/

#### Height Map
1. Tessellation Chapter I: Rendering Terrain using Height Maps：https://learnopengl.com/Guest-Articles/2021/Tessellation/Height-map

#### Tessellation
1. Tessellation Chapter II: Rendering Terrain using Tessellation Shaders & Dynamic Levels of Detail： https://learnopengl.com/Guest-Articles/2021/Tessellation/Tessellation
2. 第三十课 曲面细分： https://doc.yonyoucloud.com/doc/wiki/project/modern-opengl-tutorial/tutorial30.html
3. Tessellation： https://www.khronos.org/opengl/wiki/Tessellation
4. ✠OpenGL-12-曲面细分： https://blog.csdn.net/itzyjr/article/details/119178559