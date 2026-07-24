# Question 5: vi Crash Recovery — Evaluation and Recommended Strategy

## Scenario
A developer is editing a critical configuration file in `vi`. The system
crashes before the file is saved. What recovery mechanisms exist, and
which is the most reliable?

---

## 1. Swap Files (`.filename.swp`)

**What it is:** While editing, `vi`/`vim` continuously writes your
in-progress changes to a hidden swap file (e.g. `.config.txt.swp`) in the
same directory (or a configured swap directory), independent of whether
you've saved.

**How recovery works:**
```bash
vi -r config.txt
```
This reads the swap file and reconstructs the unsaved buffer as it was
at the moment of the crash.

**Strengths:**
- Automatic — requires no manual action from the user beforehand.
- Captures changes made *since the last save*, which is exactly the data
  that would otherwise be lost.
- Works even if the crash happens mid-edit, with zero warning.

**Limitations:**
- If the crash also corrupts or prevents flushing the swap file to disk
  (e.g. sudden power loss mid-write), recovery may be partial or fail.
- Swap files can persist and clutter directories if not cleaned up after
  a manual recovery.
- Two people editing the same file can trigger swap-file conflict
  warnings.

---

## 2. Undo History (`.filename.un~` / persistent undo)

**What it is:** If `set undofile` is enabled in `.vimrc`, vim saves a
persistent undo history to disk that survives even after the editor is
closed and reopened — not just during a single session.

**How recovery works:** Reopen the file; press `u` repeatedly, or `vim
-c "undolist"` to see history, even from a previous vim session.

**Strengths:**
- Lets you step backward through many previous states, not just the last
  save point.

**Limitations:**
- **Not enabled by default** in plain `vi`, and even in `vim` it requires
  `undofile` to be explicitly set beforehand — it won't help if it wasn't
  configured *before* the crash.
- Only useful if the crash happens *after* some edits were already
  undo-tracked and *written* to the undo file, which itself requires
  periodic disk writes similar to swap files.

---

## 3. Registers

**What it is:** Named/numbered memory buffers inside a running vi/vim
session that store yanked, deleted, or copied text (e.g. `"a`, `"1`).

**Strengths:**
- Useful for recovering specific deleted lines *within* an active
  session (e.g. undoing an accidental `dd`).

**Limitations:**
- **Registers exist only in memory for the duration of the process.**
  When the process crashes, registers are lost along with everything
  else in RAM. They provide **no crash-recovery value** — they only help
  with in-session accidental deletions, not system crashes.

---

## 4. Backup Files (`filename~`)

**What it is:** If `set backup` is enabled, vim writes a copy of the file
**as it was before the current save** to `filename~` each time you save.

**Strengths:**
- Simple, human-readable backup of the previous saved version.
- Useful for reverting to the state before your *last* save if new edits
  turn out to be wrong.

**Limitations:**
- **Only created at save time.** If the crash happens before any save in
  this session, no backup file reflecting the crash-time state exists —
  it can only get you back to the previous save, not the lost unsaved
  work.
- Not real-time; doesn't protect against losing in-progress edits.

---

## 5. Auto-recovery Prompt

**What it is:** When you next open a file that has a leftover swap file,
vim automatically detects it and prompts:
