---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2spawnmaster.147/"
support_link: "https://www.redguides.com/community/threads/mq2spawnmaster.24809/"
repository: "https://github.com/RedGuides/MQ2SpawnMaster"
config: "MQ2SpawnMaster.ini"
authors: "Cr4zyb4rd, eqmule, ChatWithThisName, Knightly"
tagline: "Get an alert when a rare spawn pops"
acknowledgements: "Digitalxero's SpawnAlert plugin"
---

# MQ2SpawnMaster

<!--desc-start-->
Watches for new spawns in zone and can react in various ways.
<!--desc-end-->

## Commands

<a href="cmd-spawnmaster/">
{% 
  include-markdown "projects/mq2spawnmaster/cmd-spawnmaster.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2spawnmaster/cmd-spawnmaster.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2spawnmaster/cmd-spawnmaster.md') }}

## Settings

Example MQ2SpawnMaster.ini

```ini
[vox.Voxvox]
OnSpawnCommand="/multiline ; /beep ; /vt gandalf 005 ; /popup SPAWN: ${SpawnMaster.LastMatch}!"
Enabled=on
[The Steamfont Mountains]
Spawn0=yendar starpyre
Spawn1=renux herkanor
```

For just a simple beep,

```ini
[Server.CharacterName]
OnSpawnCommand="/beep"
Enabled=on
```

More examples, including the master ini, can be found in the [support thread](https://www.redguides.com/community/threads/mq2spawnmaster.24809/).

## Top-Level Objects

## [SpawnMaster](tlo-spawnmaster.md)
{% include-markdown "projects/mq2spawnmaster/tlo-spawnmaster.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2spawnmaster/tlo-spawnmaster.md') }}

## DataTypes

## [SpawnMaster](datatype-spawnmaster.md)
{% include-markdown "projects/mq2spawnmaster/datatype-spawnmaster.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2spawnmaster/datatype-spawnmaster.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2spawnmaster/datatype-spawnmaster.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2spawnmaster/datatype-spawnmaster.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
