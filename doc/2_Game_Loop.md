```markdown
# 游戏主循环流程 (Game Loop Flow)

本文档描述了每一帧（Tick）内，数据是如何从环境流向 Agent，并最终转化为屏幕上的动作的。

## 流程图

```text
Start Tick
    │
    ▼
1. Environment Update
    │   • ItemLayer 刷新食物 (ensureFoodSpawned)
    │
    ▼
2. Agent Update (遍历每个 Agent)
    │
    ├── A. 生理更新 (Character::tickNeeds)
    │       • 饥饿++，疲劳++，无聊++ (随时间自然增长)
    │       • 如果正在睡觉/玩电脑 -> 对应属性下降
    │
    ├── B. 大脑决策 (Agent::decideAction) [Utility AI]
    │       • 计算各行为分数 (Score):
    │           - Hunger Score (饿了吗？有饭吗？)
    │           - Sleep Score (困了吗？有床吗？)
    │           - Boredom Score (无聊吗？有电脑吗？)
    │           - Talk Score (随机触发 or 正在聊天中?)
    │           - Stop Score (正在发呆中?)
    │       • 取最高分 -> Character::setAct(BestAct)
    │
    ├── C. 构建上下文 (ActExecutorCtx)
    │       • 打包资源: Room, Items, Pathfinder, OtherAgents...
    │
    └── D. 执行动作 (ActionExecutor::tick)
            │
            ├── 1. 切换检测
            │       • 如果 (CurrentAct != DesiredAct) -> 打断当前动作!
            │       • 工厂创建新动作链 (createActionChain)
            │         Example: Eat -> Sequence[MoveToFood, Interact]
            │
            └── 2. 动作推进 (CurrentAction->tick)
                    • 如果是 Sequence: 依次执行子动作
                    • 如果是 MoveTo: 调用寻路，移动一步
                    • 如果是 Wait: 倒计时
                    • 如果动作完成 -> 返回 Success/Failure -> 清理 currentAction
    │
    ▼
3. Rendering
    • 绘制地图、物品、所有 Agent