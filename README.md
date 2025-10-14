# 行为循环图 (Per-Tick Loop)

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                                Per-Tick Loop                                │
│                          (main.cpp while(running))                          │
└─────────────────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 1) Needs Update                                                             │
│    character.tickNeeds(dt)                                                  │
│    room.ensureFoodSpawned()                                                 │
└─────────────────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 2) Perception & Utility                                                     │
│   - pos = character.getLoc()                                                │
│   - fpos = room.foodPos()                                                   │
│   - hasFood = room.hasFood()                                                │
│   - scoreEat = CalcScoreEat(hunger, pos, hasFood, fpos,                     │
│                             onCooldown=!eatAvailable,                       │
│                             sticky=(act==Eat))                              │
│   - scoreWander = BASE_WANDER                                               │
└─────────────────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 3) Action Selection                                                         │
│   chosen = (scoreEat > scoreWander) ? Eat : Wander                          │
│   character.setAct(chosen)                                                  │
└─────────────────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 4) ActionExecutor::tick(chosen, ctx, bb)                                    │
│   ctx = { room, character, tick_index }                                     │
│   bb  = Blackboard(跨帧共享：target / path / path_i / path_invalid /节流)     │
└─────────────────────────────────────────────────────────────────────────────┘
        │
   ┌────┴─────────────────────────────────────────────────────────────────┐
   │                                                                      │
   ▼                                                                      ▼
┌───────────────────────────────────────────────┐         ┌───────────────────────────────────────────────┐
│ 4A) Wander                                   │         │ 4B) Eat                                      │
│     ActionExecutor::tick_wander(ctx)         │         │     ActionExecutor::tick_eat(ctx, bb)        │
│   • is_passable: 非墙即可                     │         │  ┌─ FindFood ──────────────────────────────┐│
│   • character.tryMove(is_passable,           │         │  │ if !bb.target_valid:                     ││
│     keep_prob=0.9) 随机带偏好                │         │  │   if !room.hasFood() → return            ││
│                                               │        │  │   bb.target = foodPos;                   ││
│   结果：                                      │        │  │   bb.target_valid = true;                ││
│   • 迈一步或原地（堵住）                      │        │  │   bb.path_invalid = true;                 ││
└───────────────────────────────────────────────┘         │  └──────────────────────────────────────────┘│
                                                          │                                              │
                                                          │  ┌─ Plan (A*) ─────────────────────────────┐│
                                                          │  │ if need_replan(ctx, bb, cur, target):   ││
                                                          │  │   bb.path.clear();                       ││
                                                          │  │   bb.last_planned_for_tick = tick_index; ││
                                                          │  │   if pf.plan(cur → target, bb.path):    ││
                                                          │  │      bb.path_i = (size>=2?1:size);       ││
                                                          │  │      bb.path_invalid = false;            ││
                                                          │  │   else return (暂不可达)                 ││
                                                          │  └──────────────────────────────────────────┘│
                                                          │                                              │
                                                          │  ┌─ FollowPath ────────────────────────────┐│
                                                          │  │ if !bb.path_invalid:                     ││
                                                          │  │   next = bb.path[bb.path_i];             ││
                                                          │  │   if character.tryStepTo(next,passable): ││
                                                          │  │       ++bb.path_i;                       ││
                                                          │  │   else                                   ││
                                                          │  │       bb.path_invalid = true; return     ││
                                                          │  └──────────────────────────────────────────┘│
                                                          │                                              │
                                                          │  ┌─ EatNow ────────────────────────────────┐│
                                                          │  │ if at foodPos && eatAvailable():         ││
                                                          │  │   character.eat(FOOD_CALORIES);          ││
                                                          │  │   room.consumeFood();                     ││
                                                          │  │   // 清理 BlackBoard                     ││
                                                          │  │   bb.target_valid = false;               ││
                                                          │  │   bb.path.clear(); bb.path_i=0;          ││
                                                          │  │   bb.path_invalid = true;                ││
                                                          │  └──────────────────────────────────────────┘│
                                                          └───────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 5) Render (IRender → ASCIIRender)                                           │
│   - render_room(room)                                                       │
│   - render_items(room)  // 门、食物覆盖                                      │
│   - render_character(@)                                                     │
│   - render_info(... scoreEat, scoreWander, Hunger, pos, dir, 诊断信息)       │
└─────────────────────────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ 6) Sleep until next tick (≈33ms), ++tick_index → 回到步骤 1                 │
└─────────────────────────────────────────────────────────────────────────────┘
