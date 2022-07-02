# Hana-SoftwareRenderer

## 关于我实现的软渲染器

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_final_screenshot.png)

开发语言：C++

开发环境：Windows 10 - Microsoft Visual Studio 2022 17.2.1

### 控制

- 按W或S可以切换场景
- 长按鼠标左键可以旋转摄像机
- 长按鼠标右键可以移动摄像机
- 按空格键可以复位摄像机
- 按A或D可以旋转灯光方向
- 按E可以切换阴影开关
- 按Q可以切换Shader

## 什么是软渲染

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_GPU_Pinpline_Stage.png)

在了解什么是软渲染之前，我们首先要明白，在正常渲染流程中，有很多阶段是由GPU固定实现的，只有部分阶段可供我们进行配置和编程，那么对于不可修改的阶段，我们很多时候只能从书本上通过理论知识去了解片面的内容，对于更加详细的细节想必是抱有一定的疑问的。

写软渲染器就是我们自己用代码去模拟这个流程，通过自己动手实现一遍，去彻底理清GPU究竟帮我们做了什么步骤，这就是我们写软渲染的学习目的（当然，只是在理解基本步骤上）。

软渲染的实现过程大致是这样：

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_implementation_process.png)

我们只借助一个画像素点的API，其余一切自己实现，最终达到最后一张图片的效果，这是我们的实现目的。

## 前置知识

除了基本的C++基础外，写软渲染是需要具备基本的图形学基础的，在这我推荐以下这些资料：

### GAMES101

[GAMES101-现代计算机图形学入门-闫令琪](https://www.bilibili.com/video/BV1X7411F744)

目前图形学入门中文视频课程中的最强课程

看完GAMES101前9课即可，后续课程与实现光栅化的渲染器关系不大。前9课程已经清晰讲过向量、矩阵、变换、渲染管线、着色等知识，学习完毕后便具备动手实操的理论知识。然后建议完成作业1至作业3，作业提供了一个比较完整的框架，其中某些关键函数会留空，让我们根据课堂内容自行去实现，并得出要求的结果，其中涉及到MVP矩阵构建、包围盒、三角形内外判断、深度缓冲、纹理映射、凹凸映射等关键步骤，这个时候我们对渲染流程已经有比较明确的认识了。

### 《Unity Shader入门精要》

另外推荐结合《Unity Shader入门精要》一起阅读，不需要读完整本书，看到第七章基础纹理即可，书本前部分内容与GAMES101前面课程高度重合，后面会讲一些shader相关的东西，因为我们需要在软渲染器上实现可编程的渲染管线，包括vertex shader和fragment shader，这要求我们对shader本身有一定的理解，个人认为跟着这本书在unity上动手实现一下blinn phong和纹理等内容，然后带着一些流程上的疑问，例如vertex shader之前发生了什么？为什么vertex shader一定要输出裁剪空间坐标？vertex shader和fragment shader之间又发生了什么？等问题我们再回头实现我们的软渲染，相信理解会更加透彻。

[《Unity Shader入门精要》- 第四章 数学基础](https://candycat1992.github.io/unity_shaders_book/unity_shaders_book_chapter_4.pdf)

入门精要的第四章数学内容是免费公开的，上面是作者Github中给出的第四章数学篇链接，个人认为这是很简洁很易懂的图形学数学基础资料，非常适合新手，我也看过诸如《3D数学基础 图形和游戏开发》这类专业数学工具书，但内容不够精炼，翻译过于生硬，且表达方式确实不易于国人理解，不适合新手入门。如果因为GAMES101是视频，翻阅起来不方便，那么配合这里的数学篇一起阅读即可，基本满足我们实现基础的软渲染所需的数学知识，美中不足的是这里都没涉及到逆矩阵的计算，对于任意n阶方阵的逆矩阵计算，稍微有点复杂，这是需要我们自行实现的，需要另外翻阅资料去了解。

### 3Blue1Brown - 线性代数的本质

[3Blue1Brown - 线性代数的本质 - 系列合集](https://www.bilibili.com/video/BV1ys411472E)

相信不少人在初学的时候跟我一样，虽然知道了矩阵的各种性质和运算，但是仍然不能理解矩阵里这些神奇的数字是究竟是怎么帮助我们完成了变换的，这个系列的视频用动态的方式生动地演示了，矩阵是如何在空间中作用的。

## 工程实践的参考资料

### TinyRenderer

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_tinyrenderer.jpg)

[Tiny Renderer or how OpenGL works: software rendering in 500 lines of code](https://github.com/ssloy/tinyrenderer)

国外大佬的一个开源教程，通过500行代码实现一个软渲染，让你去理解诸如OpenGL这类图形API内部是如何工作的。

如果说GAMES101带你完成了软渲染的关键步骤，那么TinyRenderer将会带你从零去构建一个软渲染，实际上他的实现过程就是上文配图`软渲染实现过程`的过程。

建议可以搭配以下文章一起食用，不错的讲解：

[Shawoxo - 从零构建光栅器，tinyrenderer笔记（上）](https://zhuanlan.zhihu.com/p/399056546)

[Shawoxo - 从零构建光栅器，tinyrenderer笔记（下）](https://zhuanlan.zhihu.com/p/400791821)

有同学可能会提出疑问，既然TinyRenderer教程教我们从0开始构建软渲染，那么我们是不是可以跳过GAMES101直接从TinyRenderer入手呢？实际上这个系列教程主要是教你工程实践，对于图形学本身所需要的理论知识是讲得没那么详细的，如果你跟我一样并不是什么天赋异禀的选手，建议还是从GAMES101开始看起。

### 知乎热贴 - 如何开始用 C++ 写一个光栅化渲染器

[如何开始用 C++ 写一个光栅化渲染器？](https://www.zhihu.com/question/24786878)

这个贴集中了各路大佬提供的实现思路以及他们的开源demo，可以选一个自己喜欢的进行参考，其中对我帮助最大的是zauonlok的回答：

[如何开始用 C++ 写一个光栅化渲染器？ - zauonlok的回答](https://www.zhihu.com/question/24786878/answer/820931418)

![](https://pica.zhimg.com/80/v2-5d84718cdd38cb31a49b796603df84a5_1440w.jpg?source=1940ef5c)

zauonlok给出了功能很强大的软渲染实现示例，并在回答中指出了TinyRenderer中一些可改进的地方，并且对于一些容易忽略的细节盲点，在回答中和评论区中也给出了答疑和参考。我实现的软渲染也是在TinyRenderer基础上再参考zauonlok给出的开源软渲染提取出部分内容出来结合改进。

**以上提到的资料中，GAMES101以及TinyRenderer为重要资料，其他为辅助资料**

## 实现过程

我们先根据TinyRenderer教程来实现以下步骤

1. 利用画点API实现画线
2. 利用画线API实现画实心三角形
3. 实现OBJ模型数据读取，根据数据画出模型三角形面
4. 利用zbuffer建立正确的深度数据，从而正确地绘制模型的前后关系
5. 实现透视投影
6. 实现摄像机的观察变换
7. 实现基础Shader，包括基础光照模型、纹理映射、切线空间下的法线映射等。
8. 实现ShadowMap

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_tinyrederer_final.png)

不出意外你将会得到跟我上图一样的效果，此时基本的软渲染器已经完成了。

这个时候，还存在很多可改进优化的地方，下面列举以下我做的优化。

### 改进1：实时渲染和摄像机控制

第一大痛点就是，TinyRenderer是把渲染结果输出到tga图片上的，相当于是个离线渲染，有时候渲染的问题是需要调整角度观察才能发现的，这给我们的调试带来了很多的麻烦，我们第一步需要改进的就是接上图形界面，做到实时渲染，并且接上输入信号，实现摄像机控制。


![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_tinyrenderer_orbit_camera.gif)

这两步我都是直接参考zauonlok软渲工程中的代码，其中图形界面是调用了win32的API，而摄像机则是参考另一个环绕摄像机控制的开源库，这两步做完后，我们便得到上图中的控制效果。

### 改进2：优化Shader流程

我们首先来看一下TinyRenderer中的一个Shader代码(当然我们讨论的都是用C++写的Shader，而不是特别的Shader语言)

```cpp
struct GouraudShader : public IShader {
    mat<3,3,float> varying_tri;
    Vec3f          varying_ity;

    virtual ~GouraudShader() {}

    virtual Vec3i vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        gl_Vertex = Projection*ModelView*gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));

        varying_ity[nthvert] = CLAMP(model->normal(iface, nthvert)*light_dir, 0.f, 1.f);

        gl_Vertex = Viewport*gl_Vertex;
        return proj<3>(gl_Vertex/gl_Vertex[3]);
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_ity*bar;
        color = TGAColor(255, 255, 255)*intensity;
        return false;
    }
};
```

可以看到，流程上有好些不规范的地方，例如没有单独的数据结构体，而是把数据储存在Shader中，vs中会直接访问模型本身，而不是只拿到一个顶点数据结构，而vs到fs中的数据插值过程则在fs中处理，实际这个插值过程是外部函数在处理的，我们可以尝试把这些问题改进。

```cpp
shader_struct_v2f BlinnShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.world_pos = ObjectToWorldPos(a2v->obj_pos);
	v2f.world_normal = ObjectToWorldNormal(a2v->obj_normal);
	v2f.uv = a2v->uv;
	return v2f;
}

bool BlinnShader::fragment(shader_struct_v2f* v2f, Color& color) {
	Vector3f worldNormalDir = (v2f->world_normal).normalize();
	Color albedo = tex_diffuse(v2f->uv) * shader_data->matrial->color;
	Color ambient = shader_data->ambient * albedo;
	float n_dot_l = saturate(worldNormalDir * WorldLightDir());
	Color diffuse = shader_data->light_color * albedo * n_dot_l;
	Vector3f viewDir = WorldSpaceViewDir(v2f->world_pos).normalize();
	Vector3f halfDir = (viewDir + WorldLightDir()).normalize();
	Color spcular = shader_data->light_color * shader_data->matrial->specular * std::pow(saturate(worldNormalDir * halfDir), shader_data->matrial->gloss);

	Vector4f depth_pos = shader_data->light_vp_matrix * embed<4>(v2f->world_pos);
	int shadow = is_in_shadow(depth_pos, n_dot_l);

	color = ambient + (diffuse + spcular) * shadow;
	return false;
}
```

在我的实现中，我尝试把vs和fs两个阶段封装成可以类似Unity的Shader中的写法，这样可以更有助于理解Shader函数外引擎帮我们处理了什么。

### 改进3：场景与物体

细心的同学可能发现了TinyRenderer中似乎没怎么讨论模型变换，关于MVP矩阵中，MV矩阵是合成了一个矩阵，名为ModelView，而这个矩阵的实现实则只包含了View矩阵，其实他是默认模型原点正处于世界空间原点，并且不考虑旋转和缩放，一旦我们涉及到多个物体或者单个物体的旋转和缩放便无法满足需求。

这个时候我们也可以仿照游戏引擎的规则，建立物体概念，储存每个物体的位置、旋转、缩放等信息，并建立场景概念，把所有物体归于场景管理。

```cpp
struct Transform {
	Vector3f position;
	Vector3f rotation;
	Vector3f scale;
};

class GameObject {
public:
	Transform transform;
	GameObject(Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
	virtual void tick(float delta_time);
	Matrix4x4 GetModelMatrix();
};
```

构建Transform结构体，其中包含位置、旋转、缩放信息，然后GameObject必须包含一个Transform成员，针对每个GameObject都有各自的模型变换矩阵。

至于Scene的实现我就不赘述了，主要是管控物体的生成销毁，包括模型、灯光、摄像机等。

### 改进4：接入输入信号实现场景控制

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_tinyrenderer_switch_shader.gif)

参考摄像机的控制，我们可以借助win32的API接收输入信号，实现一些输入控制逻辑，如上图我们实现了通过键盘输入切换Shader和切换场景（切换模型）的逻辑。当然，我们甚至可以控制物体移动、光照方向、阴影开关等，在我们接入了上一步的场景和物体管理后，这些都可以轻松实现。

### 改进5：GUI输出场景信息

功能上已经实现得差不多了，现在我们可以把UI文本也实现了，实时输出一些必要的场景信息，可以方便我们了解当前场景状态，做进一步的功能实现和调试。

![](https://blog-1300673521.cos.ap-guangzhou.myqcloud.com/HANA-SoftwareRenderer_final.gif)

把帧率、摄像机和灯光的transform状态信息以及一些控制指导输出到屏幕上，另外加入了灯光方向的实时控制，这就是我的软渲染器的最终效果了！

### 关于其他的优化

实际上的优化不止上面提及到的部分，例如：
1. 在透视矩阵上，TinyRenderer的实现是一个简化后的实现，其甚至不包括远近裁面的变量
2. 代码实现上，比起TinyRenderer，zauonlok的示例更符合OpenGL的实现，我们可以多做参考
3. 添加材质实现
4. 加入背面剔除
5. 加入齐次裁剪

基于前文提到的前置知识和参考资料，我们在实现的过程仍然会遇到一些没被上述资料详细讨论的内容，我们可能需要借助一些额外的资料去了解：

1. 透视矫正
2. 齐次裁剪

[透视矫正插值和图形渲染管线总结 - 孙小磊的文章](https://zhuanlan.zhihu.com/p/144331875)

[一篇文章彻底弄懂齐次裁剪 - Clawko的文章](https://zhuanlan.zhihu.com/p/102758967)

由于时间关系，我实现的软渲染也没有加上齐次裁剪，等哪天想回来做v2版本时，再来填这个坑吧。

## 结语

实现软渲染是一件非常有趣的事，希望本文在你实现软渲染的过程中能帮到你，enjoy it！

