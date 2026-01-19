```markdown
# 社交交互流程 (Social Interaction Flow)

本文档详解了两个智能体之间如何建立交互。这是一个典型的 **生产者-消费者** 模型，其中发起者（Producer）主动修改接收者（Consumer）的状态。

## 场景描述
Agent A (Initiator) 决定找 Agent B (Receiver) 聊天。

## 时序图

```text
[Agent A] (Initiator)                     [Agent B] (Receiver)
    │                                         │
    │ (decideAction 随机触发 Talk)             │ (正在 Wander 或 Idle)
    ▼                                         │
1. SelectAgentAction                          │
    • 从列表选定 B 为目标                      │
    • bb.target_agent = B                     │
    │                                         │
    ▼                                         │
2. SignalChatAction                           │
    • 调用 B->receiveCall(A) ---------------->• 设置 B.bb.is_being_called = true
    │                                         │
    ▼                                         │
3. MoveToAction (Target::Character)           ▼
    • 寻路走到 B 的坐标                     (下一帧 decideAction)
    │                                         • 发现 is_being_called == true
    │                                         • 强制 Act = WaitAlways
    │                                         • 执行 WaitForChatAction (死循环等待)
    ▼                                         │
4. TransferMemoryAction                       │
    • 获取 A 的最新记忆                        │
    • A->getName() + " said: " + memory       │
    • 写入 B 的记忆 (B->headMessage) --------->• B 增加一条记忆
    • B->finishChat() ----------------------->• B.is_being_called = false
    │                                         • B.Act 恢复为 Wander
    ▼                                         │
5. WaitAction (30 ticks)                      ▼
    • 假装聊了一会儿                           • 恢复自由，继续之前的逻辑
    │
    ▼
6. ChangeToAction(Wander)
    • A 结束聊天，切回 Wander
关键机制