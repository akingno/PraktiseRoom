from fastapi import FastAPI
from pydantic import BaseModel
from typing import List, Optional, Dict
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

class BatchResponse(BaseModel):
    decisions: Dict[str, DecisionResponse]




@app.post("/decide_batch", response_model=BatchResponse)
async def decide_batch(agents: List[AgentState]):
    
    # --- 1. 收到所有人的状态 ---
    names = [a.name for a in agents]
    print(f"\n[Batch Request] Received states for: {names}")

    # --- 2. 模拟 LLM 的“全局思考”延迟 ---
    # 这一次 sleep 代表 LLM 阅读所有人的状态并生成 tokens 的时间
    think_time = random.uniform(1.0, 3.0)
    print(f"  Thinking globally... ({think_time:.2f}s)")
    await asyncio.sleep(think_time)

    # --- 3. 做出决策 (模拟 LLM 逻辑) ---
    decisions = {}

    # 在未来接入 LLM 时，你会在这里构建一个巨大的 Prompt，包含所有 agent 的信息
    # prompt = f"Scene contains: {names}. Status: {agents}..."
    # response = llm.generate(prompt)
    # 然后解析 response 分配给每个人
    
    for agent in agents:
        possible_actions = ["Wander"]
        thought_process = "Just looking around."
        
        # 简单的基于规则的模拟 (未来替换为 LLM 的输出解析)
        if agent.hunger > 80 and agent.hasFood:
            possible_actions = ["Eat"]
            thought_process = "I'm starving, let's grab some food."
        elif agent.fatigue > 80 and agent.hasBed:
            possible_actions = ["Sleep"]
            thought_process = "Too tired... going to sleep."
        elif agent.boredom > 60 and agent.hasComputer:
            possible_actions = ["UsePC"]
            thought_process = "Bored. Checking the internet."
        
        # 模拟社交互动的随机性
        # (只有当 LLM 真的理解场景时，这里才能做出真正的协作决策，比如 A 找 B)
        elif random.random() < 0.05: 
            possible_actions = ["Talk"]
            thought_process = f"I want to talk to someone."

        chosen_action = random.choice(possible_actions)
        
        print(f"  > Decision for {agent.name}: {chosen_action}")

        # 构建单个决策结果
        decisions[agent.name] = DecisionResponse(
            action=chosen_action,
            thought=thought_process
        )

    # --- 4. 打包返回 ---
    return BatchResponse(decisions=decisions)
if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000)