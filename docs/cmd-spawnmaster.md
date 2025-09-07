---
tags:
  - command
---

# /spawnmaster

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/spawnmaster [on | off | list | uplist | downlist | load] | [ add|delete <spawn> ]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Adds or removes spawns to watch, tells when the last sighting was, and configures the plugin.
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `on|off` | Toggles SpawnMaster plugin on or off |
| `[add|delete] <"spawn name">` | Add or delete spawn name on the watch list (or target if no name given) |
| `list` | Display watch list for current zone |
| `uplist` | Display any mobs on watch list that are currently up |
| `downlist` | Display any watched mobs that have died or despawned |
| `load` | Load spawns from INI |
| `case [on|off]` | Turns on and off exact CASE matching |
| `vol <0.0 to 1.0>` | Controls master volume for the plugin. e.g. `/spawnmaster vol 1.0` |
