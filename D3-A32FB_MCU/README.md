# D3-A32FB MCU

## 项目导航

- 项目规则：`AGENTS.md`
- AI Workflow：`docs/ai/AHTT_AI_Development_Prompts.md`
- Web / Desktop 使用说明：`docs/ai/README.md`
- AHTT 项目 Skill：`.agents/skills/ahtt-development/`
- Bore 联调 Skill：`.agents/skills/ahtt-bore-sim-validation/`
- AHTT 权威文档：`docs/ahtt/`
- 验证工具：`tools/ahtt/`

## AI 工作流

```text
Web Planner → Desktop Executor → Independent Reviewer → Human Git Approval
```

公司仓库是开发、编译、板端验证和正式提交的来源；GitHub 镜像只在公司仓库确认版本后同步，供 Web Planner 读取。不得在两边长期独立维护同一规则文件。
