# QuickStats
Unreal Engine plugin for displaying custom stats. 
* Create custom stat groups.
* Combine stats to create custom stats.
* Simple UI to display stats.

**NOTE:** This is not a replacement for built-in stats display :)

![Plugin Preview](Images/plugin_image.png)

# Installation
Clone `git clone git@github.com:amuTBKT/QuickStats.git` or unzip the plugin under Project/Plugins directory.
The plugin requires Unreal Engine version 4.27.2 or above.

# Settings
Available presets and drawing layout can be configured from **ProjectSettings->Plugins->QuickStats** page.

![Plugin Settings](Images/plugin_settings.png)

# Usage
* `stat quickstats` to toggle stat rendering. This internally runs `stat` command to toggle stat groups.
* `qstats.EnablePresets PresetA PresetB` to add presets to active list.
* `qstats.SetPresets PresetA PresetB` to set active presets.
* `qstats.DisablePresets PresetA` to remove presets from active list.
* `-qstatpresets=PresetA,PresetB` commandline argument to enable presets by default.

PresetA and PresetB are names for the presets defined in plugin settings.

# Stat Expressions
The flexibility of the plugin comes from combining stats using custom expressions.<br>
Built-in expressions include:
* `UQuickStatExpressionConstant` to define constant value.
* `UQuickStatExpressionReadStat` to read stat defined in code.
* Add, Subtract, Multiply and Divide operations.

Custom expressions can be defined by inheriting from `UQuickStatExpression`

![Stat Expression](Images/stat_expression.png)
The example above shows a custom stat "%CulledPrimitives" defined as <br>
`%CulledPrimitives = (CulledPrimitives + OccludedPrimitives) / ProcessedPrimitives`

# Known Issues / Limitations
* The plugin relies on toggling stat groups internally, so the system can break if user enables stat groups using `stat STATGROUP_NAME` commands.<br>
  It is recommended not to use `stat` command once the stat presets are active. 
