# AHTT AI Workflow

## 文件职责

- `AHTT_AI_Development_Prompts.md`：Planner、Executor、Reviewer 的角色、Gate 与输出契约。
- `plans/`：新的、已批准的 AHTT Implementation Plan。
- `docs/ahtt/`：当前架构、协议事实、验证记录与历史实施计划。

## 使用流程

```text
Web Planner
→ 用户批准 Plan
→ Web → Desktop Handoff
→ Desktop Executor
→ Independent Reviewer
→ 用户批准 Git 操作
→ 公司仓库确认版本
→ GitHub mirror sync
```

Web Planner 必须只读当前镜像仓库并输出 Plan。Desktop Executor 只执行 `docs/ai/plans/` 中带有 `Approval Handoff` 且由用户明确批准的 Plan。Reviewer 默认只读，不补写代码或补做板端操作。

## Plan 生命周期

```text
DRAFT → APPROVED → EXECUTING → EXECUTED → REVIEWED → CLOSED
```

仓库、分支、HEAD、目标文件或设计假设发生影响实施的变化时，Executor 必须停止并标记 `PLAN STALE` 或 `SCOPE EXPANSION`，返回 Planner 或用户重新批准。

## Git Gate

即使验证和 Reviewer 都通过，也不得自动 `git add`、`git commit` 或 `git push`。先展示变更、验证、风险和 Git 命令预览，取得用户明确确认后再执行。
