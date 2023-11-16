# 3D-Terrain-Based-on-OpenGL-and-Perlin
### 选题名称：基于OpenGL与Perlin噪声生成3D仿真地形

#### 选题研究：
1.	Perlin噪声算法： 了解Perlin噪声算法的原理和应用，该算法用于生成地形的随机性和自然性
2.	OpenGL图形渲染： 学习如何使用OpenGL进行3D图形渲染，包括顶点和片段着色器的编写、纹理映射、光照等技术
3.	地形生成算法： 探讨各种地形生成算法以及如何将这些算法与Perlin噪声结合生成多样化的地形
4.	实时性与性能优化： 研究如何在实时环境下生成大规模、逼真的地形，包括Level of Detail（LOD）技术、渲染优化等
     
#### 开发工具：C++ 20、openGL 4.6

## 前置

### Shader类

使用模板方法的设计模式，只需重写`create_compile_shader_delegate`函数就可以添加新的shader进去，不过在构造函数之中需要调整一下新shader的位置

### 摄像机

并不存在真正的摄像机概念，而是通过将所有物体根据输入的方向反向移动来模拟出摄像机前进后退的效果

使用三个线性无关的互相正交单位向量来表示以摄像机为原点的坐标系，这三个单位向量分别是：`UP`、`FORWARD`、`RIGHT`，只需要其中两个向量即可，第三个向量可以使用叉积计算。其中最关键的`FORWARD`向量，需要根据鼠标操作明确计算出来，`FORWARD`与世界的`WORLD_UP`叉积计算出`RIGHT`向量，最后再用`RIGHT`与`FORWARD`叉积计算出`UP`向量



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