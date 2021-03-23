# ParagonAnimation
Paragon Animation Plugins

本工程仅供个人测试使用，不负责任何稳定性或者功能完善。

本插件主要提供了一个基础动画蓝图，大致实现了SlopeLean，AimOffsets，停止动画DistanceMatching，CardinalDirection等，但都还只是雏形。

使用方法：
1. 从官方商店免费下载Paragon的资源（其他资源也可以，但Paragon的动画最全）；
2. 下载本插件；
3. 选中Paragon资源的骨架，新建动画蓝图，在动画蓝图的Class Settings里修改Parent Class为插件里的ParagonAnimBP；
4. 在Anim Graph Overrides里选好Paragon资源的的动画；
5. 创建一个蓝图类，父类选择插件里的ParagonCharacter，设置好Mesh和AnimBP即可。

前置条件：
1. 记得修改引擎里的SParentPlayerTreeRow::OnShouldFilterAsset，才能让子动画蓝图可以拖放AnimationAsset到Anim Graph Overrides进去。
