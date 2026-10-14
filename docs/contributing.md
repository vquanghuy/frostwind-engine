# Contributing

Follow the [coding conventions](coding-conventions.md), [project structure](project-structure.md), and relevant [RFCs](rfcs/README.md). Build and validation instructions are in [development](development.md).

## Commit messages

Use Conventional Commits for new commits:

```text
type: description
type(scope): description
```

Use a lowercase type and a short imperative description, such as `add texture file loading`. Omit a trailing period. Scopes are optional; use a lowercase module or area when it helps identify the change, such as `rendering`, `platform`, `engine`, `examples`, or `deps`.

| Type | Purpose |
| --- | --- |
| `feat` | Add functionality |
| `fix` | Correct a bug |
| `docs` | Change documentation |
| `refactor` | Restructure code without changing behavior |
| `perf` | Improve performance |
| `build` | Change build configuration or dependencies |
| `test` | Add or update tests |
| `ci` | Change continuous integration configuration |
| `style` | Change formatting without changing behavior |
| `chore` | Other repository maintenance |
| `revert` | Revert an earlier change |

```text
feat(rendering): add texture file loading
fix(platform): handle minimized windows
docs(examples): add textured quad screenshot
refactor(engine): simplify application lifecycle
build(deps): update the pinned SDL SDK
chore: update repository metadata
```

Choose the type that best describes the commit's main purpose. Add a body when needed to explain why a change was made or record meaningful validation. Mark breaking changes with `!` before the colon and explain the incompatibility in a `BREAKING CHANGE:` footer. Commit types do not automatically trigger releases; release decisions remain explicit.

Apply this format going forward; preserve existing commit history.
