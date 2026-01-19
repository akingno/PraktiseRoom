# Room Simulation

## 核心特性 (Features)

* **多智能体系统**: 支持多个 Agent 在同一场景下独立决策。
* **需求驱动 (Need-based)**: 角色拥有饥饿、疲劳、无聊等生理指标，自动驱动行为。
* **行为链架构 (Action Chain)**: 复杂的动作（如“去吃饭”）被分解为原子动作序列（寻路 -> 移动 -> 交互）。
* **社交互动**: 实现了“发起-响应”式的社交模型，支持记忆交换。

##  架构文档 (Architecture)

* ### [1. 核心类与层级关系 (Class Hierarchy)](doc/1_Class_Hierarchy.md)
  了解 `Agent`, `ActionExecutor`, `Blackboard` 等核心类的持有与引用关系。

* ### [2. 游戏主循环流程 (Game Loop Flow)](doc/2_Game_Loop.md)
  每一帧是如何从环境刷新、到 AI 决策、再到动作执行的数据流向。

* ### [3. 社交交互流程 (Social Interaction Flow)](doc/3_Social_Flow.md)
  详解 Agent A 如何发起对话、Agent B 如何被动响应以及记忆传输的时序图。
