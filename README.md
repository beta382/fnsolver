# FnSolver

FnSolver is a tool for the video games *Xenoblade Chronicles X* and *Xenoblade Chronicles X: Definitive Edition*, which generates tailored solutions to FrontierNav layouts.

This software is inspired by [XenoProbes](https://github.com/minneyar/xenoprobes/tree/main) by danielko and minneyar.

This software uses the following third-party libraries; see [3rd-party-licenses](3rd-party-licenses) for their licenses:

- [CLI11](https://github.com/CLIUtils/CLI11)

## Table of Contents

- [Differences Between FnSolver and XenoProbes](#differences-between-fnsolver-and-xenoprobes)
- [Running](#running)
- [Options](#options)
  - [General Options](#general-options)
    - [`--help`](#--help)
    - [`--config-file`](#--config-file)
    - [`--export-config-file`](#--export-config-file)
    - [`--auto-confirm`](#--auto-confirm)
  - [Score Function](#score-function)
    - [`--score-function`](#--score-function)
    - [`--tiebreaker`](#--tiebreaker)
  - [Inventory](#inventory)
    - [`--inventory`](#--inventory)
  - [FrontierNav Overrides](#frontiernav-overrides)
    - [`--territories`](#--territories)
    - [`--locked-sites`](#--locked-sites)
    - [`--seed`](#--seed)
    - [`--force-seed`](#--force-seed)
  - [Constraints](#constraints)
    - [`--precious-resources`](#--precious-resources)
    - [`--min-mining`](#--min-mining)
    - [`--min-revenue`](#--min-revenue)
    - [`--min-storage`](#--min-storage)
  - [Solver Algorithm Parameters](#solver-algorithm-parameters)
    - [`--iterations`](#--iterations)
    - [`--bonus-iterations`](#--bonus-iterations)
    - [`--population`](#--population)
    - [`--offspring`](#--offspring)
    - [`--mutation-rate`](#--mutation-rate)
    - [`--max-age`](#--max-age)
    - [`--threads`](#--threads)
  - [Complete Examples](#complete-examples)
  - [Building](#building)
  - [FAQ](#faq)


## Differences Between FnSolver and XenoProbes

- \+ Greatly expanded configuration for layout optimization, including:
  - More score functions (and score functions that are generally "more interesting" for optimization)
  - Precious Resource constraints (for example: "generate solutions that get at least 70% of the average Bonjelium drops")
  - Solution seeding (to give the solver a nudge in the right direction when you know a powerful core setup, or want to find minor tweaks to a complete preexisting layout)
- \+ Easier configuration (subjective), including:
  - Probe inventory presets
  - Import layout seeds and locked sites from [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) URLs
  - Export configuration to a file to reuse later
- \+ Fixed bugs:
  - Accurately accounts for Duplicator and Booster chains (the former of which are core to many optimal layouts)
  - Accurately accounts for how Duplicators on sightseeing spots contribute to revenue when duplicating Research probes
  - Considers additional probes in the inventory when generating mutations (instead of a solution lineage being stuck with only the initially selected subset of the inventory)
  - More uniformly generates mutations (XenoProbes has a bias towards mutating later FrontierNav sites)
- \+ ~60-80% faster (tested across various option combinations)
- \- No GUI



## Running

FnSolver is a Command Line Interface (CLI) application.

If you're a Windows user who downloaded the [pre-built executable](https://github.com/beta382/fnsolver/releases) (and you aren't familiar with running CLI applications):

1. Use Windows Explorer to navigate to the folder you extracted the executable to.
2. From there, you can launch a Command Prompt or PowerShell session in the current folder by holding `Shift` and then right-clicking in an open area to select the "Open Cmd/PowerShell window here" option, or by typing `cmd` or `powershell` into the Windows Explorer navigation bar and hitting `Enter` (see [this short YouTube video](https://www.youtube.com/watch?v=bgSSJQolR0E) for a demonstration).
3. Once you have a Command Prompt or PowerShell session in the folder where you placed the executable, you can run FnSolver by typing `fnsolver-cli.exe` into the command line, followed by any of the desired options as described below, and then hitting `Enter`.

If you built FnSolver from source using the provided CMake presets, you should already be familiar with using the command line. The executable is located under the build folder at `build-*/fnsolver/cli/fnsolver-cli`. If you built FnSolver from source using your own build instructions, then you should already be familiar with the build tools you used and where they output the executable.

After you've started FnSolver, you will be presented with an overview of your configuration, and a prompt on whether you would like to continue on with the configuration (enter `y`) or cancel (enter `n`). Once you confirm the prompt, FnSolver will begin optimizing. You may end the optimization early at any time by pressing Ctrl-C (or your shell's alternative SIGINT key combo).

Once FnSolver concludes, it will output a report for the best FrontierNav layout it found, including:

- A [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) URL linking to a visual representation of the layout
- An overview of the resource yields for the layout
- A table giving advanced details for each FrontierNav site in the layout



## Options



### General Options

#### `--help`

Shorthand: `-h`

Displays a listing and brief description of all options, then exits.

#### `--config-file`

- Takes one argument: the name of the configuration file

Uses options from the specified configuration file.

If an option is in the configuration file and is also passed on the command line, the option passed on the command line will take priority. Note that passing an option on the command line will *entirely* replace the option from the configuration file; options with multiple arguments will be cleared and replaced, not appended to.

On/Off options (such as [`--auto-confirm`](#--auto-confirm)) that are captured in a configuration file can be disabled by passing `--<long-option>=false` on the command line. Optional list options (such as [`--territories`](#--territories)) that are captured in a configuration file have no way of being simply cleared by overriding on the command line, you must manually edit the configuration file to remove the relevant line (or comment it out by putting a `#` before the option in the configuration file).

Examples:

- `--config-file config.toml`

#### `--export-config-file`

- Takes one argument (default `./config.toml`): the name of the configuration file

Exports options (excluding this option and [`--config-file`](#--config-file)) to the specified configuration file.

If this option is not explicitly passed, then no action is taken. If the specified configuration file already exists, it will be overwritten.

Examples:

- `--export-config-file`
- `--export-config-file myconfig.toml`

#### `--auto-confirm`

Shorthand: `-y`

Automatically accepts the confirmation prompt.



### Score Function

The Score Function is used to calculate the "score" for a generated FrontierNav layout, which is then used to compare different layouts and determine which one is "better". FnSolver supports the following score functions:

- `max_mining`
  - Uses "Mining yield" as the score.
  - It is recommended to use `max_effective_mining`, as that tends to be more interesting in practice.
- `max_effective_mining`
  - Takes one argument: `storage_factor`
  - Uses the lesser of "total Storage" or "Mining yield times `storage_factor`" as the score.
  - Represents "How much Minanium can I get in `storage_factor` FrontierNav cycles, capped by my Storage".
  - Effectively shorthand for `ratio 1 0 storage_factor` (but more efficient).
- `max_revenue`
  - Uses "Revenue yield" as the score.
- `max_storage`
  - Uses "total Storage" as the score.
- `ratio`
  - Takes three argument: `mining_factor`, `revenue_factor`, and `storage_factor`
  - Uses the lesser of "yield divided by its `factor` times the largest `factor`" for each yield type with a non-zero `factor` as the score.
  - Effectively maximizes your yields while maintaining the specified ratio between them.
- `weights`
  - Takes three argument: `mining_weight`, `revenue_weight`, and `storage_weight`
  - Uses the sum of "yield times its `weight`" for each yield type as the score.
  - **Generally not recommended**, since it effectively just maximizes the yield type with the highest `weight`. It is recommended to the other Score Functions instead. Provided solely for parity with XenoProbes.

#### `--score-function`

Shorthand: `-f`

- Required.
- Takes one or more argument: the name of a Score Function followed by any arguments for that Score Function as described above

Sets the Score Function.

Examples:

- `-f max_revenue`
- `-f max_effective_mining 2`
- `-f ratio 1 1.5 4`

#### `--tiebreaker`

- Takes one argument: the name of the tiebreaker Score Function
- Must be one of `max_mining`, `max_revenue`, or `max_storage`, and must not be the same as the Score Function

Sets the tiebreaker Score Function.

Generally, this is only interesting with `max_storage`, `max_effective_mining`, or `ratio` as the Score Function. With `max_mining` or `max_revenue` as the Score Function, the non-zero efficacy on Research and Mining probes respectively make a tiebreaker largely low-impact. With `weights` as the Score Function, a tiebreaker is effectively built-in.

Examples:

- `-f max_storage --tiebreaker max_revenue`
- `-f max_effective_mining 5.5 --tiebreaker max_mining`



### Inventory

Your inventory is comprised of the FrontierNav probes you have available *and wish to use for optimization*. You may choose to use fewer probes than you have available, e.g., removing all Research probes when optimizing for Mining.

#### `--inventory`

Shorthand: `-i`

- Takes one or more argument (default `[all_de]`): the contents of the inventory
- Each argument must be one of:
  - A group name:
    - `all_og`: Sets the inventory to the maximum number of probes available in the original *Xenoblade Chronicles X* (without Combat probes)
    - `all_de`: Sets the inventory to the maximum number of probes available in *Xenoblade Chronicles X: Definitive Edition* (without Combat probes)
    - `no_mining`: Sets the quantity of all Mining probes to zero
    - `no_research`: Sets the quantity of all Research probes to zero
    - `no_booster`: Sets the quantity of all Booster probes to zero
    - `no_storage`: Sets the quantity of Storage probes to zero
    - `no_duplicator`: Sets the quantity of Duplicator probes to zero
  - An individual probe listing:
    - In the format `probe_shorthand:quantity`
    - Valid `probe_shorthand`s are:
      - `M1` through `M10` for Mining probes
      - `R1` through `R6` for Research probes
      - `B1` and `B2` for Booster probes
      - `S` for Storage probes
      - `D` for Duplicator probes
      - `C` for Combat probes
- All group names must come before the first individual probe listing

Sets the probe inventory.

Arguments are evaluated in the order they are provided. Note that providing any argument will remove the default of `all_de`, rather than appending to it. If you provide a [`--seed`](#--seed), the probes necessary for it will be deducted from the provided inventory. If the inventory has insufficient probes to fill a FrontierNav layout (after accounting for [`--seed`](#--seed) and [`--locked-sites`](#--locked-sites)), it will be padded with Basic probes.

Examples:

- `-i all_de no_research M1:0 M2:0 M3:0 M4:0 M5:4` (a basic recommended inventory for Mining optimization, with 2 spare probes)
- `-i all_og no_mining R4:2 B1:2 B2:2 D:3 S:7` (will be padded with 68 Basic probes)



### FrontierNav Overrides

#### `--territories`

- Takes one or more arguments (default empty): a list of overrides for FrontierNav site "Unexplored Territories Found Nearby" quantities
- Each argument must be in the format `site_id:quantity`
- A `quantity` must not exceed the maximum number of "Unexplored Territories Found Nearby" for that `site_id`

Sets FrontierNav site "Unexplored Territories Found Nearby" overrides.

By default, it is assumed you have found all Unexplored Territories.

Examples:

- `--territories 315:1 117:0 414:0 505:1`

#### `--locked-sites`

- Takes one or more arguments (default empty): a list of the FrontierNav sites that are locked/undiscovered
- You may provide as arguments either:
  - A [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) layout URL, where any sites with "No Probe" are considered locked/undiscovered, and all others are ignored
  - A list of FrontierNav site IDs
- Must not overlap with FrontierNav sites specified in [`--seed`](#--seed)

Sets FrontierNav sites that are are locked/undiscovered.

By default, it is assumed you have unlocked all FrontierNav sites.

If you use [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) to generate a layout URL, it is generally easiest to select the "[All Basic Probes](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/All%20Basic%20Probes?map=101-1~102-1~103-1~104-1~105-1~106-1~107-1~108-1~109-1~110-1~111-1~112-1~113-1~114-1~115-1~116-1~117-1~118-1~119-1~120-1~121-1~201-1~202-1~203-1~204-1~205-1~206-1~207-1~208-1~209-1~210-1~211-1~212-1~213-1~214-1~215-1~216-1~217-1~218-1~219-1~220-1~221-1~222-1~223-1~224-1~225-1~301-1~302-1~303-1~304-1~305-1~306-1~307-1~308-1~309-1~310-1~311-1~312-1~313-1~314-1~315-1~316-1~317-1~318-1~319-1~320-1~321-1~322-1~401-1~402-1~403-1~404-1~405-1~406-1~407-1~408-1~409-1~410-1~411-1~412-1~413-1~414-1~415-1~416-1~417-1~418-1~419-1~420-1~501-1~502-1~503-1~504-1~505-1~506-1~507-1~508-1~509-1~510-1~511-1~512-1~513-1~514-1~515-1~516-1)" default layout, and then swap the sites you have locked to "No Probe". You must click "My Current Probe Layout" for the URL in your browser's address bar to become the layout URL.

Examples:

- `--locked-sites 110 320 507`
- `--locked-sites https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/My%20Current%20Layout?map=101-1~102-1~103-1~104-1~105-1~106-1~107-1~108-1~109-1~110-0~111-1~112-1~113-1~114-1~115-1~116-1~117-1~118-1~119-1~120-1~121-1~201-1~202-1~203-1~204-1~205-1~206-1~207-1~208-1~209-1~210-1~211-1~212-1~213-1~214-1~215-1~216-1~217-1~218-1~219-1~220-1~221-1~222-1~223-1~224-1~225-1~301-1~302-1~303-1~304-1~305-1~306-1~307-1~308-1~309-1~310-1~311-1~312-1~313-1~314-1~315-1~316-1~317-1~318-1~319-1~320-0~321-1~322-1~401-1~402-1~403-1~404-1~405-1~406-1~407-1~408-1~409-1~410-1~411-1~412-1~413-1~414-1~415-1~416-1~417-1~418-1~419-1~420-1~501-1~502-1~503-1~504-1~505-1~506-1~507-0~508-1~509-1~510-1~511-1~512-1~513-1~514-1~515-1~516-1` (the same as above)

#### `--seed`

- Takes one or more arguments (default empty): a list of probe placements
- You may provide as arguments either:
  - A [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) layout URL, where any sites with a probe other than "No Probe" are considered part of the layout seed
  - A list of FrontierNav site ID and probe type pairs in the format `site_id:probe_shorthand`
    - Valid `probe_shorthand`s are:
      - `M1` through `M10` for Mining probes
      - `R1` through `R6` for Research probes
      - `B1` and `B2` for Booster probes
      - `S` for Storage probes
      - `D` for Duplicator probes
      - `C` for Combat probes
      - `-` for Basic probes
- Must not overlap with FrontierNav sites specified in [`--locked-sites`](#--locked-sites)

Sets the layout seed.

The layout seed is used to alter the generation of random FronterNav layouts as part of the [FnSolver algorithm](#solver-algorithm-parameters). When a random FronterNav layout is generated, it is guaranteed to contain the layout seed. Note that the layout seed may be deviated from when generating FrontierNav layout mutations (see [`--force-seed`](#--force-seed)).

This can be used for various purposes, including:

- Suggesting to the FnSolver algorithm a core set of probe placements that you know to be potentially-optimal (usually in conjunction with [`--force-seed`](#--force-seed)), to arrive at better FrontierNav layouts quicker, or help FnSolver find a FrontierNav layout that it otherwise would struggle to find
- Giving FnSolver a complete layout from a prior run (or that you handcrafted) for it to find further improvements or tweaks for (note that this effectively pigeonholes the FnSolver algorithm into not considering substantially different FrontierNav layouts, but can allocate many more resources towards fine-tuning that specific FrontierNav layout)

If you use [frontiernav.net](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides) to generate a layout URL, it is generally easiest to select the "[No Probes](https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/No%20Probes?map=101-0~102-0~103-0~104-0~105-0~106-0~107-0~108-0~109-0~110-0~111-0~112-0~113-0~114-0~115-0~116-0~117-0~118-0~119-0~120-0~121-0~201-0~202-0~203-0~204-0~205-0~206-0~207-0~208-0~209-0~210-0~211-0~212-0~213-0~214-0~215-0~216-0~217-0~218-0~219-0~220-0~221-0~222-0~223-0~224-0~225-0~301-0~302-0~303-0~304-0~305-0~306-0~307-0~308-0~309-0~310-0~311-0~312-0~313-0~314-0~315-0~316-0~317-0~318-0~319-0~320-0~321-0~322-0~401-0~402-0~403-0~404-0~405-0~406-0~407-0~408-0~409-0~410-0~411-0~412-0~413-0~414-0~415-0~416-0~417-0~418-0~419-0~420-0~501-0~502-0~503-0~504-0~505-0~506-0~507-0~508-0~509-0~510-0~511-0~512-0~513-0~514-0~515-0~516-0)" default layout, and then swap the sites you wish to be in the layout seed to the desired probe. You must click "My Current Probe Layout" for the URL in your browser's address bar to become the layout URL.

Examples:

- `--seed 312:D 315:D 321:D` (a Duplicator chain setup in Oblivia, useful for optimizing any yield type while keeping high Precious Resource output; 312 may be swapped with 318, or omitted entirely to let FnSolver figure out which is better)
- `--seed 405:D 408:D 409:D` (a powerful Duplicator chain setup in Sylvalum, useful for optimizing Mining or Storage while keeping high Precious Resource output)
- `--seed 110:D 111:D 112:D 113:D 114:D 405:D 408:D 409:D` (a powerful Duplicator chain setup in Primordia and Sylvalium, useful for optimizing primarily Storage while keeping high Precious Resource output)
- `--seed 508:D 509:D 511:D` (a powerful Duplicator chain setup in Cauldros, highly optimal for any yield type if you don't care about losing some Bonjelium output)
- `--seed https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/My%20Current%20Layout?map=101-0~102-0~103-0~104-0~105-0~106-0~107-0~108-0~109-0~110-0~111-0~112-0~113-0~114-0~115-0~116-0~117-0~118-0~119-0~120-0~121-0~201-0~202-0~203-0~204-0~205-0~206-0~207-0~208-0~209-0~210-0~211-0~212-0~213-0~214-0~215-0~216-0~217-0~218-0~219-0~220-0~221-0~222-0~223-0~224-0~225-0~301-0~302-0~303-0~304-0~305-0~306-0~307-0~308-0~309-0~310-0~311-0~312-0~313-0~314-0~315-0~316-0~317-0~318-0~319-0~320-0~321-0~322-0~401-0~402-0~403-0~404-0~405-0~406-0~407-0~408-0~409-0~410-0~411-0~412-0~413-0~414-0~415-0~416-0~417-0~418-0~419-0~420-0~501-0~502-0~503-0~504-0~505-0~506-0~507-0~508-20~509-20~510-0~511-20~512-0~513-0~514-0~515-0~516-0` (the same as above)

#### `--force-seed`

- Only valid with a non-empty layout seed

Forces the layout seed to be retained when FrontierNav layout mutations are generated by the [FnSolver algorithm](#solver-algorithm-parameters). This means that your entire layout seed is guaranteed to be present in all generated FrontierNav layouts.

Examples:

- `--seed 312:D 315:D 321:D --force-seed`

### Constraints

Constraints impose requirements upon generated FrontierNav layouts. These function by setting the score of a FrontierNav layout to zero if it violates the constraint.

#### `--precious-resources`

- Takes one or more arguments (default empty): a list of precious resource constraints
- Each argument must be in the format `precious_resource_name:constraint`
  - Valid `precious_resource_name`s are:
    - `arc_sand_ore` (max: 5.12)
    - `aurorite` (max: 5.12)
    - `white_cometite` (max: 4.53)
    - `enduron_lead` (max: 4.15)
    - `everfreeze_ore` (max: 4.15)
    - `foucaultium` (max: 2.34)
    - `lionbone_bort` (max: 2.64)
    - `infernium` (max: 3.33)
    - `boiled_egg_ore` (max: 3.42)
    - `marine_rutile` (max: 3.29)
    - `dawnstone` (max: 2.53)
    - `cimmerian_cinnabar` (max: 1.90)
    - `ouroboros_crystal` (max: 0.84)
    - `parhelion_platinum` (max: 2.09)
    - `bonjelium` (max: 2.60)
  - Valid `constraint`s are:
    - `all`
    - `any`
    - `x%`, where `x` is a floating-point value in (0.0, 100.0] representing the required percentage of the maximum average yield
    - `x`, an integer value representing the required minimum absolute average yield times 100

Requires that a generated FrontierNav layout yield at least the specified quantity of Precious Resources (on average).

A given FrontierNav site distributes precious resources with per-site rates, and per-site roll counts. You may consult Xeno Series Wiki (e.g., [Bonjelium](https://www.xenoserieswiki.org/wiki/Bonjelium)) for rate information. Because each site may have greatly different expected drop counts for a given Precious Resource, it does not make much sense to constrain by "number of sites that are yielding Precious Resources". As such, the constraints provided here are based on "average expected drop count".

Examples:

- `--precious-resources bonjelium:all parhelion_platinum:all marine_rutile:all`
- `--precious-resources cimmerian_cinnabar:any ouroboros_crystal:any`
- `--precious-resources bonjelium:60%`
- `--precious-resources bonjelium:160` (1.60 out of 2.60)

#### `--min-mining`

- Takes one argument (default `0`): the Mining yield constraint

Requires that a generated FrontierNav layout yield at least the specified Mining.

**Use of this option is discouraged**. Even seemingly small values can make it unreasonably difficult for the FnSolver algorithm to discover valid FrontierNav layouts. Prefer using the `ratio` Score Function instead. Provided solely for the curious.

Examples:

- `--min-mining 40000`

#### `--min-revenue`

- Takes one argument (default `0`): the Revenue yield constraint

Requires that a generated FrontierNav layout yield at least the specified Revenue.

**Use of this option is discouraged**. Even seemingly small values can make it unreasonably difficult for the FnSolver algorithm to discover valid FrontierNav layouts. Prefer using the `ratio` Score Function instead. Provided solely for the curious.

Examples:

- `--min-revenue 120000`

#### `--min-storage`

- Takes one argument (default `0`): the Storage yield constraint

Requires that a generated FrontierNav layout yield at least the specified Storage.

**Use of this option is discouraged**. Even seemingly small values can make it unreasonably difficult for the FnSolver algorithm to discover valid FrontierNav layouts. Prefer using the `ratio` Score Function instead. Provided solely for the curious.

Examples:

- `--min-storage 100000`



### Solver Algorithm Parameters

The FnSolver algorithm can be simplified as follows:

- `<population>` number of random FronterNav layouts are generated
- For `<iterations>` loops:
  - Each FronterNav layout in the population creates `<offspring>` number of mutations from itself:
    - A mutation is created by randomly (according to `<mutation-rate>`) swapping or not swapping around probes in the FronterNav layout
    - The best offspring (if it is an improvement upon its parent) replaces its parent in the population
    - If no offspring are an improvement upon the parent (and the parent is not the best FronterNav layout in the population), the parent is instead aged
    - If the parent's age reaches `<max-age>`, it is replaced by a random FronterNav layout

The total runtime of FnSolver will scale roughly linearly with `iterations * population * offspring`.

The options below control the various named parameters from the description above.

#### `--iterations`

Shorthand: `-n`

- Takes one argument (default `1000`): the number of iterations

Sets the number of iterations FnSolver will run for.

Increasing this will give FnSolver more time to discover and improve upon FronterNav layouts. You may end FnSolver prematurely at any time by pressing Ctrl-C (or whatever your shell's SIGINT keybind is), which will stop FnSolver and present the best FronterNav layout after the current iteration concludes.

Examples:

- `-n 1000`

#### `--bonus-iterations`

- Takes one argument (default `0`): the number of bonus iterations

Sets the maximum number of additional iterations FnSolver will run for if an improvement to the best FrontierNav layout is found.

Setting this option to a non-zero value allows for the number of iterations set in [`--iterations`](#--iterations) to be exceeded, in the event that a better FrontierNav layout is found shortly before FnSolver would normally terminate. The bonus iterations are not cumulative, but do reset if better FrontierNav layouts continue to be found. Put another way, FnSolver will only terminate if at least `iterations` iterations have run, AND at least `bonus-iterations` iterations have passed since the last improvement occurred.

Examples:

- `-n 1 --bonus-iterations 100`

#### `--population`

Shorthand: `-p`

- Takes one argument (default `100`): the size of the population

Sets the size of the FronterNav layout population.

Increasing this will give FnSolver more opportunities to find potentially-optimal FronterNav layouts that are substantially different from one another. One weakness of the FnSolver algorithm is that offspring are generally very similar to their parents, and therefore a FronterNav layout lineage can reach a point where it can no longer be trivially improved upon, despite not being truly optimal (this is known as a Local Maximum). A larger population can help mitigate this.

Examples:

- `-p 200`

#### `--offspring`

Shorthand: `-o`

- Takes one argument (default `200`): the number of offspring

Sets the number of offspring to generate for each FronterNav layout in each iteration.

Increasing this will give FnSolver more opportunities to find mutations that improve upon a given FronterNav layout. One weakness of the FnSolver algorithm is that the "threshold" nature of chains make them difficult to discover when only swapping around a small number of probes. Additionally, very specific mutations can be difficult to randomly happen upon. A larger number of offspring can help mitigate this (but for very complex mutations, like swapping a chain with a different chain elsewhere, there is no good solution).

Examples:

- `-o 300`

#### `--mutation-rate`

Shorthand: `-m`

- Takes one argument (default `0.04`): the mutation rate

Sets the degree by which a FronterNav layout will mutate when generating offspring.

This rate represents the chance that a probe in a given FrontierNav site placement will be swapped with a probe in another FrontierNav site placement. Because swaps are two-sided (and further complicated by potentially swapping a probe with an identical probe, or re-swapping a previously swapped probe), the real chance that a given FrontierNav site placement will undergo a meaningful probe swap is approximately 1.67 times the mutation rate. This will decrease slightly if you have any locked/undiscovered sites or a forced layout seed.

Tuning this can be somewhat difficult: too low makes it hard to find improvements that require complex mutations, while too high makes it hard to retain the core features of a particular FronterNav layout that make it potentially-optimal (and also makes it hard to find very simple mutations). A rate between ~0.03 and ~0.08 is generally recommended.

Examples:

- `-m 0.05`

#### `--max-age`

Shorthand: `-a`

- Takes one argument (default `50`): the maximum age

Sets the maximum number of iterations a FrontierNav layout lineage can go without improvement before it is killed.

Increasing this will give FronterNav layout lineages that are not the best FrontierNav layout more time to find complex or precise improvements, which might make them become the best FrontierNav layout. However, setting this too high will prevent FrontierNav layouts truly stuck in a sub-optimal Local Maximum from being removed and restarted. Generally, you should increase or decrease this roughly proportional to your `iterations`.

A FrontierNav layout lineage that has a score of zero (namely, it fails to meet [constraints](#constraints)) will be aged at 5x speed.


#### `--threads`

Shorthand: `-t`

- Takes one argument (default varies): the number of threads

Sets the number of threads to execute FnSolver in parallel with.

FnSolver tries to determine the number of logical processors on your computer to use as the default. If it cannot do this, the default will be 0, and you must manually set this option. It is recommended you set this to exactly the number of logical processors on your computer. Any less will result in worse performance due to unused system resources (though you may intentionally desire this), while any more will not yield better performance due to already using all system resources.



## Complete Examples

These examples use the pre-built Windows executable to demonstrate.

- `fnsolver-cli.exe -f max_effective_mining 2 --tiebreaker max_mining -i all_de no_research M1:0 M2:0 M3:0 M4:0 M5:4 --seed 405:D 408:D 409:D --force-seed --precious-resources bonjelium:all marine_rutile:all parhelion_platinum:all -n 2000 --bonus-iterations 100 -p 200 -o 300 --export-config-file -y`
- `fnsolver-cli.exe --config-file config.toml --seed 312:D 315:D 321:D --auto-confirm=false -n 500 --bonus-iterations 0`
- `fnsolver-cli.exe -f ratio 1 1.5 3 -i all_og --territories 315:1 117:0 414:0 505:1 --locked-sites https://frontiernav.net/wiki/xenoblade-chronicles-x/visualisations/maps/probe-guides/My%20Current%20Layout?map=101-1~102-1~103-1~104-1~105-1~106-1~107-1~108-1~109-1~110-0~111-1~112-1~113-1~114-1~115-1~116-1~117-1~118-1~119-1~120-1~121-1~201-1~202-1~203-1~204-1~205-1~206-1~207-1~208-1~209-1~210-1~211-1~212-1~213-1~214-1~215-1~216-1~217-1~218-1~219-1~220-1~221-1~222-1~223-1~224-1~225-1~301-1~302-1~303-1~304-1~305-1~306-1~307-1~308-1~309-1~310-1~311-1~312-1~313-1~314-1~315-1~316-1~317-1~318-1~319-1~320-0~321-1~322-1~401-1~402-1~403-1~404-1~405-1~406-1~407-1~408-1~409-1~410-1~411-1~412-1~413-1~414-1~415-1~416-1~417-1~418-1~419-1~420-1~501-1~502-1~503-1~504-1~505-1~506-1~507-0~508-1~509-1~510-1~511-1~512-1~513-1~514-1~515-1~516-1 -t 1`



## Building

FnSolver does not require any external libraries. You only need a need a C++ toolchain that supports C++20, and a relatively up-to-date version of CMake.

If you are building on Linux, it is recommended to configure and build from the provided CMake presets, which are:

- `debug` (GCC Debug build)
- `release` (GCC Release build with optimizations)
- `release-win` (mingw-w64 GCC cross-compile for Windows Release build with optimizations; requires mingw-w64)

Run the following commands:

```bash
> cmake --preset <preset>
> cmake --build ./build-<preset>
```

If you are building on a different platform, or wish to use a different Generator or Toolchain (e.g., Ninja, Clang/LLVM, native MSVC for Windows, etc.), it should be fairly straightforward to define your own preset (probably just need to specify the correct compiler flags), as the build isn't doing anything fancy. But you're on your own there.



## FAQ

#### Are the layouts generated by FnSolver truly optimal?

No. The problem space is intractable; even with a perfectly selected inventory (lets use `all_de no_research M1:0 M2:0 M3:0 M4:0 M5:2`, though this may not be perfect, and only addresses a subset of score functions!), there are ~7.468e87 unique layouts to consider (for the specific inventory mentioned). If it takes 30 microseconds to compute a score for one FrontierNav layout (roughly the time it takes on the my hardware), and 12 logical processors are available for parallel computing with perfect efficiency, that's 5.920e74 years to compute a score for all layouts.

As such, FnSolver uses an algorithm roughly similar to [Random-restart Stochastic Hill Climbing](https://en.wikipedia.org/wiki/Hill_climbing#Variants) to make iterative improvements to random initial FrontierNav layouts. Some shortcomings of this method are discussed in the sub-headings under [Solver Algorithm Parameters](#solver-algorithm-parameters), but in short, it is prone to Local Maximums and struggles with precise chain manipulations.

That said, the FrontierNav layouts that FnSolver outputs (given a reasonable amount of time to run) tend to be quite competitive with ones a knowledgeable and dedicated human could come up with by hand (from my own experience, within ~2%). Furthermore, such a human can provide their insight to FnSolver via the [`--seed`](#--seed) option, and often arrive at the same (or a better) FrontierNav layout.

#### macOS?

There should be nothing stopping you from building FnSolver from source on macOS, but I lack the environment to build and test it myself, so I'm not comfortably providing a pre-built executable.

#### This makes my computer hot and slow.

FnSolver is heavily (purely) CPU intensive. It's doing a bunch of math over and over without stopping. Ideally, it's using 100% of your CPU.

If you want to limit the amount of CPU FnSolver uses, use the [`--threads`](#--threads) option to set the thread count to less than the number of logical processors on your computer.

#### Configuring the [Solver Algorithm Parameters](#solver-algorithm-parameters) is overwhelming, what values should I use?

If you're not sure, just use the defaults.

I've found that increasing [`--offspring`](#--offspring) and [`--population`](#--population) tend to be more effective than increasing [`--iterations`](#--iterations), and setting [`--bonus-iterations`](#--bonus-iterations) can help avoid scenarios where you get improvements just before FnSolver would terminate, which otherwise wouldn't get sufficient time to improve further. I tend to leave [`--mutation-rate`](#--mutation-rate) and [`--max-age`](#--max-age) alone.

#### Which [Score Function](#score-function) is best?

That really depends on what you want out of FrontierNav. Many options are provided because everyone wants something different.

Generally, I think that `max_effective_mining` is the most relevant Score Function (optionally with a `max_mining` tiebreaker). Revenue seems to be fairly worthless outside of a couple quests that require it, and it captures the nuance of "you get Miranium in discreet chunks but are capped by your Storage).

#### Why shouldn't I use the [`--min-mining`](#--min-mining)/[`--min-revenue`](#--min-revenue)/[`--min-storage`](#--min-storage) constraints?

Because failing constraints set the score of a FrontierNav layout to zero, FnSolver isn't able to make "progress" towards reaching a minimum yield constraint. It either randomly generates a layout that satisfies the constraint or it doesn't. This isn't an issue with Precious Resource constraints (unless you do something extreme, like requiring 100% for all resources), because satisfying the constraint is much simpler (have any Basic or Mining probe on certain sites) and easier to "stick to" when making mutations.

I experimented with having FrontierNav layouts that failed constraints instead multiply their score by e.g. 0.01, but it didn't really do much, and made for more confusing output.

Prefer experimenting with the `ratio` Score Function in order to hit a yield threshold.

