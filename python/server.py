from fastapi import FastAPI
from pydantic import BaseModel
from typing import List, Optional
import random
import uvicorn
import asyncio

app = FastAPI()

# 1. 定义与 C++ 端对应的 JSON 数据模型
class AgentState(BaseModel):
    name: str
    hunger: float
    fatigue: float
    boredom: float
    nowTick: int
    hasFood: bool
    hasBed: bool
    hasComputer: bool
    memories: List[str] # 接收 C++ 传来的记忆列表

class DecisionResponse(BaseModel):
    action: str
    thought: str      # LLM 的思考过程，将存入 ShortMemory
    target: Optional[str] = None # 预留：如果 LLM 指定了具体目标

# 2. 模拟 LLM 的随机逻辑
@app.post("/decide", response_model=DecisionResponse)
async def decide(state: AgentState):
    print(f"Receive Request from {state.name}: H={state.hunger:.1f}, B={state.boredom:.1f}")

    think_time = random.uniform(2.0, 5.0) 
    print(f"  Thinking... ({think_time:.2f}s)")
    await asyncio.sleep(think_time) # 使用 await 释放控制权，不卡死其他请求
    
    # 简单的逻辑模拟 (未来这里替换为 LLM API 调用)
    possible_actions = ["Wander"]
    thought_process = "I am feeling okay, just looking around."

    # 优先级逻辑模拟
    if state.hunger > 80 and state.hasFood:
        possible_actions = ["Eat"]
        thought_process = "I am starving! I need food now."
    elif state.fatigue > 80 and state.hasBed:
        possible_actions = ["Sleep"]
        thought_process = "I can barely keep my eyes open. Going to bed."
    elif state.boredom > 60 and state.hasComputer:
        possible_actions = ["UsePC"]
        thought_process = "So bored... I wonder what's on the internet."
    elif random.random() < 0.1: # 偶尔想聊天
        possible_actions = ["Talk"]
        thought_process = "I feel like socializing."

    chosen_action = random.choice(possible_actions)
    print(f"  > Decision for {state.name}: {chosen_action}")
    
    return {
        "action": chosen_action,
        "thought": thought_process
    }

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000)