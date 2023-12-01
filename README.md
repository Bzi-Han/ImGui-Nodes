# ImGui-Nodes

#### 介绍

此代码基于[ChemistAion/nodes.cpp](https://gist.github.com/ChemistAion/0cd64b71711d81661344af040c142c1c)的基础上进行修改，修复了一些显示上的问题并增加了一些个人所需的功能。

#### 使用

将`modules`文件夹中的`ImGuiNodes.h`和`ImGuiNodes.cc`复制到你的项目中或者以`git submodule`的方式引入你的项目中即可。

支持Header Only集成方式，只需要定义编译预处理器宏`IMGUI_NODES_HEADER_ONLY`即可。

#### 操作键

+ 鼠标左键
  + 单击：选择节点
  + 拖拽：移动选择节点、拖拽连线、批量选中节点
  + 双击：展开或收缩节点
  + Ctrl/Shift + 单击：选中多个节点
+ 鼠标右键
  + 单击：显示创建节点菜单
  + 双击：切换节点关闭或打开状态
+ 鼠标滚轮
  + 长按并移动：移动画布
  + 滚动：上下滚动画布
  + Shift + 滚动：左右滚动画布
  + Ctrl + 滚动：缩放画布
+ 键盘
  + Home键：移动画布到中心并还原缩放比例
  + Delete键：删除所有选中节点

#### 编译测试程序

+ 0.确保在项目根目录打开命令行终端 。
+ 1.执行`git submodule update --init` 。
+ 2.执行`cmake -S . -B build`。
+ 3.执行`cmake --build build --config Debug`。
+ 4.编译完成。

#### 演示

![screenshot01.jpg](https://github.com/Bzi-Han/ImGui-Nodes/blob/main/images/screenshot01.jpg)

![screenshot02.jpg](https://github.com/Bzi-Han/ImGui-Nodes/blob/main/images/screenshot02.jpg)
