# Blender插件

## DLL模块
1. 定义接受数据的机构体，相关定义在`SkCommon.h`中
    * `BMaterial`：材质相关数据
    * `BTransform`：物体的位置信息
    * `BMesh`：顶点及索引数据，并包括`BMaterial`和`BTransform`
    * `BLight`：光源的相关数据
    * `BScene`：包括`BMesh`和`BLight`，及各自的个数
2. 添加入口函数，该函数接受一个`BScene`的指针
3. 编译至dll文件

## 插件脚本

1. 使用`ctypes`包构建相应的结构类
2. 使用blender内置包bpy读取数据，并创建相应的C结构对象：
    1. `bpy.data.objects`代表当前场景中的所有物体
    2. `object.type`表示当前物体的类型，例如`'MESH'`、`'LIGHT'`等
    3. `object.data.vertices`为物体顶点数据的集合
    4. `object.data.polygons`为物体所有面信息
    5. `polygon.vertices`为当前面的顶点序列
    6. `object.data.materials`为物体的材质集合
3. 加载DLL库
4. 获取C结构对象的指针，调用DLL函数

