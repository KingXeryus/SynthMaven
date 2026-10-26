# SynthMaven maqam scale library

34 canonical entries cover the [MaqamWorld index](https://maqamworld.com/en/maqam.php), with Ajam Ushayran grouped under Ajam and other transposition-only names searchable as aliases. Musical note spellings were independently encoded as editable relative-cent sketches; no source illustrations, recordings or article text are bundled. Research/reference credit: Johnny Farraj and MaqamWorld. The Digital Arabic Maqam Archive was consulted for research but its dataset is not bundled.

The library covers the Ajam, Bayati, Hijaz, Kurd, Nahawand, Nikriz, Rast and Sikah families, plus independent modes. It is not an exhaustive claim about all historical Arabic theory. Maqam includes melodic behavior beyond a scale. Jiharkah and Sikah Baladi require especially flexible intonation; the supplied values are starting sketches. Non-octave and upper-register behavior is reduced to an octave representation.

Names with shared pitch sets may remain distinct when their melodic identity differs (for example, Rast and Kirdan). Root transpositions are selected with ROOT rather than duplicated as Rast C, Rast D, etc. Primary/descending alternatives appear inside a single preset entry. Direction is selected manually.

Dense quarter-tone intervals are assigned to unique ascending MIDI key slots while minimizing cent displacement, so no degree overwrites another. The piano highlights the resulting playable key mapping; it does not silently remap non-scale notes.

Canonical entries: Ajam; Athar Kurd; Awj Iraq; Bastanikar; Bayati; Bayati Shuri; Dalanshin; Hijaz; Hijazkar; Hijazkar Kurd; Husayni; Huzam; Iraq; Jiharkah; Kirdan; Kurd; Lami; Mahur; Mustaar; Nahawand; Nahawand Murassaa; Nairuz; Nawa Athar; Nikriz; Rast; Saba; Saba Zamzam; Shawq Afza; Sikah; Sikah Baladi; Suzdalara; Suznak; Ushaq Masri; Zanjaran.

For the actual cent arrays, aliases and per-entry limitations, see [MaqamPresets.h](Source/MaqamPresets.h). Additional user-selected tunings can be loaded through Scala.
