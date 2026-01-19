```markdown
# 核心类与层级关系 (Class Hierarchy)

本文档展示了项目中各个类的持有权（Ownership）和引用关系。

## 架构图

```text
Main Loop (main.cpp)
 │
 ├── [Owns] Room (地图数据, 墙壁)
 ├── [Owns] ItemLayer (物品管理: 床, 电脑, 食物)
 ├── [Owns] AStarPathfinder (寻路工具)
 │
 └── [Owns] std::vector<Agent> (所有智能体)
             │
             ▼
           Agent (智能体容器)
             ├── [Owns] Character (身体与属性)
             │           ├── 属性: _hunger, _fatigue, _boredom
             │           ├── 状态: Act (Eat, Sleep, Talk...)
             │           └── 记忆: ShortMemory
             │
             ├── [Owns] Blackboard (黑板/工作记忆)
             │           ├── 导航: path, target_pos
             │           ├── 社交: is_being_called, caller_agent
             │           └── 状态: currentAction (当前正在执行的动作对象)
             │
             └── [Owns] ActionExecutor (执行器)
                         │
                         └── [Manages] Action (当前动作树根节点)
                                     ├── SequenceAction (序列组合)
                                     ├── MoveToAction (移动逻辑)
                                     ├── InteractAction (交互逻辑)
                                     └── ... (Talk, Wait, etc.))